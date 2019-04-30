/**
  *  \file u/t_server_interface_hostranking.cpp
  *  \brief Test for server::interface::HostRanking
  */

#include "server/interface/hostranking.hpp"

#include "t_server_interface.hpp"

/** Interface test. */
void
TestServerInterfaceHostRanking::testInterface()
{
    class Tester : public server::interface::HostRanking {
     public:
        virtual server::Value_t* getUserList(const ListRequest& /*req*/)
            { return 0; }
    };
    Tester t;
}

