/**
  *  \file u/t_client_si_contextreceiver.cpp
  *  \brief Test for client::si::ContextReceiver
  */

#include "client/si/contextreceiver.hpp"

#include "t_client_si.hpp"

/** Interface test. */
void
TestClientSiContextReceiver::testInterface()
{
    class Tester : public client::si::ContextReceiver {
     public:
        virtual void addNewContext(interpreter::Context* /*pContext*/)
            { }
    };
    Tester t;
}

