/**
  *  \file u/t_client_proxy_objectobserver.cpp
  *  \brief Test for client::proxy::ObjectObserver
  */

#include "client/proxy/objectobserver.hpp"

#include "t_client_proxy.hpp"

/** Interface test. */
void
TestClientProxyObjectObserver::testInterface()
{
    class Tester : public client::proxy::ObjectObserver {
     public:
        virtual void addNewListener(client::proxy::ObjectListener* /*pListener*/)
            { }
    };
    Tester t;
}

