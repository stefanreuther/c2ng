/**
  *  \file test/server/user/passwordencryptertest.cpp
  *  \brief Test for server::user::PasswordEncrypter
  */

#include "server/user/passwordencrypter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("server.user.PasswordEncrypter")
{
    class Tester : public server::user::PasswordEncrypter {
     public:
        virtual String_t encryptPassword(String_t /*password*/, String_t /*userId*/)
            { return String_t(); }
        virtual Result checkPassword(String_t /*password*/, String_t /*hash*/, String_t /*userId*/)
            { return Result(); }
    };
    Tester t;
}
