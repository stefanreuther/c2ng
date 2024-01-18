/**
  *  \file test/interpreter/expr/sequencenodetest.cpp
  *  \brief Test for interpreter::expr::SequenceNode
  */

#include "interpreter/expr/sequencenode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/expr/unarynode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/unaryoperation.hpp"
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

AFL_TEST("interpreter.expr.SequenceNode:compileValue", a)
{
    Environment env(a);

    // Test 'unKeyCreate('X'); 10'. Must create the keymap and return 10.
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::UnaryNode leftOp(interpreter::unKeyCreate, leftValue);
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::SequenceNode testee(leftOp, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify: keymap
    a.checkNonNull("11. getKeymapByName", env.world.keymaps().getKeymapByName("X"));

    // Verify: Value
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("21. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("22. result", iv, 10);
}

AFL_TEST("interpreter.expr.SequenceNode:compileEffect", a)
{
    Environment env(a);

    // Test '10; unKeyCreate('X')'. Must create the keymap
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::UnaryNode rightOp(interpreter::unKeyCreate, rightValue);
    interpreter::expr::SequenceNode testee(leftValue, rightOp);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    a.checkNonNull("11. getKeymapByName", env.world.keymaps().getKeymapByName("X"));
}

AFL_TEST("interpreter.expr.SequenceNode:compileCondition", a)
{
    Environment env(a);

    // Test 'If(unKeyCreate('X'); 10, 2, 3)'
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::UnaryNode leftOp(interpreter::unKeyCreate, leftValue);
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::SequenceNode testee(leftOp, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    interpreter::BytecodeObject::Label_t lthen = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lelse = bco->makeLabel();
    interpreter::BytecodeObject::Label_t lend = bco->makeLabel();
    testee.compileCondition(*bco, interpreter::CompilationContext(env.world), lthen, lelse);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 1);  // Not reached; indicates an error if reached
    bco->addJump(interpreter::Opcode::jAlways, lend);
    bco->addLabel(lthen);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 2);
    bco->addJump(interpreter::Opcode::jAlways, lend);
    bco->addLabel(lelse);
    bco->addInstruction(interpreter::Opcode::maPush, interpreter::Opcode::sInteger, 3);
    bco->addLabel(lend);

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify: keymap
    a.checkNonNull("11. getKeymapByName", env.world.keymaps().getKeymapByName("X"));

    // Verify: Value must be 2
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("21. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("22. result", iv, 2);
}

AFL_TEST("interpreter.expr.SequenceNode:others", a)
{
    Environment env(a);

    // Test '"X";10'
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::SequenceNode testee(leftValue, rightValue);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(env.world), leftValue), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
