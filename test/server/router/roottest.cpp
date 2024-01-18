/**
  *  \file test/server/router/roottest.cpp
  *  \brief Test for server::router::Root
  */

#include "server/router/root.hpp"

#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"
#include "server/router/configuration.hpp"
#include "server/router/session.hpp"
#include "util/process/factory.hpp"
#include "util/process/subprocess.hpp"
#include <queue>
#include <stdexcept>

namespace {
    /*
     *  A mock for the subprocess
     */

    uint32_t globalCounter = 0;

    class SubprocessMock : public util::process::Subprocess {
     public:
        SubprocessMock()
            : Subprocess(), m_isActive(false), m_processId(0)
            { }
        virtual bool isActive() const
            { return m_isActive; }
        virtual uint32_t getProcessId() const
            { return m_processId; }
        virtual bool start(const String_t& /*path*/, afl::base::Memory<const String_t> /*args*/)
            {
                m_replies.push("100 hi there\n");
                m_processId = ++globalCounter;
                m_isActive = true;
                return true;
            }
        virtual bool stop()
            {
                m_isActive = false;
                return true;
            }
        virtual bool writeLine(const String_t& /*line*/)
            { return false; }
        virtual bool readLine(String_t& result)
            {
                if (m_replies.empty()) {
                    return false;
                } else {
                    result = m_replies.front();
                    m_replies.pop();
                    return true;
                }
            }
        virtual String_t getStatus() const
            { return m_isActive ? "started " : "stopped"; }
     private:
        bool m_isActive;
        uint32_t m_processId;
        std::queue<String_t> m_replies;
    };

    class FactoryMock : public util::process::Factory {
     public:
        virtual util::process::Subprocess* createNewProcess()
            { return new SubprocessMock(); }
    };
}

/** Test basic session management.
    A: create a Root. Start and stop some sessions.
    E: correct status reported */
AFL_TEST("server.router.Root:basics", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;
    server::router::Configuration config;

    // Testee
    server::router::Root testee(factory, gen, config, 0);
    a.check("01. sessions empty", testee.sessions().empty());
    a.checkDifferent("02. config", &config, &testee.config());         // config has been copied
    AFL_CHECK_SUCCEEDS(a("03. log"), testee.log());

    // Create some sessions
    String_t args1[] = { "hi" };
    server::router::Session& s1 = testee.createSession(args1);
    a.check("11. isActive", s1.isActive());

    String_t args2[] = { "ho" };
    server::router::Session& s2 = testee.createSession(args2);
    a.check("21. isActive", s2.isActive());

    // Verify sessions can be accessed
    a.checkEqual("31. sessions", testee.sessions().size(), 2U);
    a.checkEqual("32. getSessionById", testee.getSessionById(s1.getId()), &s1);
    a.checkEqual("33. getSessionById", testee.getSessionById(s2.getId()), &s2);
    a.checkEqual("34. getSessionById", testee.getSessionById("lol"), (server::router::Session*) 0);

    // Stop a session and clean up
    s1.stop();
    testee.removeExpiredSessions();
    a.checkEqual("41. sessions", testee.sessions().size(), 1U);
    a.checkEqual("42. session 0", testee.sessions()[0], &s2);

    // Stop that one, too
    testee.stopAllSessions();
    a.checkEqual("51. sessions", testee.sessions().size(), 0U);
}

/** Test limit handling.
    A: create a Root. Start multiple sessions.
    E: After maxSessions limit is exceeded, creating more sessions fails. */
AFL_TEST("server.router.Root:limit", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;
    server::router::Configuration config;
    config.maxSessions = 5;
    config.newSessionsWin = false;

    // Testee
    server::router::Root testee(factory, gen, config, 0);

    // Create five sessions
    for (int i = 0; i < 5; ++i) {
        AFL_CHECK_SUCCEEDS(a("01. createSession"), testee.createSession(afl::base::Nothing));
    }

    // Sixths overflows and throws
    AFL_CHECK_THROWS(a("11. createSession overflow"), testee.createSession(afl::base::Nothing), std::exception);
}

/** Test limit handling, with stopped session.
    A: create a Root. Start multiple sessions.
    E: No failure after maxSessions limit is exceeded if room can be made by discarding stopped sessions. */
AFL_TEST("server.router.Root:limit:stopped", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;
    server::router::Configuration config;
    config.maxSessions = 5;
    config.newSessionsWin = false;

    // Testee
    server::router::Root testee(factory, gen, config, 0);

    // Create five sessions but stop one
    for (int i = 0; i < 4; ++i) {
        AFL_CHECK_SUCCEEDS(a("01. createSession"), testee.createSession(afl::base::Nothing));
    }
    testee.createSession(afl::base::Nothing).stop();

    // Sixths will cause stopped one to be gc'ed.
    AFL_CHECK_SUCCEEDS(a("11. createSession"), testee.createSession(afl::base::Nothing));
}

/** Test conflict handling.
    A: create root with newSessionsWin=false. Create two sessions.
    E: creating the second session fails */
AFL_TEST("server.router.Root:conflict", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;
    server::router::Configuration config;
    config.newSessionsWin = false;

    // Testee
    server::router::Root testee(factory, gen, config, 0);

    // Create one session
    String_t args[] = {"-Wfoo"};
    server::router::Session& s = testee.createSession(args);

    // Creating another one fails (newSessionsWin=false)
    AFL_CHECK_THROWS(a("01. createSession conflict"), testee.createSession(args), std::exception);

    // Verify list
    a.checkEqual("11. sessions", testee.sessions().size(), 1U);
    a.checkEqual("12. sessions", testee.sessions()[0], &s);
}

/** Test conflict handling, new session winds.
    A: create root with newSessionsWin=true. Create two sessions.
    E: creating the second session succeeds and closes the first one */
AFL_TEST("server.router.Root:conflict:newSessionsWin", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;
    server::router::Configuration config;
    config.newSessionsWin = true;

    // Testee
    server::router::Root testee(factory, gen, config, 0);

    // Create two sessions. Second one survives.
    String_t args[] = {"-Wfoo"};
    /*server::router::Session& s1 =*/ testee.createSession(args);
    server::router::Session& s2   =   testee.createSession(args);

    // Verify list
    a.checkEqual("01. sessions", testee.sessions().size(), 1U);
    a.checkEqual("02. sessions", testee.sessions()[0], &s2);
}

/** Test restarting a session.
    A: create a session. Restart it.
    E: verify that pid changes in response to restart. */
AFL_TEST("server.router.Root:restart", a)
{
    // Environment
    FactoryMock factory;
    server::common::NumericalIdGenerator gen;

    // Create session
    server::router::Root testee(factory, gen, server::router::Configuration(), 0);
    server::router::Session& s = testee.createSession(afl::base::Nothing);
    a.check("01. isActive", s.isActive());
    uint32_t pid1 = s.getProcessId();

    testee.restartSession(s);
    a.check("11. isActive", s.isActive());
    uint32_t pid2 = s.getProcessId();

    a.checkDifferent("21. pid", pid1, pid2);
}
