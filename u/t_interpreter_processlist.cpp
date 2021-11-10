/**
  *  \file u/t_interpreter_processlist.cpp
  *  \brief Test for interpreter::ProcessList
  */

#include "interpreter/processlist.hpp"

#include "t_interpreter.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/object.hpp"
#include "game/test/counter.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/world.hpp"

namespace {
    using afl::data::IntegerValue;
    using interpreter::Process;
    using interpreter::BCORef_t;
    using interpreter::Opcode;
    using game::test::Counter;

    void addStateSetter(interpreter::BytecodeObject& bco, Process::State st)
    {
        class StateSetter : public interpreter::CallableValue {
         public:
            StateSetter(Process::State st)
                : m_state(st)
                { }
            virtual void call(Process& p, afl::data::Segment&, bool wantResult)
                {
                    if (wantResult) {
                        p.pushNewValue(0);
                    }
                    p.setState(m_state);
                }
            virtual bool isProcedureCall() const
                { return false; }
            virtual int32_t getDimension(int32_t) const
                { return 0; }
            virtual interpreter::Context* makeFirstContext()
                { return 0; }
            virtual StateSetter* clone() const
                { return new StateSetter(m_state); }
            virtual String_t toString(bool) const
                { return "#<state>"; }
            virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
                { TS_FAIL("store unexpected"); }

         private:
            Process::State m_state;
        };
        StateSetter sts(st);

        bco.addPushLiteral(&sts);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
    }

    BCORef_t makeSuspendBCO()
    {
        BCORef_t bco = *new interpreter::BytecodeObject();
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return bco;
    }

    BCORef_t makeFailBCO()
    {
        BCORef_t bco = *new interpreter::BytecodeObject();
        bco->addInstruction(Opcode::maPush,    Opcode::sInteger, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        return bco;
    }

    BCORef_t makeEmptyBCO()
    {
        BCORef_t bco = *new interpreter::BytecodeObject();
        return bco;
    }

    // Make a BCO that first sets state st, then pushes integer n.
    BCORef_t makeStateBCO(Process::State st, uint16_t n)
    {
        BCORef_t bco = *new interpreter::BytecodeObject();
        addStateSetter(*bco, st);
        bco->addInstruction(Opcode::maPush, Opcode::sInteger, n);
        return bco;
    }

    int32_t toInteger(const afl::data::Value* v)
    {
        const IntegerValue* iv = dynamic_cast<const IntegerValue*>(v);
        if (iv == 0) {
            throw interpreter::Error::typeError();
        }
        return iv->getValue();
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
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    Process& pa = testee.create(world, "a");
    Process& pb = testee.create(world, "b");
    Process& pc = testee.create(world, "c");
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
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    Process& p = testee.create(world, "testSuspend");

    p.pushFrame(makeSuspendBCO(), false);
    TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

    // Prepare execution
    Counter c;
    TS_ASSERT_EQUALS(c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Nothing scheduled yet
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 0);
    TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

    // Resume it. Will still not run because we didn't start it.
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    TS_ASSERT_EQUALS(p.getState(), Process::Runnable);
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 0);
    TS_ASSERT_EQUALS(p.getState(), Process::Runnable);

    // Start it! This must run the process until it suspends
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(c.get(), 1);
    TS_ASSERT_EQUALS(p.getState(), Process::Suspended);
}

/** Test joinProcess().
    Moving a process from one process group to another joins the process groups. */
void
TestInterpreterProcessList::testJoin()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // One process in another process group
    uint32_t pgB = testee.allocateProcessGroup();
    Process& p3 = testee.create(world, "3");
    p3.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p3, pgB);

    // Join p2 into pgB
    testee.joinProcess(p2, pgB);

    // Run
    testee.startProcessGroup(pgB);
    testee.run();

    // All processes terminated now
    TS_ASSERT_EQUALS(p1.getState(), Process::Ended);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);
    TS_ASSERT_EQUALS(p3.getState(), Process::Ended);

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
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeFailBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // States
    TS_ASSERT_EQUALS(p1.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    TS_ASSERT_EQUALS(p1.getState(), Process::Running);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Failed);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);

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
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeFailBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // States
    TS_ASSERT_EQUALS(p1.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    TS_ASSERT_EQUALS(p1.getState(), Process::Running);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Terminate
    testee.terminateProcess(p1);
    TS_ASSERT_EQUALS(p1.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), Process::Running);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);

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
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Three processes
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    Process& p3 = testee.create(world, "3");

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

/** Test end signalisation. */
void
TestInterpreterProcessList::testEndSignal()
{
    class Finalizer : public Process::Finalizer {
     public:
        Finalizer(int& count)
            : m_count(count)
            { }
        void finalizeProcess(Process&)
            { ++m_count; }
     private:
        int& m_count;
    };

    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes; neither has any code, so they will end immediately
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");

    // Process finalizers
    int finalizeCount = 0;
    p1.setNewFinalizer(new Finalizer(finalizeCount));
    p2.setNewFinalizer(new Finalizer(finalizeCount));

    // Process group finalizers
    Counter pgCount;
    testee.sig_processGroupFinish.add(&pgCount, &Counter::increment);

    // Add both processes to a process group and run them
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p1, pgid);
    testee.resumeProcess(p2, pgid);
    testee.startProcessGroup(pgid);
    testee.run();

    TS_ASSERT_EQUALS(finalizeCount, 2);
    TS_ASSERT_EQUALS(pgCount.get(), 1);
}

/** Test wait, continueProcess.
    This is what happens when a process calls UI. */
void
TestInterpreterProcessList::testWait()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Process
    Process& p = testee.create(world, "p");
    p.pushFrame(makeStateBCO(Process::Waiting, 44), false);

    // Process group finalizers
    Counter pgCount;
    testee.sig_processGroupFinish.add(&pgCount, &Counter::increment);

    // Run process to Waiting
    int32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(p.getState(), Process::Waiting);
    TS_ASSERT_EQUALS(pgCount.get(), 0);

    testee.continueProcess(p);
    testee.run();

    // Process now terminated
    TS_ASSERT_EQUALS(p.getState(), Process::Ended);
    TS_ASSERT_EQUALS(pgCount.get(), 1);
    TS_ASSERT_EQUALS(toInteger(p.getResult()), 44);
}

/** Test wait, continueProcessWithFailure. */
void
TestInterpreterProcessList::testWaitError()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Process
    Process& p = testee.create(world, "p");
    p.pushFrame(makeStateBCO(Process::Waiting, 44), false);

    // Process group finalizers
    Counter pgCount;
    testee.sig_processGroupFinish.add(&pgCount, &Counter::increment);

    // Run process to Waiting
    int32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(p.getState(), Process::Waiting);
    TS_ASSERT_EQUALS(pgCount.get(), 0);

    testee.continueProcessWithFailure(p, "boom");
    testee.run();

    // Process now terminated
    TS_ASSERT_EQUALS(p.getState(), Process::Failed);
    TS_ASSERT_EQUALS(pgCount.get(), 1);
    TS_ASSERT_EQUALS(String_t(p.getError().what()), "boom");
}

/** Test wait, continueProcessWithFailure, catch. */
void
TestInterpreterProcessList::testWaitCatch()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Process
    Process& p = testee.create(world, "p");

    BCORef_t bco = makeEmptyBCO();
    bco->addInstruction(Opcode::maJump, Opcode::jCatch, 5);                 // pos 0
    addStateSetter(*bco, Process::Waiting);                                 // pos 1+2
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 99);              // pos 3
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialTerminate, 0);  // pos 4
    bco->addInstruction(Opcode::maUnary, interpreter::unLength, 0);
    p.pushFrame(bco, false);

    // Process group finalizers
    Counter pgCount;
    testee.sig_processGroupFinish.add(&pgCount, &Counter::increment);

    // Run process to Waiting
    int32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(p.getState(), Process::Waiting);
    TS_ASSERT_EQUALS(pgCount.get(), 0);

    testee.continueProcessWithFailure(p, "boom");
    testee.run();

    // Process now terminated
    TS_ASSERT_EQUALS(p.getState(), Process::Ended);
    TS_ASSERT_EQUALS(pgCount.get(), 1);
    TS_ASSERT_EQUALS(toInteger(p.getResult()), 4);
}

/** Test wait, Terminate while waiting. */
void
TestInterpreterProcessList::testWaitTerminate()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeStateBCO(Process::Waiting, 44), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // Start one
    testee.startProcessGroup(pgA);
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Waiting);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Terminate the waiting process - this will start the other one
    testee.terminateProcess(p1);
    TS_ASSERT_EQUALS(p1.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), Process::Running);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);
}

/** Test terminateProcess(), removeTerminatedProcesses(). */
void
TestInterpreterProcessList::testRemoveKeep()
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    // Both processes are suspended
    TS_ASSERT_EQUALS(p1.getState(), Process::Suspended);
    TS_ASSERT_EQUALS(p2.getState(), Process::Suspended);

    // Terminate one
    testee.terminateProcess(p1);
    TS_ASSERT_EQUALS(p1.getState(), Process::Terminated);
    TS_ASSERT_EQUALS(p2.getState(), Process::Suspended);

    // Remove it
    testee.removeTerminatedProcesses();
    TS_ASSERT_EQUALS(testee.getProcessList().size(), 1U);
    TS_ASSERT_EQUALS(testee.getProcessList()[0], &p2);
}

/** Test resumeSuspendedProcesses(). */
void
TestInterpreterProcessList::testResume()
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    p1.setState(Process::Frozen);

    // Resume
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeSuspendedProcesses(pgid);
    TS_ASSERT_EQUALS(p1.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Start & run
    testee.startProcessGroup(pgid);
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);
}

/** Test terminateAllProcesses(). */
void
TestInterpreterProcessList::testTerminateAll()
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    p1.setState(Process::Frozen);

    // Terminate
    testee.terminateAllProcesses();
    TS_ASSERT_EQUALS(p1.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(p2.getState(), Process::Terminated);
}

/** Test resumeSuspendedProcesses(), with no applicable processes. */
void
TestInterpreterProcessList::testResumeNone()
{
    interpreter::ProcessList testee;
    Counter ctr;
    testee.sig_processGroupFinish.add(&ctr, &Counter::increment);

    // Resume
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeSuspendedProcesses(pgid);
    TS_ASSERT_EQUALS(ctr.get(), 0);

    testee.startProcessGroup(pgid);
    TS_ASSERT_EQUALS(ctr.get(), 1);
}

/** Test various mismatches.
    Thise are error cases that do not cause a process state to change. */
void
TestInterpreterProcessList::testMismatches()
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    // join: process cannot be Suspended (or Frozen)
    {
        interpreter::ProcessList t;
        Process& p = t.create(world, "p");
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

        t.joinProcess(p, t.allocateProcessGroup());
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);
    }

    // resume: process cannot be Terminated (or Ended, Failed, ...)
    {
        interpreter::ProcessList t;
        Process& p = t.create(world, "p");
        p.setState(Process::Terminated);

        t.resumeProcess(p, t.allocateProcessGroup());
        TS_ASSERT_EQUALS(p.getState(), Process::Terminated);
    }

    // terminate: will overwrite its exit status but not do anything else
    {
        interpreter::ProcessList t;
        Process& p = t.create(world, "p");
        p.setState(Process::Ended);

        t.terminateProcess(p);
        TS_ASSERT_EQUALS(p.getState(), Process::Terminated);
    }

    // continue: cannot continue a suspended process
    {
        interpreter::ProcessList t;
        Process& p = t.create(world, "p");
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

        t.continueProcess(p);
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);
        t.continueProcessWithFailure(p, "f");
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);
    }

    // continue: cannot continue a failed process
    {
        interpreter::ProcessList t;
        Process& p = t.create(world, "p");
        p.setState(Process::Failed);

        t.continueProcess(p);
        TS_ASSERT_EQUALS(p.getState(), Process::Failed);
        t.continueProcessWithFailure(p, "f");
        TS_ASSERT_EQUALS(p.getState(), Process::Failed);
    }
}

/** Test run() with process returning state Frozen. */
void
TestInterpreterProcessList::testRunFreeze()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes in one process group
    uint32_t pgA = testee.allocateProcessGroup();
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeStateBCO(Process::Frozen, 77), false);
    p2.pushFrame(makeEmptyBCO(), false);
    testee.resumeProcess(p1, pgA);
    testee.resumeProcess(p2, pgA);

    // States
    TS_ASSERT_EQUALS(p1.getState(), Process::Runnable);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    TS_ASSERT_EQUALS(p1.getState(), Process::Running);
    TS_ASSERT_EQUALS(p2.getState(), Process::Runnable);

    // Run
    testee.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Frozen);
    TS_ASSERT_EQUALS(p2.getState(), Process::Ended);
}

/** Test object association. */
void
TestInterpreterProcessList::testObject()
{
    // Mocks
    class MyObject : public game::map::Object {
        virtual String_t getName(game::ObjectName /*which*/, afl::string::Translator& /*tx*/, game::InterpreterInterface& /*iface*/) const
            { return "MyObject"; }
        virtual game::Id_t getId() const
            { return 77; }
        virtual bool getOwner(int& /*result*/) const
            { return false; }
        virtual bool getPosition(game::map::Point& /*result*/) const
            { return false; }
    };
    class MyObjectContext : public interpreter::Context {
     public:
        MyObjectContext(MyObject& obj)
            : m_obj(obj)
            { }
        virtual PropertyAccessor* lookup(const afl::data::NameQuery& /*name*/, PropertyIndex_t& /*result*/)
            { return 0; }
        virtual bool next()
            { return false; }
        virtual Context* clone() const
            { return new MyObjectContext(m_obj); }
        virtual game::map::Object* getObject()
            { return &m_obj; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/)
            { }
        virtual String_t toString(bool) const
            { return "#<MyObject>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { TS_FAIL("store unexpected"); }
     private:
        MyObject& m_obj;
    };

    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;
    MyObject obj;

    // Some processes
    /*Process& p1 =*/ testee.create(world, "1");
    Process&   p2 =   testee.create(world, "2");
    /*Process& p3 =*/ testee.create(world, "3");
    p2.pushNewContext(new MyObjectContext(obj));
    p2.markContextTOS();
    obj.markClean();

    // Will not find the process with wrong kind
    TS_ASSERT(testee.getProcessByObject(&obj, Process::pkBaseTask) == 0);

    // Find the process with correct kind
    TS_ASSERT_EQUALS(testee.getProcessByObject(&obj, Process::pkDefault), &p2);

    // Kill it
    TS_ASSERT_EQUALS(obj.isDirty(), false);
    testee.terminateProcess(p2);
    testee.removeTerminatedProcesses();
    TS_ASSERT_EQUALS(obj.isDirty(), true);

    // Will no longer find the process
    TS_ASSERT(testee.getProcessByObject(&obj, Process::pkDefault) == 0);
}

