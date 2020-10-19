/**
  *  \file u/t_server_router_session.cpp
  *  \brief Test for server::router::Session
  */

#include <stdexcept>
#include "server/router/session.hpp"

#include "t_server_router.hpp"
#include "afl/container/ptrqueue.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/callreceiver.hpp"
#include "server/errors.hpp"
#include "util/process/nullfactory.hpp"
#include "util/process/subprocess.hpp"

using server::router::Session;
using util::process::NullFactory;
using afl::string::Format;

namespace {
    /*
     *  A mock for the subprocess
     */

    class SubprocessMock : public util::process::Subprocess, public afl::test::CallReceiver {
     public:
        SubprocessMock(afl::test::Assert a)
            : Subprocess(), CallReceiver(a), m_isActive(false), m_processId(0), m_status()
            { }
        virtual bool isActive() const
            { return m_isActive; }
        virtual uint32_t getProcessId() const
            { return m_processId; }
        virtual bool start(const String_t& path, afl::base::Memory<const String_t> args)
            {
                checkCall(Format("start(%s,%d)", path, args.size()));
                consumeStatus();
                return consumeReturnValue<bool>();
            }
        virtual bool stop()
            {
                /* If a test fails midway, the Session object will be destructed, causing stop() to be called at unexpected places.
                   This produces an exception-while-unwinding, and therefore an unhelpful error message.
                   Disable the checkCall() for stop() to debug. */
#if 1
                checkCall("stop()");
                consumeStatus();
                return consumeReturnValue<bool>();
#else
                return true;
#endif
            }
        virtual bool writeLine(const String_t& line)
            {
                checkCall(Format("writeLine(%s)", line));
                return consumeReturnValue<bool>();
            }
        virtual bool readLine(String_t& result)
            {
                checkCall("readLine()");
                bool ok = consumeReturnValue<bool>();
                if (ok) {
                    result = consumeReturnValue<String_t>();
                }
                return ok;
            }
        virtual String_t getStatus() const
            { return m_status; }

        void provideStatus(bool active, uint32_t processId, String_t status)
            {
                provideReturnValue(active);
                provideReturnValue(processId);
                provideReturnValue(status);
            }
     private:
        bool m_isActive;
        uint32_t m_processId;
        String_t m_status;

        void consumeStatus()
            {
                m_isActive  = consumeReturnValue<bool>();
                m_processId = consumeReturnValue<uint32_t>();
                m_status    = consumeReturnValue<String_t>();
            }
    };

    class FactoryMock : public util::process::Factory {
     public:
        virtual util::process::Subprocess* createNewProcess()
            {
                assert(!m_queue.empty());
                return m_queue.extractFront();
            }

        void pushBackNew(util::process::Subprocess* p)
            { m_queue.pushBackNew(p); }

     private:
        afl::container::PtrQueue<util::process::Subprocess> m_queue;
    };
}


/** Test initialisation.
    A: create a session
    E: verify stored parameters */
void
TestServerRouterSession::testInit()
{
    // Setup
    NullFactory factory;
    String_t args[] = { "a", "b" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Check
    TS_ASSERT_EQUALS(testee.getId(), "session_id");
    TS_ASSERT_EQUALS(testee.getProcessId(), 0U);
    TS_ASSERT_EQUALS(testee.isModified(), false);
    TS_ASSERT_EQUALS(testee.isUsed(), false);
    TS_ASSERT_EQUALS(testee.isActive(), false);
    TS_ASSERT(testee.getLastAccessTime() <= afl::sys::Time::getCurrentTime());

    // Verify args: return value is a copy of ctor parameter
    afl::base::Memory<const String_t> savedArgs = testee.getCommandLine();
    TS_ASSERT_EQUALS(savedArgs.size(), 2U);
    TS_ASSERT_EQUALS(*savedArgs.at(0), args[0]);
    TS_ASSERT_DIFFERS(savedArgs.at(0), &args[0]);

    // We cannot talk to this session (not started)
    TS_ASSERT_EQUALS(testee.talk("hello"), server::SESSION_TIMED_OUT);

    // We cannot start this session (NullFactory refuses)
    TS_ASSERT_EQUALS(testee.start("prog"), false);
}

/** Test conflict resolution.
    A: create a session. invoke checkConflict() with various parameters.
    E: correct conflicts detected */
void
TestServerRouterSession::testConflict()
{
    // Setup
    NullFactory factory;
    String_t args[] = { "a", "-Wwhite", "-Rred", "-Wdir=x/y" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Check
    // - non-marker
    TS_ASSERT(!testee.checkConflict("a", false));

    // - 'W' marker
    TS_ASSERT( testee.checkConflict("-Rwhite", false));
    TS_ASSERT( testee.checkConflict("-Wwhite", false));

    // - 'R' marker
    TS_ASSERT(!testee.checkConflict("-Rred", false));
    TS_ASSERT( testee.checkConflict("-Wred", false));

    // - wildcard style
    TS_ASSERT( testee.checkConflict("-Wdir=x/y",  false));
    TS_ASSERT(!testee.checkConflict("-Wdir=x/yz", false));
    TS_ASSERT(!testee.checkConflict("-Wdir=x*",   false));
    TS_ASSERT(!testee.checkConflict("-Wdir=x",    false));
    TS_ASSERT( testee.checkConflict("-Wdir=x/y",  true));
    TS_ASSERT(!testee.checkConflict("-Wdir=x/yz", true));
    TS_ASSERT( testee.checkConflict("-Wdir=x*",   true));
    TS_ASSERT(!testee.checkConflict("-Wdir=x",    true));

    // Check session conflict
    {
        String_t args2[] = { "-Wwhite" };
        Session s2(factory, args2, "s2", log, 0);
        TS_ASSERT(testee.checkConflict(s2));
    }
    {
        String_t args3[] = { "-Rred" };
        Session s3(factory, args3, "s3", log, 0);
        TS_ASSERT(!testee.checkConflict(s3));
    }
}

/** Test talk().
    A: create a session with a proper subprocess mock. Invoke a variety of talk() commands.
    E: expected sequence of writeLine(), readLine() on subprocess */
void
TestServerRouterSession::testTalk()
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock("testTalk");
    factory.pushBackNew(proc);

    // Testee/environment
    String_t args[] = { "a", "b", "c" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Startup sequence
    proc->expectCall("start(prog,3)");
    proc->provideStatus(true, 42, "started");
    proc->provideReturnValue(true);

    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("100 hi there\n"));

    bool ok = testee.start("prog");
    TS_ASSERT(ok);
    TS_ASSERT_EQUALS(testee.getProcessId(), 42U);
    TS_ASSERT_EQUALS(testee.isUsed(), false);
    TS_ASSERT_EQUALS(testee.isModified(), false);
    TS_ASSERT_EQUALS(testee.isActive(), true);

    // Submit a read command
    proc->expectCall("writeLine(GET obj/main\n)");
    proc->provideReturnValue(true);
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("200 ok\n"));
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("{\"main\":{}}\n"));
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t(".\n"));

    String_t answer = testee.talk("GET obj/main");
    TS_ASSERT_EQUALS(answer, "200 ok\n{\"main\":{}}\n");
    TS_ASSERT_EQUALS(testee.isUsed(), true);

    // This will mark the session modified as far as router is concerned!
    TS_ASSERT_EQUALS(testee.isModified(), true);

    // Submit a write command
    proc->expectCall("writeLine(POST obj/main\n[]\n.\n)");
    proc->provideReturnValue(true);
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("200 ok\n"));
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("{\"reply\":{}}\n"));
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t(".\n"));

    answer = testee.talk("POST obj/main\n[]");
    TS_ASSERT_EQUALS(answer, "200 ok\n{\"reply\":{}}\n");
    TS_ASSERT_EQUALS(testee.isUsed(), true);
    TS_ASSERT_EQUALS(testee.isModified(), true);

    // Save it
    proc->expectCall("writeLine(SAVE\n)");
    proc->provideReturnValue(true);
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("100 ok\n"));
    testee.save(false);
    TS_ASSERT(!testee.isModified());

    // Stop
    proc->expectCall("stop()");
    proc->provideStatus(false, 0, "stopped");
    proc->provideReturnValue(true);
    testee.stop();
}

/** Test writeLine() error.
    This simulates the process stopping to take input mid-way.
    A: create a session with a proper subprocess mock. Have it return false from writeLine() eventually.
    E: Session performs proper shutdown sequence and status update */
void
TestServerRouterSession::testWriteError()
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock("testTalk");
    factory.pushBackNew(proc);

    // Testee/environment
    String_t args[] = { "a", "b", "c" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Startup sequence
    proc->expectCall("start(prog,3)");
    proc->provideStatus(true, 42, "started");
    proc->provideReturnValue(true);

    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("100 hi there\n"));

    bool ok = testee.start("prog");
    TS_ASSERT(ok);

    // Submit a command which fails. This causes us to stop immediately.
    proc->expectCall("writeLine(GET obj/main\n)");
    proc->provideReturnValue(false);
    proc->expectCall("stop()");
    proc->provideStatus(false, 0, "stopped");
    proc->provideReturnValue(true);
    TS_ASSERT_THROWS(testee.talk("GET obj/main"), std::exception);

    TS_ASSERT(!testee.isActive());
}

/** Test startup sequence error.
    This simulates the process not talking protocol (e.g. if you gave it "--help").
    A: create a session with a proper subprocess mock. Have it return invalid protocol on startup.
    E: Session performs proper shutdown sequence and status update */
void
TestServerRouterSession::testStartupError()
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock("testTalk");
    factory.pushBackNew(proc);

    // Testee/environment
    String_t args[] = { "a", "b", "c" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Failing startup sequence
    proc->expectCall("start(prog,3)");
    proc->provideStatus(true, 42, "started");
    proc->provideReturnValue(true);

    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("lolwhat\n"));
    proc->expectCall("readLine()");
    proc->provideReturnValue(false);

    proc->expectCall("stop()");
    proc->provideStatus(false, 0, "stopped");
    proc->provideReturnValue(true);

    bool ok = testee.start("prog");
    TS_ASSERT(!ok);
    TS_ASSERT(!testee.isActive());
}

