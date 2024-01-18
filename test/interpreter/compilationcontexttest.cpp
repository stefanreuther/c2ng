/**
  *  \file test/interpreter/compilationcontexttest.cpp
  *  \brief Test for interpreter::CompilationContext
  */

#include "interpreter/compilationcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/world.hpp"

/** Simple test. */
AFL_TEST("interpreter.CompilationContext", a)
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // Testee
    interpreter::CompilationContext testee(world);

    // Default state
    a.check("01. hasFlag", testee.hasFlag(interpreter::CompilationContext::CaseBlind));
    a.check("02. hasFlag", !testee.hasFlag(interpreter::CompilationContext::LocalContext));
    a.check("03. hasFlag", !testee.hasFlag(interpreter::CompilationContext::WantTerminators));

    // Modify state
    testee.withFlag(interpreter::CompilationContext::LocalContext)
        .withoutFlag(interpreter::CompilationContext::CaseBlind);
    a.check("11. hasFlag", !testee.hasFlag(interpreter::CompilationContext::CaseBlind));
    a.check("12. hasFlag", testee.hasFlag(interpreter::CompilationContext::LocalContext));

    // World
    a.checkEqual("21. world", &testee.world(), &world);
}
