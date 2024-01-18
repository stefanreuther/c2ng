/**
  *  \file test/server/router/sessiontest.cpp
  *  \brief Test for server::router::Session
  */

#include "server/router/session.hpp"

#include "afl/container/ptrqueue.hpp"
#include "afl/string/format.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/callreceiver.hpp"
#include "afl/test/testrunner.hpp"
#include "server/errors.hpp"
#include "util/process/nullfactory.hpp"
#include "util/process/subprocess.hpp"
#include <stdexcept>

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
AFL_TEST("server.router.Session:init", a)
{
    // Setup
    NullFactory factory;
    String_t args[] = { "a", "b" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Check
    a.checkEqual("01. getId",             testee.getId(), "session_id");
    a.checkEqual("02. getProcessId",      testee.getProcessId(), 0U);
    a.checkEqual("03. isModified",        testee.isModified(), false);
    a.checkEqual("04. isUsed",            testee.isUsed(), false);
    a.checkEqual("05. isActive",          testee.isActive(), false);
    a.check     ("06. getLastAccessTime", testee.getLastAccessTime() <= afl::sys::Time::getCurrentTime());

    // Verify args: return value is a copy of ctor parameter
    afl::base::Memory<const String_t> savedArgs = testee.getCommandLine();
    a.checkEqual("11. size", savedArgs.size(), 2U);
    a.checkEqual("12. args", *savedArgs.at(0), args[0]);
    a.checkDifferent("13. args", savedArgs.at(0), &args[0]);

    // We cannot talk to this session (not started)
    a.checkEqual("21. talk", testee.talk("hello"), server::SESSION_TIMED_OUT);

    // We cannot start this session (NullFactory refuses)
    a.checkEqual("31. start", testee.start("prog"), false);
}

/** Test conflict resolution.
    A: create a session. invoke checkConflict() with various parameters.
    E: correct conflicts detected */
AFL_TEST("server.router.Session:checkConflict", a)
{
    // Setup
    NullFactory factory;
    String_t args[] = { "a", "-Wwhite", "-Rred", "-Wdir=x/y" };
    afl::sys::Log log;
    Session testee(factory, args, "session_id", log, 0);

    // Check
    // - non-marker
    a.check("01", !testee.checkConflict("a", false));

    // - 'W' marker
    a.check("11",  testee.checkConflict("-Rwhite", false));
    a.check("12",  testee.checkConflict("-Wwhite", false));

    // - 'R' marker
    a.check("21", !testee.checkConflict("-Rred", false));
    a.check("22",  testee.checkConflict("-Wred", false));

    // - wildcard style
    a.check("31",  testee.checkConflict("-Wdir=x/y",  false));
    a.check("32", !testee.checkConflict("-Wdir=x/yz", false));
    a.check("33", !testee.checkConflict("-Wdir=x*",   false));
    a.check("34", !testee.checkConflict("-Wdir=x",    false));
    a.check("35",  testee.checkConflict("-Wdir=x/y",  true));
    a.check("36", !testee.checkConflict("-Wdir=x/yz", true));
    a.check("37",  testee.checkConflict("-Wdir=x*",   true));
    a.check("38", !testee.checkConflict("-Wdir=x",    true));

    // Check session conflict
    {
        String_t args2[] = { "-Wwhite" };
        Session s2(factory, args2, "s2", log, 0);
        a.check("41", testee.checkConflict(s2));
    }
    {
        String_t args3[] = { "-Rred" };
        Session s3(factory, args3, "s3", log, 0);
        a.check("42", !testee.checkConflict(s3));
    }
}

/** Test talk().
    A: create a session with a proper subprocess mock. Invoke a variety of talk() commands.
    E: expected sequence of writeLine(), readLine() on subprocess */
AFL_TEST("server.router.Session:talk", a)
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock(a);
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
    a.check("01. start", ok);
    a.checkEqual("02. getProcessId", testee.getProcessId(), 42U);
    a.checkEqual("03. isUsed",       testee.isUsed(), false);
    a.checkEqual("04. isModified",   testee.isModified(), false);
    a.checkEqual("05. isActive",     testee.isActive(), true);

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
    a.checkEqual("11. talk",   answer, "200 ok\n{\"main\":{}}\n");
    a.checkEqual("12. isUsed", testee.isUsed(), true);

    // This will mark the session modified as far as router is concerned!
    a.checkEqual("21. isModified", testee.isModified(), true);

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
    a.checkEqual("31. talk",       answer, "200 ok\n{\"reply\":{}}\n");
    a.checkEqual("32. isUsed",     testee.isUsed(), true);
    a.checkEqual("33. isModified", testee.isModified(), true);

    // Save it
    proc->expectCall("writeLine(SAVE\n)");
    proc->provideReturnValue(true);
    proc->expectCall("readLine()");
    proc->provideReturnValue(true);
    proc->provideReturnValue(String_t("100 ok\n"));
    testee.save(false);
    a.check("41. isModified", !testee.isModified());

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
AFL_TEST("server.router.Session:error:write", a)
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock(a);
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
    a.check("01. start", ok);

    // Submit a command which fails. This causes us to stop immediately.
    proc->expectCall("writeLine(GET obj/main\n)");
    proc->provideReturnValue(false);
    proc->expectCall("stop()");
    proc->provideStatus(false, 0, "stopped");
    proc->provideReturnValue(true);
    AFL_CHECK_THROWS(a("11. talk"), testee.talk("GET obj/main"), std::exception);

    a.check("21. isActive", !testee.isActive());
}

/** Test startup sequence error.
    This simulates the process not talking protocol (e.g. if you gave it "--help").
    A: create a session with a proper subprocess mock. Have it return invalid protocol on startup.
    E: Session performs proper shutdown sequence and status update */
AFL_TEST("server.router.Session:error:startup", a)
{
    // Provide a mock
    FactoryMock factory;
    SubprocessMock* proc = new SubprocessMock(a);
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
    a.check("01. start", !ok);
    a.check("02. isActive", !testee.isActive());
}
