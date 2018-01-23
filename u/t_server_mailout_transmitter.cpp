/**
  *  \file u/t_server_mailout_transmitter.cpp
  *  \brief Test for server::mailout::Transmitter
  */

#include "server/mailout/transmitter.hpp"

#include "t_server_mailout.hpp"

/** Interface test. */
void
TestServerMailoutTransmitter::testInterface()
{
    class Tester : public server::mailout::Transmitter {
     public:
        virtual void send(int32_t /*messageId*/)
            { }
        virtual void notifyAddress(String_t /*address*/)
            { }
        virtual void runQueue()
            { }
    };
    Tester t;
}

