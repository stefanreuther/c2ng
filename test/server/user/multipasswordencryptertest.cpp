/**
  *  \file test/server/user/multipasswordencryptertest.cpp
  *  \brief Test for server::user::MultiPasswordEncrypter
  */

#include "server/user/multipasswordencrypter.hpp"

#include "afl/test/testrunner.hpp"
#include "server/user/passwordencrypter.hpp"

using server::user::PasswordEncrypter;

namespace {
    class TestEncrypter : public PasswordEncrypter {
     public:
        TestEncrypter(String_t prefix)
            : m_prefix(prefix)
            { }
        virtual String_t encryptPassword(String_t password, String_t userId)
            { return m_prefix + password + userId; }
        virtual Result checkPassword(String_t password, String_t hash, String_t userId)
            { return hash == encryptPassword(password, userId) ? ValidCurrent : Invalid; }
     private:
        String_t m_prefix;
    };
}

/** Simple test. */
AFL_TEST("server.user.MultiPasswordEncrypter", a)
{
    // Test objects
    TestEncrypter ea("a");
    TestEncrypter eb("b");
    a.checkEqual("01", ea.encryptPassword("P", "u"), "aPu");
    a.checkEqual("02", ea.checkPassword("P", "aPu", "u"), PasswordEncrypter::ValidCurrent);

    // Tester
    server::user::MultiPasswordEncrypter testee(ea, eb);
    a.checkEqual("11", testee.encryptPassword("P", "u"), "aPu");
    a.checkEqual("12", testee.checkPassword("P", "aPu", "u"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("13", testee.checkPassword("P", "bPu", "u"), PasswordEncrypter::ValidNeedUpdate);
    a.checkEqual("14", testee.checkPassword("P", "cPu", "u"), PasswordEncrypter::Invalid);
}
