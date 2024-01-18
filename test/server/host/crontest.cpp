/**
  *  \file test/server/host/crontest.cpp
  *  \brief Test for server::host::Cron
  */

#include "server/host/cron.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.host.Cron")
{
    class Tester : public server::host::Cron {
     public:
        virtual Event_t getGameEvent(int32_t /*gameId*/)
            { return Event_t(); }
        virtual void listGameEvents(std::vector<Event_t>& /*result*/)
            { }
        virtual void handleGameChange(int32_t /*gameId*/)
            { }
        virtual void suspendScheduler(server::Time_t /*absTime*/)
            { }
    };
    Tester t;
}
