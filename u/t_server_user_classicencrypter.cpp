/**
  *  \file u/t_server_user_classicencrypter.cpp
  *  \brief Test for server::user::ClassicEncrypter
  */

#include "server/user/classicencrypter.hpp"

#include "t_server_user.hpp"

/** Simple regression test.
    Properties verified:
    - passwords are case-/space-sensitive
    - user Id does not affect password
    - hashes are case-sensitive
    - "1," is a fixed token, not parsed
    - system key affects encryption */
void
TestServerUserClassicEncrypter::testIt()
{
    server::user::ClassicEncrypter testee("key");

    // Encryption
    // echo -n keyp | openssl md5 -binary | base64 | tr -d =
    TS_ASSERT_EQUALS(testee.encryptPassword("p",  "1000"), "1,y63WJ9sp7eDZKIFW4MxfEA");
    TS_ASSERT_EQUALS(testee.encryptPassword("p",  "1001"), "1,y63WJ9sp7eDZKIFW4MxfEA");
    TS_ASSERT_EQUALS(testee.encryptPassword("P",  "1000"), "1,Uv8lbADNWPrhUlr50jvP/g");
    TS_ASSERT_EQUALS(testee.encryptPassword("",   "1000"), "1,PG4LipwVIkqCKLmpjKFTHQ");
    TS_ASSERT_EQUALS(testee.encryptPassword("p ", "1001"), "1,zRaTCt1GIyXHIky3Eba0yA");
    TS_ASSERT_EQUALS(testee.encryptPassword(" p", "1001"), "1,XtUac2s5et/zJRPZjyi3hw");

    // Verification
    using server::user::PasswordEncrypter;
    TS_ASSERT_EQUALS(testee.checkPassword("p", "1,y63WJ9sp7eDZKIFW4MxfEA", "1000"), PasswordEncrypter::ValidCurrent);
    TS_ASSERT_EQUALS(testee.checkPassword("p", "1,y63WJ9sp7eDZKIFW4MxfEA", "1001"), PasswordEncrypter::ValidCurrent);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "1,y63WJ9sp7eDZKIFW4MxfEA", "1000"), PasswordEncrypter::Invalid);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "1,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::ValidCurrent);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "1,UV8LBADNWPRHULR50JVP/G", "1000"), PasswordEncrypter::Invalid);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "2,whatever", "1000"), PasswordEncrypter::Invalid);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "10,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::Invalid);
    TS_ASSERT_EQUALS(testee.checkPassword("P", "01,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::Invalid);

    server::user::ClassicEncrypter testee2("other");
    TS_ASSERT_EQUALS(testee2.encryptPassword("p", "1000"), "1,2iZrHREPqpf8Km/Jwzc5Sw");
}

