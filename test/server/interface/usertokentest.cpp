/**
  *  \file test/server/interface/usertokentest.cpp
  *  \brief Test for server::interface::UserToken
  */

#include "server/interface/usertoken.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.interface.UserToken")
{
    class Tester : public server::interface::UserToken {
     public:
        virtual String_t getToken(String_t /*userId*/, String_t /*tokenType*/)
            { return String_t(); }
        virtual Info checkToken(String_t /*token*/, afl::base::Optional<String_t> /*requiredType*/, bool /*autoRenew*/)
            { return Info(); }
        virtual void clearToken(String_t /*userId*/, afl::base::Memory<const String_t> /*tokenTypes*/)
            { }
    };
    Tester t;
}
