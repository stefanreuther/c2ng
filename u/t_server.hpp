/**
  *  \file u/t_server.hpp
  *  \brief Tests for server
  */
#ifndef C2NG_U_T_SERVER_HPP
#define C2NG_U_T_SERVER_HPP

#include <cxxtest/TestSuite.h>

class TestServerApplication : public CxxTest::TestSuite {
 public:
    void testSimple();
};

class TestServerConfigurationHandler : public CxxTest::TestSuite {
 public:
    void testCommandLine();
    void testFile();
    void testFileOverride();
    void testNoFile();
};

class TestServerErrors : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerPorts : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerTypes : public CxxTest::TestSuite {
 public:
    void testToInteger();
    void testToString();
};

class TestServertypes : public CxxTest::TestSuite {
 public:
    void testTime();
};

#endif
