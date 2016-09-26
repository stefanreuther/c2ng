/**
  *  \file u/t_client_si.hpp
  *  \brief Tests for client::si
  */
#ifndef C2NG_U_T_CLIENT_SI_HPP
#define C2NG_U_T_CLIENT_SI_HPP

#include <cxxtest/TestSuite.h>

class TestClientSiContextProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestClientSiControl : public CxxTest::TestSuite {
 public:
    void testMulti();
    void testSingle();
};

class TestClientSiRequestLink1 : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestClientSiRequestLink2 : public CxxTest::TestSuite {
 public:
    void testIt();
    void testConvert();
};

class TestClientSiUserTask : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
