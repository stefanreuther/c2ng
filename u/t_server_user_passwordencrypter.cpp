/**
  *  \file u/t_server_user_passwordencrypter.cpp
  *  \brief Test for server::user::PasswordEncrypter
  */

#include "server/user/passwordencrypter.hpp"

#include "t_server_user.hpp"

/** Interface test. */
void
TestServerUserPasswordEncrypter::testInterface()
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

