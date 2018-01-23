/**
  *  \file u/t_server_common.hpp
  *  \brief Tests for server::common
  */
#ifndef C2NG_U_T_SERVER_COMMON_HPP
#define C2NG_U_T_SERVER_COMMON_HPP

#include <cxxtest/TestSuite.h>

class TestServerCommonRaceNames : public CxxTest::TestSuite {
 public:
    void testSuccess();
    void testError();
};

class TestServerCommonRoot : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerCommonSession : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFormatWord();
};

class TestServerCommonSessionProtocolHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerCommonSessionProtocolHandlerFactory : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerCommonUser : public CxxTest::TestSuite {
 public:
    void testRealName();
};

#endif
