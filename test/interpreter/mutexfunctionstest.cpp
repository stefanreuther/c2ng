/**
  *  \file test/interpreter/mutexfunctionstest.cpp
  *  \brief Test for interpreter::MutexFunctions
  */

#include "interpreter/mutexfunctions.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/process.hpp"
#include "interpreter/singlecontext.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/world.hpp"

namespace {
    using afl::data::BooleanValue;
    using afl::data::StringValue;
    using interpreter::BCORef_t;
    using interpreter::Opcode;
    using interpreter::Process;

    /* A simple replacement for GlobalContext */
    class SimpleGlobalContext : public interpreter::SingleContext, public interpreter::Context::ReadOnlyAccessor {
     public:
        SimpleGlobalContext(interpreter::World& w)
            : m_world(w)
            { }

        // Context:
        virtual Context::PropertyAccessor* lookup(const afl::data::NameQuery& name, PropertyIndex_t& result)
            {
                afl::data::NameMap::Index_t ix = m_world.globalPropertyNames().getIndexByName(name);
                if (ix != afl::data::NameMap::nil) {
                    result = ix;
                    return this;
                } else {
                    return 0;
                }
            }
        virtual afl::data::Value* get(PropertyIndex_t index)
            { return afl::data::Value::cloneOf(m_world.globalValues().get(index)); }
        virtual SimpleGlobalContext* clone() const
            { return new SimpleGlobalContext(m_world); }
        virtual afl::base::Deletable* getObject()
            { return 0; }
        virtual void enumProperties(interpreter::PropertyAcceptor& /*acceptor*/) const
            { }

        // BaseValue:
        virtual String_t toString(bool /*readable*/) const
            { return "#<global>"; }
        virtual void store(interpreter::TagNode& out, afl::io::DataSink& aux, interpreter::SaveContext& ctx) const
            { rejectStore(out, aux, ctx); }

     private:
        interpreter::World& m_world;
    };

    /* Common environment for all tests. */
    struct Environment {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;

        Environment()
            : log(), tx(), fs(), world(log, tx, fs)
            { world.addNewGlobalContext(new SimpleGlobalContext(world)); }
    };

    BCORef_t makeBCO()
    {
        return interpreter::BytecodeObject::create(false);
    }

    /* Wrap a BCO to execute with the dummy mutex functions */
    BCORef_t makeDummyBCO(BCORef_t child)
    {
        BCORef_t result = makeBCO();
        registerDummyMutexFunctions(*result);

        interpreter::SubroutineValue sv(child);
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 0);
        return result;
    }

    /* Compile a 'With Lock(<lockName>) Do Stop' command */
    BCORef_t makeTakeLockBCO(String_t lockName)
    {
        BCORef_t result = makeBCO();

        // Lock name
        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maPush, Opcode::sNamedVariable, result->addName("LOCK"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return result;
    }

    /* Compile a 'With Lock(<lockName>, <hint>) Do Stop' command */
    BCORef_t makeTakeLockWithHintBCO(String_t lockName, String_t hint)
    {
        BCORef_t result = makeBCO();

        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);

        afl::data::StringValue hintSV(hint);
        result->addPushLiteral(&hintSV);
        result->addInstruction(Opcode::maPush, Opcode::sNamedVariable, result->addName("LOCK"));
        result->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 2);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        result->addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
        return result;
    }

    /* Compile a 'GetLockInfo(<lockName>, <type>)' command */
    BCORef_t makeGetLockInfoBCO(String_t lockName, uint16_t type)
    {
        BCORef_t result = makeBCO();
        afl::data::StringValue sv(lockName);
        result->addPushLiteral(&sv);
        result->addInstruction(Opcode::maPush, Opcode::sInteger, type);
        result->addInstruction(Opcode::maPush, Opcode::sNamedVariable, result->addName("GETLOCKINFO"));
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
AFL_TEST("interpreter.MutexFunctions:preconditions", a)
{
    Environment env;
    a.checkNonNull("01. LOCK",        env.world.getGlobalValue("LOCK"));
    a.checkNonNull("02. GETLOCKINFO", env.world.getGlobalValue("GETLOCKINFO"));
}

/** Test taking a lock.

    A: create a test environment. Start process that takes a lock.
    E: Lock must register as taken. */
AFL_TEST("interpreter.MutexFunctions:take-lock", a)
{
    Environment env;

    // Run process
    Process p(env.world, "pro", 42);
    p.pushFrame(makeTakeLockBCO("LNAME"), true);
    p.run(0);
    a.checkEqual("01. getState", p.getState(), Process::Suspended);

    // Verify lock status
    interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
    a.check("11. mtx", mtx);
    a.checkEqual("12. getOwner", mtx->getOwner(), &p);
}

/** Test locking conflict.

    A: create a test environment. Start two processes taking the same lock.
    E: second process must report an error. */
AFL_TEST("interpreter.MutexFunctions:locking-conflict", a)
{
    Environment env;

    // Run process
    Process p1(env.world, "pro", 42);
    p1.pushFrame(makeTakeLockBCO("LNAME"), true);
    p1.run(0);
    a.checkEqual("01. getState", p1.getState(), Process::Suspended);

    // Run another process that wishes to take that lock
    {
        Process p2(env.world, "bro", 44);
        p2.pushFrame(makeTakeLockBCO("LNAME"), true);
        p2.run(0);
        a.checkEqual("11. getState", p2.getState(), Process::Failed);
    }

    // Run another process that wishes to take that lock - dummy version
    {
        Process p2(env.world, "bro", 44);
        p2.pushFrame(makeDummyBCO(makeTakeLockBCO("LNAME")), true);
        p2.run(0);
        a.checkEqual("21. getState", p2.getState(), Process::Suspended);
    }
}

/** Test implicit lock release.

    A: create a test environment. Run a process taking a lock. Remove the process object.
    E: lock must be freed when the process object dies. */
AFL_TEST("interpreter.MutexFunctions:implicit-lock-release", a)
{
    Environment env;

    // Run process
    {
        Process p(env.world, "pro", 42);
        p.pushFrame(makeTakeLockBCO("LNAME"), true);
        p.run(0);
        a.checkEqual("01. getState", p.getState(), Process::Suspended);

        // Verify lock status
        interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
        a.check("11. mtx", mtx);
        a.checkEqual("12. getOwner", mtx->getOwner(), &p);
    }

    // Process gone, so lock must also be gone
    interpreter::MutexList::Mutex* mtx = env.world.mutexList().query("LNAME");
    a.checkNull("21. mtx", mtx);
}

/** Test GetLockInfo(,0).

    A: create a test environment. Run a process taking a lock. Call GetLockInfo(,0).
    E: must return true */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:0", a)
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockBCO("LNAME"), true);
    taker.run(0);

    // Real
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeGetLockInfoBCO("LNAME", 0), true);
        querier.run(0);
        a.checkEqual("01. getState", querier.getState(), Process::Ended);
        a.checkEqual("02. result", toBoolean(querier), true);
    }

    // Dummy
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeDummyBCO(makeGetLockInfoBCO("LNAME", 0)), true);
        querier.run(0);
        a.checkEqual("11. getState", querier.getState(), Process::Ended);
        a.checkEqual("12. result", toBoolean(querier), false);
    }
}

/** Test GetLockInfo(,1).

    A: create a test environment. Run a process taking a lock. Call GetLockInfo(,1).
    E: must return name or process owning the lock. */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:1", a)
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockBCO("LNAME"), true);
    taker.run(0);

    // Real
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeGetLockInfoBCO("LNAME", 1), true);
        querier.run(0);
        a.checkEqual("01. getState", querier.getState(), Process::Ended);
        a.checkEqual("02. result", toString(querier), "pro");
    }

    // Dummy
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeDummyBCO(makeGetLockInfoBCO("LNAME", 1)), true);
        querier.run(0);
        a.checkEqual("11. getState", querier.getState(), Process::Ended);
        a.checkNull("12. result", querier.getResult());
    }
}

/** Test GetLockInfo(,2).

    A: create a test environment. Run a process taking a lock with hint. Call GetLockInfo(,2).
    E: must return the hint */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:2", a)
{
    Environment env;
    Process taker(env.world, "pro", 42);
    taker.pushFrame(makeTakeLockWithHintBCO("HNAME", "Hint!"), true);
    taker.run(0);

    // Real
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeGetLockInfoBCO("HNAME", 2), true);
        querier.run(0);
        a.checkEqual("01. getState", querier.getState(), Process::Ended);
        a.checkEqual("02. result", toString(querier), "Hint!");
    }

    // Dummy
    {
        Process querier(env.world, "q", 77);
        querier.pushFrame(makeDummyBCO(makeGetLockInfoBCO("HNAME", 2)), true);
        querier.run(0);
        a.checkEqual("11. getState", querier.getState(), Process::Ended);
        a.checkNull("12. result", querier.getResult());
    }
}

/** Test GetLockInfo(,0), idle/failure case.

    A: create a test environment. Call GetLockInfo(,0).
    E: must return false */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:0:fail", a)
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 0), true);
    querier.run(0);
    a.checkEqual("01. getState", querier.getState(), Process::Ended);
    a.checkEqual("02. result", toBoolean(querier), false);
}

/** Test GetLockInfo(,1), idle/failure case.

    A: create a test environment. Call GetLockInfo(,1).
    E: must return null */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:1:fail", a)
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 1), true);
    querier.run(0);
    a.checkEqual("01. getState", querier.getState(), Process::Ended);
    a.checkNull("02. result", querier.getResult());
}

/** Test GetLockInfo(,2), idle/failure case.

    A: create a test environment. Call GetLockInfo(,2).
    E: must return null */
AFL_TEST("interpreter.MutexFunctions:GetLockInfo:2:fail", a)
{
    Environment env;
    Process querier(env.world, "q", 77);
    querier.pushFrame(makeGetLockInfoBCO("LNAME", 2), true);
    querier.run(0);
    a.checkEqual("01. getState", querier.getState(), Process::Ended);
    a.checkNull("02. result", querier.getResult());
}

/** Test failure case: Lock(Empty).

    A: create a test environment. Call 'Lock(EMPTY)'.
    E: must report an error. */
AFL_TEST("interpreter.MutexFunctions:Lock:null", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(0);
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);

    // Real
    {
        Process p(env.world, "p", 1);
        p.pushFrame(bco, true);
        p.run(0);
        a.checkEqual("01. getState", p.getState(), Process::Failed);
    }

    // Dummy
    {
        Process p(env.world, "p", 1);
        p.pushFrame(makeDummyBCO(bco), true);
        p.run(0);
        a.checkEqual("11. getState", p.getState(), Process::Failed);
    }
}

/** Test failure case: ForEach Lock.

    A: create a test environment. Execute 'ForEach Lock'.
    E: must report an error. */
AFL_TEST("interpreter.MutexFunctions:Lock:loop", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maSpecial, Opcode::miSpecialFirst, 0);

    // Real
    {
        Process p(env.world, "p", 1);
        p.pushFrame(bco, true);
        p.run(0);
        a.checkEqual("01. getState", p.getState(), Process::Failed);
    }

    // Dummy
    {
        Process p(env.world, "p", 1);
        p.pushFrame(makeDummyBCO(bco), true);
        p.run(0);
        a.checkEqual("11. getState", p.getState(), Process::Failed);
    }
}

/** Test border case: Dim(Lock).

    A: create a test environment. Execute 'Dim(Lock,1)'.
    E: must report an error (Lock has no dimensions). */
AFL_TEST("interpreter.MutexFunctions:Lock:Dim", a)
{
    Environment env;

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    bco->addInstruction(Opcode::maBinary, interpreter::biArrayDim, 0);

    // Real
    {
        Process p(env.world, "p", 1);
        p.pushFrame(bco, true);
        p.run(0);
        a.checkEqual("01. getState", p.getState(), Process::Failed);
    }

    // Dummy
    {
        Process p(env.world, "p", 1);
        p.pushFrame(makeDummyBCO(bco), true);
        p.run(0);
        a.checkEqual("11. getState", p.getState(), Process::Failed);
    }
}

/** Test border case: Str(Lock).

    A: create a test environment. Execute 'Str(Lock)'.
    E: must report correct value. */
AFL_TEST("interpreter.MutexFunctions:Lock:Str", a)
{
    Environment env;
    Process p(env.world, "p", 1);

    BCORef_t bco = makeBCO();
    bco->addPushLiteral(env.world.getGlobalValue("LOCK"));
    bco->addInstruction(Opcode::maUnary, interpreter::unStr, 0);
    p.pushFrame(bco, true);
    p.run(0);

    a.checkEqual("01. getState", p.getState(), Process::Ended);
    a.checkEqual("02. result", toString(p), "Lock");
}
