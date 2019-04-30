/**
  *  \file u/t_server_interface_hosthistory.cpp
  *  \brief Test for server::interface::HostHistory
  */

#include "server/interface/hosthistory.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostHistory::testInterface()
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

