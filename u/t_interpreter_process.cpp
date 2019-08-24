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

    // Name
    testee.setName("otherName");
    TS_ASSERT_EQUALS(testee.getName(), "otherName");

    // State
    testee.setState(interpreter::Process::Ended);
    TS_ASSERT_EQUALS(testee.getState(), testee.Ended);
}

/** Test freezing. */
void
TestInterpreterProcess::testFreeze()
{
    // Test implementation of Freezer
    class TestFreezer : public interpreter::Process::Freezer { };

    // Make a process
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process testee(world, "processName", 42);

    // We can freeze a fresh process
    TestFreezer fz;
    TS_ASSERT_THROWS_NOTHING(testee.freeze(fz));
    TS_ASSERT_EQUALS(testee.getState(), interpreter::Process::Frozen);
    TS_ASSERT_EQUALS(testee.getFreezer(), static_cast<interpreter::Process::Freezer*>(&fz));

    // We cannot freeze it again, not even re-using the same freezer
    {
        TestFreezer fz2;
        TS_ASSERT_THROWS(testee.freeze(fz2), interpreter::Error);
        TS_ASSERT_THROWS(testee.freeze(fz), interpreter::Error);
    }

    // Unfreeze
    TS_ASSERT_THROWS_NOTHING(testee.unfreeze());
    TS_ASSERT_EQUALS(testee.getState(), interpreter::Process::Suspended);
    TS_ASSERT_EQUALS(testee.getFreezer(), (interpreter::Process::Freezer*) 0);

    // Can freeze again
    TS_ASSERT_THROWS_NOTHING(testee.freeze(fz));
    TS_ASSERT_EQUALS(testee.getState(), interpreter::Process::Frozen);
    TS_ASSERT_EQUALS(testee.getFreezer(), static_cast<interpreter::Process::Freezer*>(&fz));
}

/** Test freezing. */
void
TestInterpreterProcess::testFreeze2()
{
    // Test implementation of Freezer
    class TestFreezer : public interpreter::Process::Freezer { };

    // Make a process
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process testee(world, "processName", 42);

    // Change state
    testee.setState(interpreter::Process::Waiting);

    // Process cannot be frozen in wrong state
    TestFreezer fz;
    TS_ASSERT_THROWS(testee.freeze(fz), interpreter::Error);
    TS_ASSERT_EQUALS(testee.getState(), interpreter::Process::Waiting);
    TS_ASSERT_EQUALS(testee.getFreezer(), (interpreter::Process::Freezer*) 0);

    // Process cannot be unfrozen in wrong state (but this does not throw)
    TS_ASSERT_THROWS_NOTHING(testee.unfreeze());
    TS_ASSERT_EQUALS(testee.getState(), interpreter::Process::Waiting);
}

