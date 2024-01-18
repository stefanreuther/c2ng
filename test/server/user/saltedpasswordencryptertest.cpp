/**
  *  \file test/server/user/saltedpasswordencryptertest.cpp
  *  \brief Test for server::user::SaltedPasswordEncrypter
  */

#include "server/user/saltedpasswordencrypter.hpp"

#include "afl/test/testrunner.hpp"
#include "server/common/numericalidgenerator.hpp"

using server::user::PasswordEncrypter;

/** Simple functionality test.
    Properties verified:
    - encrypting multiple times will generate different hashes
    - all hashes are accepted
    - invalid keys are rejected */
AFL_TEST("server.user.SaltedPasswordEncrypter:basics", a)
{
    server::common::NumericalIdGenerator saltGen;
    server::user::SaltedPasswordEncrypter testee(saltGen);

    // Encrypting multiple times will generate different hashes
    a.checkEqual("01", testee.encryptPassword("p", "1001"), "2,1,acfe2a18abfde0975bff6a86078fa78b9d01f012");
    a.checkEqual("02", testee.encryptPassword("p", "1001"), "2,2,f07dea7e86c7bd0ee99bb51e3b184b5371280cde");
    a.checkEqual("03", testee.encryptPassword("p", "1001"), "2,3,13f40f242c637e360803b26e46825ed0790d1a7b");

    // All are accepted
    a.checkEqual("11", testee.checkPassword("p", "2,1,acfe2a18abfde0975bff6a86078fa78b9d01f012", "1001"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("12", testee.checkPassword("p", "2,2,f07dea7e86c7bd0ee99bb51e3b184b5371280cde", "1001"), PasswordEncrypter::ValidCurrent);
    a.checkEqual("13", testee.checkPassword("p", "2,3,13f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::ValidCurrent);

    // Invalid
    a.checkEqual("21", testee.checkPassword("q", "2,3,13f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::Invalid);
    a.checkEqual("22", testee.checkPassword("p", "2,4,13f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::Invalid);
    a.checkEqual("23", testee.checkPassword("p", "1,3,13f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::Invalid);
    a.checkEqual("24", testee.checkPassword("p", "2,313f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::Invalid);
    a.checkEqual("25", testee.checkPassword("p", "2313f40f242c637e360803b26e46825ed0790d1a7b", "1001"), PasswordEncrypter::Invalid);
}

/** Simple functionality test.
    Properties verified:
    - Encrypting different passwords will produce different hashes even with same salt */
AFL_TEST("server.user.SaltedPasswordEncrypter:difference", a)
{
    // Encrypting different passwords will produce different hashes even with same salt
    {
        server::common::NumericalIdGenerator saltGen;
        server::user::SaltedPasswordEncrypter testee(saltGen);
        a.checkEqual("01", testee.encryptPassword("p", "1001"), "2,1,acfe2a18abfde0975bff6a86078fa78b9d01f012");
    }
    {
        server::common::NumericalIdGenerator saltGen;
        server::user::SaltedPasswordEncrypter testee(saltGen);
        a.checkEqual("02", testee.encryptPassword("q", "1001"), "2,1,065406afdb6f1c7ccde15e69bec0d0df69511c36");
    }
}
