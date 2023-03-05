/**
  *  \file u/t_interpreter_expr_sequencenode.cpp
  *  \brief Test for interpreter::expr::SequenceNode
  */

#include "interpreter/expr/sequencenode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
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

        Environment(String_t name)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, name, 42)
            { }
    };
}

void
TestInterpreterExprSequenceNode::testValue()
{
    Environment env("testValue");

    // Test 'unKeyCreate('X'); 10'. Must create the keymap and return 10.
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::UnaryNode leftOp(interpreter::unKeyCreate, leftValue);
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::SequenceNode testee(leftOp, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: keymap
    TS_ASSERT(env.world.keymaps().getKeymapByName("X") != 0);

    // Verify: Value
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 10);
}

void
TestInterpreterExprSequenceNode::testEffect()
{
    Environment env("testEffect");

    // Test '10; unKeyCreate('X')'. Must create the keymap
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::UnaryNode rightOp(interpreter::unKeyCreate, rightValue);
    interpreter::expr::SequenceNode testee(leftValue, rightOp);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    TS_ASSERT(env.world.keymaps().getKeymapByName("X") != 0);
}

void
TestInterpreterExprSequenceNode::testCondition()
{
    Environment env("testCondition");

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
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: keymap
    TS_ASSERT(env.world.keymaps().getKeymapByName("X") != 0);

    // Verify: Value must be 2
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 2);
}

void
TestInterpreterExprSequenceNode::testOther()
{
    Environment env("testOther");

    // Test '"X";10'
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("X")));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));
    interpreter::expr::SequenceNode testee(leftValue, rightValue);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), leftValue), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

