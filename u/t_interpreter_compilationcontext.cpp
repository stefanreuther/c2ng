/**
  *  \file u/t_interpreter_compilationcontext.cpp
  *  \brief Test for interpreter::CompilationContext
  */

#include "interpreter/compilationcontext.hpp"

#include "t_interpreter.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/world.hpp"

/** Simple test. */
void
TestInterpreterCompilationContext::testIt()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);

    // Testee
    interpreter::CompilationContext testee(world);

    // Default state
    TS_ASSERT(testee.hasFlag(interpreter::CompilationContext::CaseBlind));
    TS_ASSERT(!testee.hasFlag(interpreter::CompilationContext::LocalContext));
    TS_ASSERT(!testee.hasFlag(interpreter::CompilationContext::WantTerminators));

    // Modify state
    testee.withFlag(interpreter::CompilationContext::LocalContext)
        .withoutFlag(interpreter::CompilationContext::CaseBlind);
    TS_ASSERT(!testee.hasFlag(interpreter::CompilationContext::CaseBlind));
    TS_ASSERT(testee.hasFlag(interpreter::CompilationContext::LocalContext));

    // World
    TS_ASSERT_EQUALS(&testee.world(), &world);
}

