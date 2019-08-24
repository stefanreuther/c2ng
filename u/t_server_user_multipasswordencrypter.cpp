/**
  *  \file u/t_server_user_multipasswordencrypter.cpp
  *  \brief Test for server::user::MultiPasswordEncrypter
  */

#include "server/user/multipasswordencrypter.hpp"

#include "t_server_user.hpp"
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
void
TestServerUserMultiPasswordEncrypter::testIt()
{
    // Test objects
    TestEncrypter a("a");
    TestEncrypter b("b");
    TS_ASSERT_EQUALS(a.encryptPassword("P", "u"), "aPu");
    TS_ASSERT_EQUALS(a.checkPassword("P", "aPu", "u"), PasswordEncrypter::ValidCurrent);

    // Tester
    server::user::MultiPasswordEncrypter testee(a, b);
    TS_ASSERT_EQUALS(testee.encryptPassword("P", "u"), "aPu");
    TS_ASSERT_EQUALS(testee.checkPassword("P", "aPu", "u"), PasswordEncrypter::ValidCurrent);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "bPu", "u"), PasswordEncrypter::ValidNeedUpdate);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "cPu", "u"), PasswordEncrypter::Invalid);
}

