/**
  *  \file test/server/mailout/transmittertest.cpp
  *  \brief Test for server::mailout::Transmitter
  */

#include "server/mailout/transmitter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.mailout.Transmitter")
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
