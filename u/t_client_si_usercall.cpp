/**
  *  \file u/t_client_si_usercall.cpp
  *  \brief Test for client::si::UserCall
  */

#include "client/si/usercall.hpp"

#include "t_client_si.hpp"

/** Interface test. */
void
TestClientSiUserCall::testIt()
{
    class Tester : public client::si::UserCall {
     public:
        virtual void handle(client::si::Control& /*ctl*/)
            { }
    };
    Tester t;
}

