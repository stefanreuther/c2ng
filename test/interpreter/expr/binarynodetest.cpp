/**
  *  \file test/interpreter/expr/binarynodetest.cpp
  *  \brief Test for interpreter::expr::BinaryNode
  */

#include "interpreter/expr/binarynode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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

        Environment(afl::test::Assert a)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

/** Test compileValue(). */
AFL_TEST("interpreter.expr.BinaryNode:compileValue", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(17)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(4)));
    interpreter::expr::BinaryNode testee(interpreter::biAdd, leftValue, rightValue);

    // Compile: '17 + 4'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 21);    // 17+4
}

/** Test compileEffect(). */
AFL_TEST("interpreter.expr.BinaryNode:compileEffect", a)
{
    Environment env(a);

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
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify: result must be first
    const interpreter::KeymapValue* kv = dynamic_cast<const interpreter::KeymapValue*>(env.proc.getResult());
    a.checkNonNull("11. KeymapValue", kv);
    a.checkEqual("12. getKeymap", kv->getKeymap(), first);

    // Verify: keymap has been added
    a.check("21. hasParent", first->hasParent(*second));
}

/** Test compileStore(), compileRead(), compileWrite().
    Those are rejected for BinaryNode. */
AFL_TEST("interpreter.expr.BinaryNode:others", a)
{
    Environment env(a);

    // Testing '17 + 4'
    interpreter::expr::LiteralNode leftValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(17)));
    interpreter::expr::LiteralNode rightValue(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(4)));
    interpreter::expr::BinaryNode testee(interpreter::biAdd, leftValue, rightValue);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(env.world), leftValue), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
