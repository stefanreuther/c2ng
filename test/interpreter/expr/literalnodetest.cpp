/**
  *  \file test/interpreter/expr/literalnodetest.cpp
  *  \brief Test for interpreter::expr::LiteralNode
  */

#include "interpreter/expr/literalnode.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
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

        Environment(afl::test::Assert a)
            : log(), tx(), fs(),
              world(log, tx, fs),
              proc(world, a.getLocation(), 42)
            { }
    };
}

/** Test compileValue().
    More tests covering this code are in TestInterpreterExprParser::testLiterals, TestInterpreterExprParser::testSequence. */
AFL_TEST("interpreter.expr.LiteralNode:compileValue", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode testee(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));

    // Compile
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    testee.compileValue(*bco, interpreter::CompilationContext(env.world));

    // Run
    env.proc.pushFrame(bco, true);
    AFL_CHECK_SUCCEEDS(a("01. run"), env.proc.run());

    // Verify
    const afl::data::Value* pv = env.proc.getResult();
    int32_t iv = 0;
    a.checkEqual("11. checkIntegerArg", interpreter::checkIntegerArg(iv, pv), true);
    a.checkEqual("12. result", iv, 10);
}

/** Test compileStore, compileRead, compileWrite.
    Those must be rejected without generating code. */
AFL_TEST("interpreter.expr.LiteralNode:others", a)
{
    Environment env(a);
    interpreter::expr::LiteralNode testee(std::auto_ptr<afl::data::Value>(interpreter::makeIntegerValue(10)));

    // Cannot assign or modify
    interpreter::BCORef_t bco = interpreter::BytecodeObject::create(false);
    AFL_CHECK_THROWS(a("01. compileStore"), testee.compileStore(*bco, interpreter::CompilationContext(env.world), testee), interpreter::Error);
    AFL_CHECK_THROWS(a("02. compileRead"),  testee.compileRead(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    AFL_CHECK_THROWS(a("03. compileWrite"), testee.compileWrite(*bco, interpreter::CompilationContext(env.world)), interpreter::Error);
    a.checkEqual("04. getNumInstructions", bco->getNumInstructions(), 0U);
}
