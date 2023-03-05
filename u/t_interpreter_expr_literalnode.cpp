/**
  *  \file u/t_interpreter_expr_literalnode.cpp
  *  \brief Test for interpreter::expr::LiteralNode
  */

#include "interpreter/expr/literalnode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
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

        Environment(String_t name)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, name, 42)
            { }
    };
}

/** Test compileValue().
    More tests covering this code are in TestInterpreterExprParser::testLiterals, TestInterpreterExprParser::testSequence. */
void
TestInterpreterExprLiteralNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode testee(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));

    // Compile
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 10);
}

/** Test compileStore, compileRead, compileWrite.
    Those must be rejected without generating code. */
void
TestInterpreterExprLiteralNode::testOther()
{
    Environment env("testOther");
    interpreter::expr::LiteralNode testee(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), testee), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

