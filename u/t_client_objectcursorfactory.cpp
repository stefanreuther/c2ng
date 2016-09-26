/**
  *  \file u/t_client_objectcursorfactory.cpp
  *  \brief Test for client::ObjectCursorFactory
  */

#include "client/objectcursorfactory.hpp"

#include "t_client.hpp"

/** Interface test. */
void
TestClientObjectCursorFactory::testIt()
{
    class Tester : public client::ObjectCursorFactory {
     public:
        virtual game::map::ObjectCursor* getCursor(game::Session& /*session*/)
            { return 0; }
    };
    Tester t;
}

