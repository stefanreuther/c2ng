/**
  *  \file u/t_client.hpp
  *  \brief Tests for client
  */
#ifndef C2NG_U_T_CLIENT_HPP
#define C2NG_U_T_CLIENT_HPP

#include <cxxtest/TestSuite.h>

class TestClientDownlink : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestClientObjectCursorFactory : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
