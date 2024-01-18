/**
  *  \file test/server/interface/sessionroutertest.cpp
  *  \brief Test for server::interface::SessionRouter
  */

#include "server/interface/sessionrouter.hpp"
#include "afl/test/testrunner.hpp"

using server::interface::SessionRouter;

/** Interface test. */
AFL_TEST_NOARG("server.interface.SessionRouter:interface")
{
    class Tester : public SessionRouter {
     public:
        virtual String_t getStatus()
            { return String_t(); }
        virtual String_t getInfo(SessionId_t /*sessionId*/)
            { return String_t(); }
        virtual String_t talk(SessionId_t /*sessionId*/, String_t /*command*/)
            { return String_t(); }
        virtual void sessionAction(SessionId_t /*sessionId*/, Action /*action*/)
            { }
        virtual void groupAction(String_t /*key*/, Action /*action*/, afl::data::StringList_t& /*result*/)
            { }
        virtual SessionId_t create(afl::base::Memory<const String_t> /*args*/)
            { return String_t(); }
        virtual String_t getConfiguration()
            { return String_t(); }
    };
    Tester t;
}

/** Test SessionRouter::parseAction. */
AFL_TEST("server.interface.SessionRouter:parseAction", a)
{
    SessionRouter::Action ac = SessionRouter::Save;

    a.check("01", SessionRouter::parseAction("close", ac));
    a.checkEqual("02", ac, SessionRouter::Close);

    a.check("11", SessionRouter::parseAction("Restart", ac));
    a.checkEqual("12", ac, SessionRouter::Restart);

    a.check("21", SessionRouter::parseAction("SAVE", ac));
    a.checkEqual("22", ac, SessionRouter::Save);

    a.check("31", SessionRouter::parseAction("saveNN", ac));
    a.checkEqual("32", ac, SessionRouter::SaveNN);

    a.check("41", !SessionRouter::parseAction("SAVEN", ac));
    a.check("42", !SessionRouter::parseAction("S", ac));
    a.check("43", !SessionRouter::parseAction("", ac));
    a.check("44", !SessionRouter::parseAction(" save ", ac));
}

/** Test SessionRouter::formatAction. */
AFL_TEST("server.interface.SessionRouter:formatAction", a)
{
    a.checkEqual("01", SessionRouter::formatAction(SessionRouter::Close),   "CLOSE");
    a.checkEqual("02", SessionRouter::formatAction(SessionRouter::Restart), "RESTART");
    a.checkEqual("03", SessionRouter::formatAction(SessionRouter::Save),    "SAVE");
    a.checkEqual("04", SessionRouter::formatAction(SessionRouter::SaveNN),  "SAVENN");
}
