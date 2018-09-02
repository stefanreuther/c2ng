/**
  *  \file u/t_server_host_file.hpp
  *  \brief Tests for server::host::file
  */
#ifndef C2NG_U_T_SERVER_HOST_FILE_HPP
#define C2NG_U_T_SERVER_HOST_FILE_HPP

#include <cxxtest/TestSuite.h>

class TestServerHostFileFileItem : public CxxTest::TestSuite {
 public:
    void testIt();
    void testList();
    void testListLimited();
};

class TestServerHostFileGameRootItem : public CxxTest::TestSuite {
 public:
    void testGame();
};

class TestServerHostFileItem : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testResolvePath();
};

class TestServerHostFileRootItem : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestServerHostFileToolItem : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRestricted();
};

#endif
