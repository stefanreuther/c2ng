/**
  *  \file u/t_interpreter_process.cpp
  *  \brief Test for interpreter::Process
  */

#include "interpreter/process.hpp"

#include "t_interpreter.hpp"
#include "interpreter/world.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"

/** Test process properties. */
void
TestInterpreterProcess::testProperties()
{
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process testee(world, "processName", 42);

    // Initial states
    TS_ASSERT_EQUALS(testee.getName(), "processName");
    TS_ASSERT_EQUALS(testee.getProcessId(), 42U);
    TS_ASSERT_EQUALS(testee.getState(), testee.Suspended);

    // Initial group Id is unset
    TS_ASSERT_EQUALS(testee.getProcessGroupId(), 0U);
    testee.setProcessGroupId(23);
    TS_ASSERT_EQUALS(testee.getProcessGroupId(), 23U);

    // Initial priority is 50
    TS_ASSERT_EQUALS(testee.getPriority(), 50);
    testee.setPriority(12);
    TS_ASSERT_EQUALS(testee.getPriority(), 12);

    // No initial kind
    TS_ASSERT_EQUALS(testee.getProcessKind(), testee.pkDefault);
    testee.setProcessKind(testee.pkBaseTask);
    TS_ASSERT_EQUALS(testee.getProcessKind(), testee.pkBaseTask);
}

