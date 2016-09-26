/**
  *  \file u/t_interpreter_processlist.cpp
  *  \brief Test for interpreter::ProcessList
  */

#include "interpreter/processlist.hpp"

#include "t_interpreter.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/world.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "afl/io/nullfilesystem.hpp"

namespace {
    class Counter {
     public:
        Counter()
            : m_value(0)
            { }
        void increment()
            { ++m_value; }
        int get()
            { return m_value; }
     private:
        int m_value;
    };
}

/** Test empty process list: run. */
void
TestInterpreterProcessList::testEmpty1()
{
    interpreter::ProcessList testee;
    testee.run();
    testee.removeTerminatedProcesses();
}

/** Test empty process list: signaling */
void
TestInterpreterProcessList::testEmpty2()
{
    interpreter::ProcessList testee;
    Counter c;
    TS_ASSERT_EQUALS(c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Just running does nothing
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 0);

    // Running an empty process group signals once
    testee.startProcessGroup(testee.allocateProcessGroup());
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 1);
}

/** Test allocateProcessGroup(). */
void
TestInterpreterProcessList::testAllocateProcessGroup()
{
    interpreter::ProcessList testee;
    uint32_t a = testee.allocateProcessGroup();
    uint32_t b = testee.allocateProcessGroup();
    uint32_t c = testee.allocateProcessGroup();
    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_DIFFERS(b, c);
    TS_ASSERT_DIFFERS(c, a);
}

/** Test execution vs suspension. */
void
TestInterpreterProcessList::testSuspend()
{
    interpreter::ProcessList testee;

    // Add a process that will suspend
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process& p = testee.create(world, "testSuspend");

    interpreter::BCORef_t bco = new interpreter::BytecodeObject();
    bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
    p.pushFrame(bco, false);
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Suspended);

    // Prepare execution
    Counter c;
    TS_ASSERT_EQUALS(c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Nothing scheduled yet
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 0);
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Suspended);

    // Resume it. Will still not run because we didn't start it.
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Runnable);
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 0);
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Runnable);

    // Start it! This must run the process until it suspends
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(p.getState(), interpreter::Process::Suspended);
}
