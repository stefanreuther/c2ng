/**
  *  \file u/t_server_mailin.hpp
  *  \brief Tests for server::mailin
  */
#ifndef C2NG_U_T_SERVER_MAILIN_HPP
#define C2NG_U_T_SERVER_MAILIN_HPP

#include <cxxtest/TestSuite.h>

class TestServerMailinMailInApplication : public CxxTest::TestSuite {
 public:
    void testHelp();
    void testReject();
};

class TestServerMailinMailProcessor : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testTurn();
    void testError407();
    void testError404();
    void testError412();
    void testError422();
    void testErrorOther();
    void testMultiple();
    void testNested();
    void testDeep();
};

#endif
