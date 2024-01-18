/**
  *  \file test/server/interface/hostcrontest.cpp
  *  \brief Test for server::interface::HostCron
  */

#include "server/interface/hostcron.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostCron")
{
    class Tester : public server::interface::HostCron {
     public:
        virtual Event getGameEvent(int32_t /*gameId*/)
            { return Event(); }
        virtual void listGameEvents(afl::base::Optional<int32_t> /*limit*/, std::vector<Event>& /*result*/)
            { }
        virtual bool kickstartGame(int32_t /*gameId*/)
            { return false; }
        virtual void suspendScheduler(int32_t /*relativeTime*/)
            { }
        virtual void getBrokenGames(BrokenMap_t& /*result*/)
            { }
    };
    Tester t;
}
