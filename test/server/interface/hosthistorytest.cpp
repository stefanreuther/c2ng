/**
  *  \file test/server/interface/hosthistorytest.cpp
  *  \brief Test for server::interface::HostHistory
  */

#include "server/interface/hosthistory.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostHistory")
{
    class Tester : public server::interface::HostHistory {
     public:
        virtual void getEvents(const EventFilter& /*filter*/, afl::container::PtrVector<Event>& /*result*/)
            { }
        virtual void getTurns(int32_t /*gameId*/, const TurnFilter& /*filter*/, afl::container::PtrVector<Turn>& /*result*/)
            { }
    };
    Tester t;
}
