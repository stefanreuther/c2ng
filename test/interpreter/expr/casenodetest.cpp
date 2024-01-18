/**
  *  \file test/interpreter/expr/casenodetest.cpp
  *  \brief Test for interpreter::expr::CaseNode
  */

#include "interpreter/expr/casenode.hpp"

#include "afl/base/deleter.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

namespace {
    struct Environment {
        afl::base::Deleter del;
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment(afl::test::Assert a)
            : del(), log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

/** Test case-blind operation. */
AFL_TEST("interpreter.expr.CaseNode:enabled", a)
{
    Environment env(a);

    // Test '"a" = "A"', case-blind
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("a")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("A")));
    interpreter::expr::CaseNode testee(interpreter::biCompareEQ, leftValue, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world).withFlag(interpreter::CompilationContext::CaseBlind));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    a.checkEqual("11. getBooleanValue", interpreter::getBooleanValue(env.proc.getResult()), true);
}

/** Test case-sensitive operation. */
AFL_TEST("interpreter.expr.CaseNode:disabled", a)
{
    Environment env(a);

    // Test '"a" = "A"', case-sensitive
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("a")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("A")));
    interpreter::expr::CaseNode testee(interpreter::biCompareEQ, leftValue, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world).withoutFlag(interpreter::CompilationContext::CaseBlind));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    a.checkEqual("11. getBooleanValue", interpreter::getBooleanValue(env.proc.getResult()), false);
}

/** Test convertToAssignment(), success case. */
AFL_TEST("interpreter.expr.CaseNode:convertToAssignment:success", a)
{
    Environment env(a);

    // Test successful conversion: 'a = 10'
    interpreter::expr::IdentifierNode leftValue("A");
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::CaseNode testee(interpreter::biCompareEQ, leftValue, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("A");
    testee.convertToAssignment(env.del).compileEffect(*bco, interpreter::CompilationContext(env.world));

    // Returned value is local variable
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify: returned value is 10, newly-assigned values
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 10);
}

/** Test convertToAssignment(), failure case. */
AFL_TEST("interpreter.expr.CaseNode:convertToAssignment:failure", a)
{
    Environment env(a);

    // Test unsuccessful conversion: 'a <> 10'
    interpreter::expr::IdentifierNode leftValue("A");
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::CaseNode testee(interpreter::biCompareNE, leftValue, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("A");
    testee.convertToAssignment(env.del).compileEffect(*bco, interpreter::CompilationContext(env.world));

    // Returned value is local variable
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify: returned value is null, initial value of local variable
    const afl::data::Value* pv = env.proc.getResult();
    a.checkNull("11. result", pv);
}
