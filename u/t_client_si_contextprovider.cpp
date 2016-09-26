/**
  *  \file u/t_client_si_contextprovider.cpp
  *  \brief Test for client::si::ContextProvider
  */

#include "client/si/contextprovider.hpp"

#include "t_client_si.hpp"

/** Interface test. */
void
TestClientSiContextProvider::testIt()
{
    class Tester : public client::si::ContextProvider {
     public:
        virtual void createContext(game::Session& /*session*/, interpreter::Process& /*proc*/)
            { }
    };
    Tester t;
}

