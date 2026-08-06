/**
  *  \file test/server/interface/filesharetest.cpp
  *  \brief Test for server::interface::FileShare
  */

#include "server/interface/fileshare.hpp"

#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.FileShare")
{
    class Tester : public server::interface::FileShare {
     public:
        virtual int32_t getSequenceNumber(String_t /*userId*/)
            { return 0; }
        virtual void listDirectories(String_t /*userId*/, afl::container::PtrVector<Info>& /*result*/)
            { }
        virtual void listGameInfo(String_t /*userId*/, afl::container::PtrVector<server::interface::FileGame::GameInfo>& /*result*/)
            { }
    };
    Tester t;
}
