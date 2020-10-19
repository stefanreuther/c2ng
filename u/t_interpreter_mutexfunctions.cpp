/**
  *  \file u/t_interpreter_mutexfunctions.cpp
  *  \brief Test for interpreter::MutexFunctions
  */

#include "interpreter/mutexfunctions.hpp"

#include "t_interpreter.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/world.hpp"
#include "interpreter/process.hpp"
#include "interpreter/opcode.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/booleanvalue.hpp"

namespace {
    using afl::data::BooleanValue;
    using afl::data::StringValue;
    using interpreter::BCORef_t;
    using interpreter::Opcode;
    using interpreter::Process;

    /* Common environment for all tests. */
    struct Environment {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        interpreter::World world;

        Environment()
            : log(), fs(), world(log, fs)
            { }
    };

    BCORef_t makeBCO()
    {
        BCORef_t result = *new interpreter::BytecodeObject();
        result->setIsProcedure(false);
        return result;
    }

    /* Compile a 'With Lock(<lockName>) Do Stop' command */
    BCORef_t makeTakeLockBCO(String_t lockName, Environment& env)
    {
        BCORef_t result = makeBCO();

        // Lock name
        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);

        // LOCK function. We don't have a GlobalContext here, so pushvar will not work. Look it up manually.
        result->addPushLiteral(env.world.getGlobalValue("LOCK"));

        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return result;
    }

    /* Compile a 'With Lock(<lockName>, <hint>) Do Stop' command */
    BCORef_t makeTakeLockWithHintBCO(String_t lockName, String_t hint, Environment& env)
    {
        BCORef_t result = makeBCO();

        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);

        afl::data::StringValue hintSV(hint);
        result->addPushLiteral(&hintSV);
        result->addPushLiteral(env.world.getGlobalValue("LOCK"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 2);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return result;
    }

    /* Compile a 'GetLockInfo(<lockName>, <type>)' command */
    BCORef_t makeGetLockInfoBCO(String_t lockName, uint16_t type, Environment& env)
    {
        BCORef_t result = makeBCO();
        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maPush, Opcode::sInteger, type);
        result->addPushLiteral(env.world.getGlobalValue("GETLOCKINFO"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 2);
        return result;
    }

    bool toBoolean(const Process& proc)
    {
        const BooleanValue* bv = dynamic_cast<const BooleanValue*>(proc.getResult());
        if (bv == 0) {
            throw interpreter::Error::typeError();
        }
        return bv->getValue();
    }

    String_t toString(const Process& proc)
    {
        const StringValue* sv = dynamic_cast<const StringValue*>(proc.getResult());
        if (sv == 0) {
            throw interpreter::Error::typeError();
        }
        return sv->getValue();
    }
}

/** Test preconditions.

    A: Create a test environment.
    E: World's constructor must have called registerMutexFunctions; verify that functions are there. */
void
TestInterpreterMutexFunctions::testInit()
{
    Environment env;
    TS_ASSERT(env.world.getGlobalValue("LOCK") != 0);
    TS_ASSERT(env.world.getGlobalValue("GETLOCKINFO") != 0);
}

/** Test taking a lock.

    A: create a test environment. Start process that takes a lock.
    E: Lock must register as taken. */
void
TestInterpreterMutexFunctions::testTakeLock()
{
    Environment env;

    // Run process
    Process p(env.world, "pro", 42);
    p.pushFrame(makeTakeLockBCO("LNAME", env), true);
    p.run();
    TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

    // Verify lock status
    interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
    TS_ASSERT(mtx);
    TS_ASSERT_EQUALS(mtx->getOwner(), &p);
}

/** Test locking conflict.

    A: create a test environment. Start two processes taking the same lock.
    E: second process must report an error. */
void
TestInterpreterMutexFunctions::testTakeLockConflict()
{
    Environment env;

    // Run process
    Process p1(env.world, "pro", 42);
    p1.pushFrame(makeTakeLockBCO("LNAME", env), true);
    p1.run();
    TS_ASSERT_EQUALS(p1.getState(), Process::Suspended);

    // Run another process that wishes to take that lock
    Process p2(env.world, "bro", 44);
    p2.pushFrame(makeTakeLockBCO("LNAME", env), true);
    p2.run();
    TS_ASSERT_EQUALS(p2.getState(), Process::Failed);
}

/** Test implicit lock release.

    A: create a test environment. Run a process taking a lock. Remove the process object.
    E: lock must be freed when the process object dies. */
void
TestInterpreterMutexFunctions::testReleaseLockOnExit()
{
    Environment env;

    // Run process
    {
        Process p(env.world, "pro", 42);
        p.pushFrame(makeTakeLockBCO("LNAME", env), true);
        p.run();
        TS_ASSERT_EQUALS(p.getState(), Process::Suspended);

        // Verify lock status
        interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
        TS_ASSERT(mtx);
        TS_ASSERT_EQUALS(mtx->getOwner(), &p);
    }

    // Process gone, so lock must also be gone
    interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
    TS_ASSERT(mtx == 0);
}

/** Test GetLockInfo(,0).

    A: create a test environment. Run a process taking a lock. Call GetLockInfo(,0).
    E: must return true */
void
TestInterpreterMutexFunctions::testGetLockInfo0()
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockBCO("LNAME", env), true);
    taker.run();

    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 0, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toBoolean(querier), true);
}

/** Test GetLockInfo(,1).

    A: create a test environment. Run a process taking a lock. Call GetLockInfo(,1).
    E: must return name or process owning the lock. */
void
TestInterpreterMutexFunctions::testGetLockInfo1()
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockBCO("LNAME", env), true);
    taker.run();

    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 1, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(querier), "pro");
}

/** Test GetLockInfo(,2).

    A: create a test environment. Run a process taking a lock with hint. Call GetLockInfo(,2).
    E: must return the hint */
void
TestInterpreterMutexFunctions::testGetLockInfo2()
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockWithHintBCO("HNAME", "Hint!", env), true);
    taker.run();

    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("HNAME", 2, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(querier), "Hint!");
}

/** Test GetLockInfo(,0), idle/failure case.

    A: create a test environment. Call GetLockInfo(,0).
    E: must return false */
void
TestInterpreterMutexFunctions::testGetLockInfoFail0()
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 0, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toBoolean(querier), false);
}

/** Test GetLockInfo(,1), idle/failure case.

    A: create a test environment. Call GetLockInfo(,1).
    E: must return null */
void
TestInterpreterMutexFunctions::testGetLockInfoFail1()
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 1, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT(querier.getResult() == 0);
}

/** Test GetLockInfo(,2), idle/failure case.

    A: create a test environment. Call GetLockInfo(,2).
    E: must return null */
void
TestInterpreterMutexFunctions::testGetLockInfoFail2()
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 2, env), true);
    querier.run();
    TS_ASSERT_EQUALS(querier.getState(), Process::Ended);
    TS_ASSERT(querier.getResult() == 0);
}

/** Test failure case: Lock(Empty).

    A: create a test environment. Call 'Lock(EMPTY)'.
    E: must report an error. */
void
TestInterpreterMutexFunctions::testFailNull()
{
    Environment env;
    Process p(env.world, "p", 1);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(0);
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
    p.pushFrame(bco, true);
    p.run();

    TS_ASSERT_EQUALS(p.getState(), Process::Failed);
}

/** Test failure case: Lock(Empty).

    A: create a test environment. Execute 'ForEach Lock'.
    E: must report an error. */
void
TestInterpreterMutexFunctions::testFailIter()
{
    Environment env;
    Process p(env.world, "p", 1);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirst, 0);
    p.pushFrame(bco, true);
    p.run();

    TS_ASSERT_EQUALS(p.getState(), Process::Failed);
}

/** Test border case: Dim(Lock).

    A: create a test environment. Execute 'Dim(Lock,1)'.
    E: must report an error (Lock has no dimensions). */
void
TestInterpreterMutexFunctions::testDim()
{
    Environment env;
    Process p(env.world, "p", 1);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    bco->addInstruction(Opcode::maBinary, interpreter::biArrayDim, 0);
    p.pushFrame(bco, true);
    p.run();

    TS_ASSERT_EQUALS(p.getState(), Process::Failed);
}

/** Test border case: Str(Lock).

    A: create a test environment. Execute 'Str(Lock)'.
    E: must report correct value. */
void
TestInterpreterMutexFunctions::testToString()
{
    Environment env;
    Process p(env.world, "p", 1);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maUnary, interpreter::unStr, 0);
    p.pushFrame(bco, true);
    p.run();

    TS_ASSERT_EQUALS(p.getState(), Process::Ended);
    TS_ASSERT_EQUALS(toString(p), "Lock");
}

