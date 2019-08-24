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

    interpreter::BCORef_t makeSuspendBCO()
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialSuspend, 0);
        return bco;
    }

    interpreter::BCORef_t makeFailBCO()
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        bco->addInstruction(interpreter::Opcode::maPush,    interpreter::Opcode::sInteger, 0);
        bco->addInstruction(interpreter::Opcode::maSpecial, interpreter::Opcode::miSpecialThrow, 0);
        return bco;
    }

    interpreter::BCORef_t makeEmptyBCO()
    {
        interpreter::BCORef_t bco = *new interpreter::BytecodeObject();
        return bco;
    }
}

/** Test empty process list: run.
    This is a boundary case that must be handled correctly. */
void
TestInterpreterProcessList::testEmpty1()
{
    interpreter::ProcessList testee;
    testee.run();
    testee.removeTerminatedProcesses();
}

/** Test empty process list: signaling.
    A process group that becomes empty must be signalled correctly. */
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

/** Test allocateProcessGroup().
    Id allocation must produce different Ids. */
void
TestInterpreterProcessList::testAllocateProcessGroup()
{
    interpreter::ProcessList testee;

    // Process groups
    uint32_t a = testee.allocateProcessGroup();
    uint32_t b = testee.allocateProcessGroup();
    uint32_t c = testee.allocateProcessGroup();
    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_DIFFERS(b, c);
    TS_ASSERT_DIFFERS(c, a);

    // Process Ids
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process& pa = testee.create(world, "a");
    interpreter::Process& pb = testee.create(world, "b");
    interpreter::Process& pc = testee.create(world, "c");
    TS_ASSERT_DIFFERS(pa.getProcessId(), pb.getProcessId());
    TS_ASSERT_DIFFERS(pb.getProcessId(), pc.getProcessId());
    TS_ASSERT_DIFFERS(pc.getProcessId(), pa.getProcessId());
}

/** Test execution vs suspension.
    A suspending process causes the process group to signal. */
void
TestInterpreterProcessList::testSuspend()
{
    interpreter::ProcessList testee;

    // Add a process that will suspend
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);
    interpreter::Process& p = testee.create(world, "testSuspend");

    p.pushFrame(makeSuspendBCO(), false);
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

/** Test joinProcess().
    Moving a process from one process group to another joins the process groups. */
void
TestInterpreterProcessList::testJoin()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    interpreter::Process& p1 = testee.create(world, "1");
    interpreter::Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // One process in another process group
    uint32_t pgB = testee.allocateProcessGroup();
    interpreter::Process& p3 = testee.create(world, "3");
    p3.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p3, pgB);

    // Join p2 into pgB
    testee.joinProcess(p2, pgB);

    // Run
    testee.startProcessGroup(pgB);
    testee.run();

    // All processes terminated now
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Ended);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Ended);
    TS_ASSERT_EQUALS(p3.getState(), interpreter::Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    TS_ASSERT(testee.getProcessList().empty());
}

/** Test execution with failing processes.
    If a process fails, the next one from its process group executes. */
void
TestInterpreterProcessList::testFail()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    interpreter::Process& p1 = testee.create(world, "1");
    interpreter::Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeFailBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // States
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Running);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Failed);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    TS_ASSERT(testee.getProcessList().empty());
}

/** Test termination.
    If a process that is about to run is terminated, the next one from its process group executes. */
void
TestInterpreterProcessList::testTerminate()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    interpreter::Process& p1 = testee.create(world, "1");
    interpreter::Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeFailBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // States
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Running);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Runnable);

    // Terminate
    testee.terminateProcess(p1);
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Running);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), interpreter::Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), interpreter::Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    TS_ASSERT(testee.getProcessList().empty());
}

/** Test priority handling. */
void
TestInterpreterProcessList::testPriority()
{
    // Environment
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, fs);

    interpreter::ProcessList testee;

    // Three processes
    interpreter::Process& p1 = testee.create(world, "1");
    interpreter::Process& p2 = testee.create(world, "2");
    interpreter::Process& p3 = testee.create(world, "3");

    // Verify initial priorities and placement
    TS_ASSERT_EQUALS(p1.getPriority(), 50);
    TS_ASSERT_EQUALS(p2.getPriority(), 50);
    TS_ASSERT_EQUALS(p3.getPriority(), 50);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p1);
    TS_ASSERT_EQUALS(testee.getProcessList()[1], &p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[2], &p3);
    TS_ASSERT_EQUALS(testee.getProcessById(p1.getProcessId()), &p1);
    TS_ASSERT_EQUALS(testee.getProcessById(p2.getProcessId()), &p2);
    TS_ASSERT_EQUALS(testee.getProcessById(p3.getProcessId()), &p3);

    uint32_t unknownPID = (p1.getProcessId() | p2.getProcessId() | p3.getProcessId()) + 1;
    TS_ASSERT(testee.getProcessById(unknownPID) == 0);

    // Null operation on 2's priority
    p2.setPriority(50);
    testee.handlePriorityChange(p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p1);
    TS_ASSERT_EQUALS(testee.getProcessList()[1], &p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[2], &p3);

    // Improve 2's priority --> [2,1,3]
    p2.setPriority(10);
    testee.handlePriorityChange(p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[1], &p1);
    TS_ASSERT_EQUALS(testee.getProcessList()[2], &p3);

    // Improve 3's priority --> [2,3,1]
    p3.setPriority(10);
    testee.handlePriorityChange(p3);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[1], &p3);
    TS_ASSERT_EQUALS(testee.getProcessList()[2], &p1);

    // Drop 2's priority to same value as 1 --> [3,1,2]
    p2.setPriority(50);
    testee.handlePriorityChange(p2);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p3);
    TS_ASSERT_EQUALS(testee.getProcessList()[1], &p1);
    TS_ASSERT_EQUALS(testee.getProcessList()[2], &p2);
}

