/**
  *  \file test/server/interface/basetest.cpp
  *  \brief Test for server::interface::Base
  */

#include "server/interface/base.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.Base")
{
    class Tester : public server::interface::Base {
     public:
        virtual String_t ping()
            { return String_t(); }
        virtual void setUserContext(String_t /*user*/)
            { }
    };
    Tester t;
}
