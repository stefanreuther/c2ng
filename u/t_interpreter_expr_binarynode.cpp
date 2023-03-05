/**
  *  \file u/t_interpreter_expr_binarynode.cpp
  *  \brief Test for interpreter::expr::BinaryNode
  */

#include "interpreter/expr/binarynode.hpp"

#include "t_interpreter_expr.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/keymapvalue.hpp"
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

/** Test compileValue(). */
void
TestInterpreterExprBinaryNode::testValue()
{
    Environment env("testValue");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(17)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(4)));
    interpreter::expr::BinaryNode testee(interpreter::biAdd, leftValue, rightValue);

    // Compile: '17 + 4'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    TS_ASSERT_EQUALS(interpreter::checkIntegerArg(iv, pv), true);
    TS_ASSERT_EQUALS(iv, 21);    // 17+4
}

/** Test compileEffect(). */
void
TestInterpreterExprBinaryNode::testEffect()
{
    Environment env("testEffect");

    // A binary operation with an easily observable result is biKeyAddParent, so we're testing that.
    util::KeymapRef_t first = env.world.keymaps().createKeymap("FIRST");
    util::KeymapRef_t second = env.world.keymaps().createKeymap("SECOND");
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(new interpreter::KeymapValue(first)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(new interpreter::KeymapValue(second)));
    interpreter::expr::BinaryNode testee(interpreter::biKeyAddParent, leftValue, rightValue);

    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    TS_ASSERT_THROWS_NOTHING(env.proc.run());

    // Verify: result must be first
    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(env.proc.getResult());
    TS_ASSERT(kv != 0);
    TS_ASSERT_EQUALS(kv->getKeymap(), first);

    // Verify: keymap has been added
    TS_ASSERT(first->hasParent(*second));
}

/** Test compileStore(), compileRead(), compileWrite().
    Those are rejected for BinaryNode. */
void
TestInterpreterExprBinaryNode::testOther()
{
    Environment env("testOther");

    // Testing '17 + 4'
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(17)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(4)));
    interpreter::expr::BinaryNode testee(interpreter::biAdd, leftValue, rightValue);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    TS_ASSERT_THROWS(testee.compileStore(*bco, interpreter::CompilationContext(env.world), leftValue), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_THROWS(testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    TS_ASSERT_EQUALS(bco->getNumInstructions(), 0U);
}

