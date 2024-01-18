/**
  *  \file test/server/user/classicencryptertest.cpp
  *  \brief Test for server::user::ClassicEncrypter
  */

#include "server/user/classicencrypter.hpp"
#include "afl/test/testrunner.hpp"

/** Simple regression test.
    Properties verified:
    - passwords are case-/space-sensitive
    - user Id does not affect password
    - hashes are case-sensitive
    - "1," is a fixed token, not parsed
    - system key affects encryption */
AFL_TEST("server.user.ClassicEncrypter", a)
{
    server::user::ClassicEncrypter testee("key");

    // Encryption
    // echo -n keyp | openssl md5 -binary | base64 | tr -d =
    a.checkEqual("01", testee.encryptPassword("p",  "1000"), "1,y63WJ9sp7eDZKIFW4MxfEA");
    a.checkEqual("02", testee.encryptPassword("p",  "1001"), "1,y63WJ9sp7eDZKIFW4MxfEA");
    a.checkEqual("03", testee.encryptPassword("P",  "1000"), "1,Uv8lbADNWPrhUlr50jvP/g");
    a.checkEqual("04", testee.encryptPassword("",   "1000"), "1,PG4LipwVIkqCKLmpjKFTHQ");
    a.checkEqual("05", testee.encryptPassword("p ", "1001"), "1,zRaTCt1GIyXHIky3Eba0yA");
    a.checkEqual("06", testee.encryptPassword(" p", "1001"), "1,XtUac2s5et/zJRPZjyi3hw");

    // Verification
    using server::user::PasswordEncrypter;
    a.checkEqual("11", testee.checkPassword("p", "1,y63WJ9sp7eDZKIFW4MxfEA", "1000"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("12", testee.checkPassword("p", "1,y63WJ9sp7eDZKIFW4MxfEA", "1001"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("13", testee.checkPassword("P", "1,y63WJ9sp7eDZKIFW4MxfEA", "1000"), PasswordEncrypter::Invalid);
    a.checkEqual("14", testee.checkPassword("P", "1,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("15", testee.checkPassword("P", "1,UV8LBADNWPRHULR50JVP/G", "1000"), PasswordEncrypter::Invalid);
    a.checkEqual("16", testee.checkPassword("P", "2,whatever", "1000"), PasswordEncrypter::Invalid);
    a.checkEqual("17", testee.checkPassword("P", "10,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::Invalid);
    a.checkEqual("18", testee.checkPassword("P", "01,Uv8lbADNWPrhUlr50jvP/g", "1000"), PasswordEncrypter::Invalid);

    server::user::ClassicEncrypter testee2("other");
    a.checkEqual("21", testee2.encryptPassword("p", "1000"), "1,2iZrHREPqpf8Km/Jwzc5Sw");
}
