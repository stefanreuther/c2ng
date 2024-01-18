/**
  *  \file test/server/interface/userdatatest.cpp
  *  \brief Test for server::interface::UserData
  */

#include "server/interface/userdata.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.UserData")
{
    class Tester : public server::interface::UserData {
     public:
        virtual void set(String_t /*userId*/, String_t /*key*/, String_t /*value*/)
            { }
        virtual String_t get(String_t /*userId*/, String_t /*key*/)
            { return String_t(); }
    };
    Tester t;
}
