/**
  *  \file u/t_server_interface_hostcron.cpp
  *  \brief Test for server::interface::HostCron
  */

#include "server/interface/hostcron.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostCron::testInterface()
{
    class Tester : public server::interface::HostCron {
     public:
        virtual Event getGameEvent(int32_t /*gameId*/)
            { return Event(); }
        virtual void listGameEvents(afl::base::Optional<int32_t> /*limit*/, std::vector<Event>& /*result*/)
            { }
        virtual bool kickstartGame(int32_t /*gameId*/)
            { return false; }
    };
    Tester t;
}

