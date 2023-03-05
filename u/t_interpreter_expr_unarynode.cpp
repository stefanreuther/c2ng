/**
  *  \file u/t_interpreter_expr_unarynode.cpp
  *  \brief Test for interpreter::expr::UnaryNode
  */

#include "interpreter/expr/unarynode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/literalnode.hpp"
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
TestInterpreterExprUnaryNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(66)));
    interpreter::expr::UnaryNode testee(interpreter::unInc, value);

    // Compile: '66 + 1'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 67);
}

void
TestInterpreterExprUnaryNode::testEffect()
{
    Environment env("testEffect");

    // An unary operation with an easily observable result is unKeyCreate.
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("K")));
    interpreter::expr::UnaryNode testee(interpreter::unKeyCreate, value);
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileEffect(*bco, interpreter::CompilationContext(env.world));

    // Keymap must not exist
    TS_ASSERT(env.world.keymaps().getKeymapByName("K") == 0);

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    TS_ASSERT(env.world.keymaps().getKeymapByName("K") != 0);
}

void
TestInterpreterExprUnaryNode::testOther()
{
    Environment env("testOther");

    // Testing '66 + 1'
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(66)));
    interpreter::expr::UnaryNode testee(interpreter::unInc, value);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), value), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

