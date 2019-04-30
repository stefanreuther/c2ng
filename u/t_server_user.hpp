/**
  *  \file u/t_server_user.hpp
  *  \brief Tests for server::user
  */
#ifndef C2NG_U_T_SERVER_USER_HPP
#define C2NG_U_T_SERVER_USER_HPP

#include <cxxtest/TestSuite.h>

class TestServerUserClassicEncrypter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerUserCommandHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerUserConfiguration : public CxxTest::TestSuite {
 public:
    void testInitialisation();
};

class TestServerUserPasswordEncrypter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerUserToken : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerUserUser : public CxxTest::TestSuite {
 public:
    void testAccessors();
};

class TestServerUserUserData : public CxxTest::TestSuite {
 public:
    void testIt();
    void testExpire();
    void testExpire2();
    void testError();
};

class TestServerUserUserManagement : public CxxTest::TestSuite {
 public:
    void testCreation();
    void testName();
    void testBlockedName();
    void testProfile();
    void testLogin();
};

class TestServerUserUserToken : public CxxTest::TestSuite {
 public:
    void testIt();
    void testTokenTypes();
    void testClearToken();
};

#endif
