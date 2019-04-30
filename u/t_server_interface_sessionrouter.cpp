/**
  *  \file u/t_server_interface_sessionrouter.cpp
  *  \brief Test for server::interface::SessionRouter
  */

#include "server/interface/sessionrouter.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceSessionRouter::testInterface()
{
    class Tester : public server::interface::SessionRouter {
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
void
TestServerInterfaceSessionRouter::testParse()
{
    using server::interface::SessionRouter;
    server::interface::SessionRouter::Action a = SessionRouter::Save;

    TS_ASSERT(SessionRouter::parseAction("close", a));
    TS_ASSERT_EQUALS(a, SessionRouter::Close);

    TS_ASSERT(SessionRouter::parseAction("Restart", a));
    TS_ASSERT_EQUALS(a, SessionRouter::Restart);

    TS_ASSERT(SessionRouter::parseAction("SAVE", a));
    TS_ASSERT_EQUALS(a, SessionRouter::Save);

    TS_ASSERT(SessionRouter::parseAction("saveNN", a));
    TS_ASSERT_EQUALS(a, SessionRouter::SaveNN);

    TS_ASSERT(!SessionRouter::parseAction("SAVEN", a));
    TS_ASSERT(!SessionRouter::parseAction("S", a));
    TS_ASSERT(!SessionRouter::parseAction("", a));
    TS_ASSERT(!SessionRouter::parseAction(" save ", a));
}

/** Test SessionRouter::formatAction. */
void
TestServerInterfaceSessionRouter::testFormat()
{
    using server::interface::SessionRouter;
    TS_ASSERT_EQUALS(SessionRouter::formatAction(SessionRouter::Close),   "CLOSE");
    TS_ASSERT_EQUALS(SessionRouter::formatAction(SessionRouter::Restart), "RESTART");
    TS_ASSERT_EQUALS(SessionRouter::formatAction(SessionRouter::Save),    "SAVE");
    TS_ASSERT_EQUALS(SessionRouter::formatAction(SessionRouter::SaveNN),  "SAVENN");
}

