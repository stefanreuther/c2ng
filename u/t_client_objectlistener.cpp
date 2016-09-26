/**
  *  \file u/t_client_objectlistener.cpp
  *  \brief Test for client::ObjectListener
  */

#include "client/objectlistener.hpp"

#include "t_client.hpp"

/** Interface test. */
void
TestClientObjectListener::testIt()
{
    class Tester : public client::ObjectListener {
     public:
        virtual void handle(game::Session& /*s*/, game::map::Object* /*obj*/)
            { }
    };
    Tester t;
}

