/**
  *  \file test/server/interface/hostrankingtest.cpp
  *  \brief Test for server::interface::HostRanking
  */

#include "server/interface/hostranking.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.HostRanking")
{
    class Tester : public server::interface::HostRanking {
     public:
        virtual server::Value_t* getUserList(const ListRequest& /*req*/)
            { return 0; }
    };
    Tester t;
}
