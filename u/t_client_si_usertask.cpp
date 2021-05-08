/**
  *  \file u/t_client_si_usertask.cpp
  *  \brief Test for client::si::UserTask
  */

#include "client/si/usertask.hpp"

#include "t_client_si.hpp"
#include "client/si/requestlink2.hpp"

/** Interface test. */
void
TestClientSiUserTask::testIt()
{
    class Tester : public client::si::UserTask {
     public:
        virtual void handle(client::si::Control& /*ctl*/, client::si::RequestLink2 /*link*/)
            { }
    };
    Tester t;
}

