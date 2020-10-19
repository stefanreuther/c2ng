/**
  *  \file u/t_server_router.hpp
  *  \brief Tests for server::router
  */
#ifndef C2NG_U_T_SERVER_ROUTER_HPP
#define C2NG_U_T_SERVER_ROUTER_HPP

#include <cxxtest/TestSuite.h>

class TestServerRouterConfiguration : public CxxTest::TestSuite {
 public:
    void testInit();
};

class TestServerRouterRoot : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLimit();
    void testLimitStopped();
    void testConflict();
    void testConflictNewWins();
    void testRestart();
};

class TestServerRouterSession : public CxxTest::TestSuite {
 public:
    void testInit();
    void testConflict();
    void testTalk();
    void testWriteError();
    void testStartupError();
};

#endif
