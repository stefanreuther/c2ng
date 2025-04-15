/**
  *  \file test/interpreter/expr/assignmentnodetest.cpp
  *  \brief Test for interpreter::expr::AssignmentNode
  */

#include "interpreter/expr/assignmentnode.hpp"

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
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        interpreter::Process proc;

        Environment(afl::test::Assert a)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

/** Test compileValue(). */
AFL_TEST("interpreter.expr.AssignmentNode:compileValue", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Compile: 'V := 10', keeping the result on stack. To prove that the result was correctly kept, add variable and value.
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("V");
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);
    bco->addInstruction(interpreter::Opcode::maBinary, interpreter::biAdd, 0);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. value", iv, 20);
}

/** Test compileEffect(). */
AFL_TEST("interpreter.expr.AssignmentNode:compileEffect", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(19)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Compile: 'V := 10', effect only. To prove that it worked, load the variable.
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    uint16_t lv = bco->addLocalVariable("V");
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sLocal, lv);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. value", iv, 19);
}

/** Test compileStore, compileRead, compileWrite.
    Those must be rejected without generating code. */
AFL_TEST("interpreter.expr.AssignmentNode:others", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(19)));
    interpreter::expr::IdentifierNode var("V");
    interpreter::expr::AssignmentNode testee(var, value);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(env.world), testee), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
