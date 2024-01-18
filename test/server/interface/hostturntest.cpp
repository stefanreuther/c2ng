/**
  *  \file test/server/interface/hostturntest.cpp
  *  \brief Test for server::interface::HostTurn
  */

#include "server/interface/hostturn.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostTurn:interface")
{
    class Tester : public server::interface::HostTurn {
     public:
        virtual Result submit(const String_t& /*blob*/, afl::base::Optional<int32_t> /*game*/, afl::base::Optional<int32_t> /*slot*/, afl::base::Optional<String_t> /*mail*/, afl::base::Optional<String_t> /*info*/)
            { return Result(); }
        virtual void setTemporary(int32_t /*gameId*/, int32_t /*slot*/, bool /*flag*/)
            { }
    };
    Tester t;
}

/** Test initialisation of Result. */
AFL_TEST("server.interface.HostTurn:init", a)
{
    server::interface::HostTurn::Result t;
    a.checkEqual("01. state",         t.state, 0);
    a.checkEqual("02. output",        t.output, "");
    a.checkEqual("03. gameId",        t.gameId, 0);
    a.checkEqual("04. slot",          t.slot, 0);
    a.checkEqual("05. previousState", t.previousState, 0);
    a.checkEqual("06. userId",        t.userId, "");
    a.checkEqual("07. gameName",      t.gameName, "");
    a.checkEqual("08. turnNumber",    t.turnNumber, 0);
    a.checkEqual("09. allowTemp",     t.allowTemp, false);
}
