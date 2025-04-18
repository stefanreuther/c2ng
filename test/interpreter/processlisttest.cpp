/**
  *  \file test/interpreter/processlisttest.cpp
  *  \brief Test for interpreter::ProcessList
  */

#include "interpreter/processlist.hpp"

#include <stdexcept>
#include "afl/data/integervalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/counter.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/simplecontext.hpp"
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
            virtual size_t getDimension(size_t) const
                { return 0; }
            virtual interpreter::Context* makeFirstContext()
                { return 0; }
            virtual StateSetter* clone() const
                { return new StateSetter(m_state); }
            virtual String_t toString(bool) const
                { return "#<state>"; }
            virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
                { throw std::runtime_error("store unexpected"); }

         private:
            Process::State m_state;
        };
        StateSetter sts(st);

        bco.addPushLiteral(&sts);
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
    }

    BCORef_t makeSuspendBCO()
    {
        BCORef_t bco = interpreter::BytecodeObject::create(true);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return bco;
    }

    BCORef_t makeFailBCO()
    {
        BCORef_t bco = interpreter::BytecodeObject::create(true);
        bco->addInstruction(Opcode::maPush,    Opcode::sInteger, 0);
        bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
        return bco;
    }

    BCORef_t makeEmptyBCO()
    {
        BCORef_t bco = interpreter::BytecodeObject::create(true);
        return bco;
    }

    // Make a BCO that first sets state st, then pushes integer n.
    BCORef_t makeStateBCO(Process::State st, uint16_t n)
    {
        BCORef_t bco = interpreter::BytecodeObject::create(true);
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
AFL_TEST_NOARG("interpreter.ProcessList:run:empty")
{
    interpreter::ProcessList testee;
    testee.run(0);
    testee.removeTerminatedProcesses();
}

/** Test empty process list: signaling.
    A process group that becomes empty must be signalled correctly. */
AFL_TEST("interpreter.ProcessList:run:empty-process-group", a)
{
    interpreter::ProcessList testee;
    Counter c;
    a.checkEqual("01. signal count", c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Just running does nothing
    testee.run(0);
    a.checkEqual("11. signal count", c.get(), 0);

    // Running an empty process group signals once
    testee.startProcessGroup(testee.allocateProcessGroup());
    testee.run(0);
    a.checkEqual("21. signal count", c.get(), 1);
}

/** Test allocateProcessGroup().
    Id allocation must produce different Ids. */
AFL_TEST("interpreter.ProcessList:allocateProcessGroup", a)
{
    interpreter::ProcessList testee;

    // Process groups
    uint32_t ga = testee.allocateProcessGroup();
    uint32_t gb = testee.allocateProcessGroup();
    uint32_t gc = testee.allocateProcessGroup();
    a.checkDifferent("01. allocateProcessGroup", ga, gb);
    a.checkDifferent("02. allocateProcessGroup", gb, gc);
    a.checkDifferent("03. allocateProcessGroup", gc, ga);

    // Process Ids
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    Process& pa = testee.create(world, "a");
    Process& pb = testee.create(world, "b");
    Process& pc = testee.create(world, "c");
    a.checkDifferent("11. getProcessId", pa.getProcessId(), pb.getProcessId());
    a.checkDifferent("12. getProcessId", pb.getProcessId(), pc.getProcessId());
    a.checkDifferent("13. getProcessId", pc.getProcessId(), pa.getProcessId());
}

/** Test execution vs suspension.
    A suspending process causes the process group to signal. */
AFL_TEST("interpreter.ProcessList:suspend", a)
{
    interpreter::ProcessList testee;

    // Add a process that will suspend
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    Process& p = testee.create(world, "testSuspend");

    p.pushFrame(makeSuspendBCO(), false);
    a.checkEqual("01. getState", p.getState(), Process::Suspended);

    // Prepare execution
    Counter c;
    a.checkEqual("11. signal count", c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Nothing scheduled yet
    testee.run(0);
    a.checkEqual("21. signal count", c.get(), 0);
    a.checkEqual("22. getState", p.getState(), Process::Suspended);

    // Resume it. Will still not run because we didn't start it.
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeProcess(p, pgid);
    a.checkEqual("31. getState", p.getState(), Process::Runnable);
    testee.run(0);
    a.checkEqual("32. signal count", c.get(), 0);
    a.checkEqual("33. getState", p.getState(), Process::Runnable);

    // Start it! This must run the process until it suspends
    testee.startProcessGroup(pgid);
    testee.run(0);
    a.checkEqual("41. signal count", c.get(), 1);
    a.checkEqual("42. getState", p.getState(), Process::Suspended);
}

/** Test joinProcess().
    Moving a process from one process group to another joins the process groups. */
AFL_TEST("interpreter.ProcessList:joinProcess", a)
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
    testee.run(0);

    // All processes terminated now
    a.checkEqual("01. getState", p1.getState(), Process::Ended);
    a.checkEqual("02. getState", p2.getState(), Process::Ended);
    a.checkEqual("03. getState", p3.getState(), Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    a.check("11. empty", testee.getProcessList().empty());
}

/** Test execution with failing processes.
    If a process fails, the next one from its process group executes. */
AFL_TEST("interpreter.ProcessList:process-failure", a)
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
    a.checkEqual("01. getState", p1.getState(), Process::Runnable);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    a.checkEqual("11. getState", p1.getState(), Process::Running);
    a.checkEqual("12. getState", p2.getState(), Process::Runnable);

    // Run
    testee.run(0);
    a.checkEqual("21. getState", p1.getState(), Process::Failed);
    a.checkEqual("22. getState", p2.getState(), Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    a.check("31. empty", testee.getProcessList().empty());
}

/** Test termination.
    If a process that is about to run is terminated, the next one from its process group executes. */
AFL_TEST("interpreter.ProcessList:termination", a)
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
    a.checkEqual("01. getState", p1.getState(), Process::Runnable);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    a.checkEqual("11. getState", p1.getState(), Process::Running);
    a.checkEqual("12. getState", p2.getState(), Process::Runnable);

    // Terminate
    testee.terminateProcess(p1);
    a.checkEqual("21. getState", p1.getState(), Process::Terminated);
    a.checkEqual("22. getState", p2.getState(), Process::Running);

    // Run
    testee.run(0);
    a.checkEqual("31. getState", p1.getState(), Process::Terminated);
    a.checkEqual("32. getState", p2.getState(), Process::Ended);

    // Reap zombies
    testee.removeTerminatedProcesses();
    a.check("41. empty", testee.getProcessList().empty());
}

/** Test priority handling. */
AFL_TEST("interpreter.ProcessList:priority", a)
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
    a.checkEqual("01. getPriority", p1.getPriority(), 50);
    a.checkEqual("02. getPriority", p2.getPriority(), 50);
    a.checkEqual("03. getPriority", p3.getPriority(), 50);
    a.checkEqual("04. list item", testee.getProcessList()[0], &p1);
    a.checkEqual("05. list item", testee.getProcessList()[1], &p2);
    a.checkEqual("06. list item", testee.getProcessList()[2], &p3);
    a.checkEqual("07. findProcessById", testee.findProcessById(p1.getProcessId()), &p1);
    a.checkEqual("08. findProcessById", testee.findProcessById(p2.getProcessId()), &p2);
    a.checkEqual("09. findProcessById", testee.findProcessById(p3.getProcessId()), &p3);

    uint32_t unknownPID = (p1.getProcessId() | p2.getProcessId() | p3.getProcessId()) + 1;
    a.checkNull("11. findProcessById", testee.findProcessById(unknownPID));

    // Null operation on 2's priority
    p2.setPriority(50);
    testee.handlePriorityChange(p2);
    a.checkEqual("21. list item", testee.getProcessList()[0], &p1);
    a.checkEqual("22. list item", testee.getProcessList()[1], &p2);
    a.checkEqual("23. list item", testee.getProcessList()[2], &p3);

    // Improve 2's priority --> [2,1,3]
    p2.setPriority(10);
    testee.handlePriorityChange(p2);
    a.checkEqual("31. list item", testee.getProcessList()[0], &p2);
    a.checkEqual("32. list item", testee.getProcessList()[1], &p1);
    a.checkEqual("33. list item", testee.getProcessList()[2], &p3);

    // Improve 3's priority --> [2,3,1]
    p3.setPriority(10);
    testee.handlePriorityChange(p3);
    a.checkEqual("41. list item", testee.getProcessList()[0], &p2);
    a.checkEqual("42. list item", testee.getProcessList()[1], &p3);
    a.checkEqual("43. list item", testee.getProcessList()[2], &p1);

    // Drop 2's priority to same value as 1 --> [3,1,2]
    p2.setPriority(50);
    testee.handlePriorityChange(p2);
    a.checkEqual("51. list item", testee.getProcessList()[0], &p3);
    a.checkEqual("52. list item", testee.getProcessList()[1], &p1);
    a.checkEqual("53. list item", testee.getProcessList()[2], &p2);
}

/** Test end signalisation. */
AFL_TEST("interpreter.ProcessList:end-signal", a)
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
    testee.run(0);

    a.checkEqual("01. finalizeCount", finalizeCount, 2);
    a.checkEqual("02. signal count", pgCount.get(), 1);
}

/** Test wait, continueProcess.
    This is what happens when a process calls UI. */
AFL_TEST("interpreter.ProcessList:continueProcess", a)
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
    testee.run(0);
    a.checkEqual("01. getState", p.getState(), Process::Waiting);
    a.checkEqual("02. get", pgCount.get(), 0);

    testee.continueProcess(p);
    testee.run(0);

    // Process now terminated
    a.checkEqual("11. getState", p.getState(), Process::Ended);
    a.checkEqual("12. get", pgCount.get(), 1);
    a.checkEqual("13. result", toInteger(p.getResult()), 44);
}

/** Test wait, continueProcessWithFailure. */
AFL_TEST("interpreter.ProcessList:continueProcessWithFailure", a)
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
    testee.run(0);
    a.checkEqual("01. getState", p.getState(), Process::Waiting);
    a.checkEqual("02. get", pgCount.get(), 0);

    testee.continueProcessWithFailure(p, "boom");
    testee.run(0);

    // Process now terminated
    a.checkEqual("11. getState", p.getState(), Process::Failed);
    a.checkEqual("12. get", pgCount.get(), 1);
    a.checkEqual("13. getError", String_t(p.getError().what()), "boom");
}

/** Test wait, continueProcessWithFailure, catch. */
AFL_TEST("interpreter.ProcessList:continueProcessWithFailure:catch", a)
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
    testee.run(0);
    a.checkEqual("01. getState", p.getState(), Process::Waiting);
    a.checkEqual("02. get", pgCount.get(), 0);

    testee.continueProcessWithFailure(p, "boom");
    testee.run(0);

    // Process now terminated
    a.checkEqual("11. getState", p.getState(), Process::Ended);
    a.checkEqual("12. get", pgCount.get(), 1);
    a.checkEqual("13. result", toInteger(p.getResult()), 4);
}

/** Test wait, Terminate while waiting. */
AFL_TEST("interpreter.ProcessList:terminateProcess:while-waiting", a)
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
    testee.run(0);
    a.checkEqual("01. getState", p1.getState(), Process::Waiting);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);

    // Terminate the waiting process - this will start the other one
    testee.terminateProcess(p1);
    a.checkEqual("11. getState", p1.getState(), Process::Terminated);
    a.checkEqual("12. getState", p2.getState(), Process::Running);

    // Run
    testee.run(0);
    a.checkEqual("21. getState", p1.getState(), Process::Terminated);
    a.checkEqual("22. getState", p2.getState(), Process::Ended);
}

/** Test terminateProcess(), removeTerminatedProcesses(). */
AFL_TEST("interpreter.ProcessList:terminateProcess", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    // Both processes are suspended
    a.checkEqual("01. getState", p1.getState(), Process::Suspended);
    a.checkEqual("02. getState", p2.getState(), Process::Suspended);

    // Terminate one
    testee.terminateProcess(p1);
    a.checkEqual("11. getState", p1.getState(), Process::Terminated);
    a.checkEqual("12. getState", p2.getState(), Process::Suspended);

    // Remove it
    testee.removeTerminatedProcesses();
    a.checkEqual("21. getProcessList", testee.getProcessList().size(), 1U);
    a.checkEqual("22. list item", testee.getProcessList()[0], &p2);
}

/** Test resumeSuspendedProcesses(). */
AFL_TEST("interpreter.ProcessList:resumeSuspendedProcesses", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    p1.setState(Process::Frozen);

    // Resume
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeSuspendedProcesses(pgid);
    a.checkEqual("01. getState", p1.getState(), Process::Frozen);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);

    // Start & run
    testee.startProcessGroup(pgid);
    testee.run(0);
    a.checkEqual("11. getState", p1.getState(), Process::Frozen);
    a.checkEqual("12. getState", p2.getState(), Process::Ended);
}

/** Test terminateAllProcesses(). */
AFL_TEST("interpreter.ProcessList:terminateAllProcesses", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;

    // Two processes
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    p1.pushFrame(makeEmptyBCO(), false);
    p2.pushFrame(makeEmptyBCO(), false);

    p1.setState(Process::Frozen);

    // Terminate
    testee.terminateAllProcesses();
    a.checkEqual("01. getState", p1.getState(), Process::Frozen);
    a.checkEqual("02. getState", p2.getState(), Process::Terminated);
}

/** Test resumeSuspendedProcesses(), with no applicable processes. */
AFL_TEST("interpreter.ProcessList:resumeSuspendedProcesses:empty-pg", a)
{
    interpreter::ProcessList testee;
    Counter ctr;
    testee.sig_processGroupFinish.add(&ctr, &Counter::increment);

    // Resume
    uint32_t pgid = testee.allocateProcessGroup();
    testee.resumeSuspendedProcesses(pgid);
    a.checkEqual("01. get", ctr.get(), 0);

    testee.startProcessGroup(pgid);
    a.checkEqual("11. get", ctr.get(), 1);
}

/*
 *  Test various mismatches.
 *  These are error cases that do not cause a process state to change.
 */

// join: process cannot be Suspended (or Frozen)
AFL_TEST("interpreter.ProcessList:joinProcess:suspended", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList t;
    Process& p = t.create(world, "p");
    a.checkEqual("01. getState", p.getState(), Process::Suspended);

    t.joinProcess(p, t.allocateProcessGroup());
    a.checkEqual("11. getState", p.getState(), Process::Suspended);
}

// resume: process cannot be Terminated (or Ended, Failed, ...)
AFL_TEST("interpreter.ProcessList:resumeProcess:terminated", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList t;
    Process& p = t.create(world, "p");
    p.setState(Process::Terminated);

    t.resumeProcess(p, t.allocateProcessGroup());
    a.checkEqual("01. getState", p.getState(), Process::Terminated);
}

// terminate: will overwrite its exit status but not do anything else
AFL_TEST("interpreter.ProcessList:terminateProcess:ended", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList t;
    Process& p = t.create(world, "p");
    p.setState(Process::Ended);

    t.terminateProcess(p);
    a.checkEqual("01. getState", p.getState(), Process::Terminated);
}

// continue: cannot continue a suspended process
AFL_TEST("interpreter.ProcessList:continueProcess:suspended", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList t;
    Process& p = t.create(world, "p");
    a.checkEqual("01. getState", p.getState(), Process::Suspended);

    t.continueProcess(p);
    a.checkEqual("11. getState", p.getState(), Process::Suspended);
    t.continueProcessWithFailure(p, "f");
    a.checkEqual("12. getState", p.getState(), Process::Suspended);
}

// continue: cannot continue a failed process
AFL_TEST("interpreter.ProcessList:continueProcess:failed", a)
{
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    interpreter::ProcessList t;
    Process& p = t.create(world, "p");
    p.setState(Process::Failed);

    t.continueProcess(p);
    a.checkEqual("01. getState", p.getState(), Process::Failed);
    t.continueProcessWithFailure(p, "f");
    a.checkEqual("02. getState", p.getState(), Process::Failed);
}

/** Test run() with process returning state Frozen. */
AFL_TEST("interpreter.ProcessList:process-freezes-itself", a)
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
    a.checkEqual("01. getState", p1.getState(), Process::Runnable);
    a.checkEqual("02. getState", p2.getState(), Process::Runnable);

    // Start one
    testee.startProcessGroup(pgA);
    a.checkEqual("11. getState", p1.getState(), Process::Running);
    a.checkEqual("12. getState", p2.getState(), Process::Runnable);

    // Run
    testee.run(0);
    a.checkEqual("21. getState", p1.getState(), Process::Frozen);
    a.checkEqual("22. getState", p2.getState(), Process::Ended);
}

/** Test object association. */
AFL_TEST("interpreter.ProcessList:findProcessByObject", a)
{
    // Mocks
    class MyObject : public afl::base::Deletable {
     public:
        MyObject()
            : m_dirty()
            { }
        bool isDirty()
            { return m_dirty; }
        void markDirty()
            { m_dirty = true; }
        void markClean()
            { m_dirty = false; }
     private:
        bool m_dirty;
    };
    class MyObjectContext : public interpreter::SimpleContext {
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
        virtual MyObject* getObject()
            { return &m_obj; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }
        virtual String_t toString(bool) const
            { return "#<MyObject>"; }
        virtual void store(interpreter::TagNode& /*out*/, afl::io::DataSink& /*aux*/, interpreter::SaveContext& /*ctx*/) const
            { throw std::runtime_error("store unexpected"); }
     private:
        MyObject& m_obj;
    };

    // We need now explicitly
    struct Listener {
        static void onProcessStateChange(const interpreter::Process& proc, bool)
            {
                if (MyObject* p = dynamic_cast<MyObject*>(proc.getInvokingObject())) {
                    p->markDirty();
                }
            }
    };

    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;
    testee.sig_processStateChange.add(&Listener::onProcessStateChange);
    MyObject obj;

    // Some processes
    /*Process& p1 =*/ testee.create(world, "1");
    Process&   p2 =   testee.create(world, "2");
    /*Process& p3 =*/ testee.create(world, "3");
    p2.pushNewContext(new MyObjectContext(obj));
    p2.markContextTOS();
    obj.markClean();

    // Will not find the process with wrong kind
    a.checkNull("01. findProcessByObject pkBaseTask", testee.findProcessByObject(&obj, Process::pkBaseTask));

    // Find the process with correct kind
    a.checkEqual("11. findProcessByObject pkDefault", testee.findProcessByObject(&obj, Process::pkDefault), &p2);

    // Kill it
    a.checkEqual("21. isDirty", obj.isDirty(), false);
    testee.terminateProcess(p2);
    testee.removeTerminatedProcesses();
    a.checkEqual("22. isDirty", obj.isDirty(), true);

    // Will no longer find the process
    a.checkNull("31. findProcessByObject pkDefault", testee.findProcessByObject(&obj, Process::pkDefault));
}

/** Test terminating empty process group. */
AFL_TEST("interpreter.ProcessList:terminateProcessGroup:empty", a)
{
    interpreter::ProcessList testee;
    Counter c;
    a.checkEqual("01. signal count", c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    testee.terminateProcessGroup(42);
    a.checkEqual("11. signal count", c.get(), 1);
}

/** Test terminating non-empty process group. */
AFL_TEST("interpreter.ProcessList:terminateProcessGroup:populated", a)
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);

    interpreter::ProcessList testee;
    Counter c;
    a.checkEqual("01. signal count", c.get(), 0);
    testee.sig_processGroupFinish.add(&c, &Counter::increment);

    // Set up processes
    Process& p1 = testee.create(world, "1");
    Process& p2 = testee.create(world, "2");
    Process& p3 = testee.create(world, "3");
    testee.resumeProcess(p1, 42);
    testee.resumeProcess(p2, 23);
    testee.resumeProcess(p3, 42);

    // Terminate
    testee.terminateProcessGroup(42);
    a.checkEqual("11. signal count", c.get(), 1);

    // Verify state
    a.checkEqual("21. state", p1.getState(), Process::Terminated);
    a.checkEqual("22. state", p2.getState(), Process::Runnable);
    a.checkEqual("23. state", p3.getState(), Process::Terminated);
}
