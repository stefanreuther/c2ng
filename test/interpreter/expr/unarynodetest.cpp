/**
  *  \file test/interpreter/expr/unarynodetest.cpp
  *  \brief Test for interpreter::expr::UnaryNode
  */

#include "interpreter/expr/unarynode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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

        Environment(afl::test::Assert a)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

AFL_TEST("interpreter.expr.UnaryNode:compileValue", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(66)));
    interpreter::expr::UnaryNode testee(interpreter::unInc, value);

    // Compile: '66 + 1'
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run(0));

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 67);
}

AFL_TEST("interpreter.expr.UnaryNode:compileEffect", a)
{
    Environment env(a);

    // An unary operation with an easily observable result is unKeyCreate.
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeStringValue("K")));
    interpreter::expr::UnaryNode testee(interpreter::unKeyCreate, value);
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileEffect(*bco, interpreter::CompilationContext(env.world));

    // Keymap must not exist
    a.checkNull("01. getKeymapByName", env.world.keymaps().getKeymapByName("K"));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("11. run"), env.proc.run(0));

    // Verify
    a.checkNonNull("21. getKeymapByName", env.world.keymaps().getKeymapByName("K"));
}

AFL_TEST("interpreter.expr.UnaryNode:others", a)
{
    Environment env(a);

    // Testing '66 + 1'
    interpreter::expr::LiteralNode value(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(66)));
    interpreter::expr::UnaryNode testee(interpreter::unInc, value);

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(env.world), value), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
