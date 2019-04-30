/**
  *  \file u/t_client_proxy.hpp
  *  \brief Tests for client::proxy
  */
#ifndef C2NG_U_T_CLIENT_PROXY_HPP
#define C2NG_U_T_CLIENT_PROXY_HPP

#include <cxxtest/TestSuite.h>

class TestClientProxyObjectListener : public CxxTest::TestSuite {
 public:
};

class TestClientProxyObjectObserver : public CxxTest::TestSuite {
 public:
    void testInterface();
};

#endif
