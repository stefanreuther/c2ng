/**
  *  \file u/t_server_host_spec.hpp
  *  \brief Tests for server::host::spec
  */
#ifndef C2NG_U_T_SERVER_HOST_SPEC_HPP
#define C2NG_U_T_SERVER_HOST_SPEC_HPP

#include <cxxtest/TestSuite.h>

class TestServerHostSpecDirectory : public CxxTest::TestSuite {
 public:
    void testFileAccess();
    void testDisabledFileAccess();
    void testDirectoryAccess();
    void testFragmentRedirect();
    void testNoFragmentRedirect();
};

class TestServerHostSpecPublisher : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestServerHostSpecPublisherImpl : public CxxTest::TestSuite {
 public:
    void testBeams();
    void testConfig();
    void testEngines();
    void testFCodes();
    void testFlakConfig();
    void testFlakConfigSeparate();
    void testTorps();
    void testTruehull();
    void testHullfunc();
    void testHulls();
    void testSingleHull();
    void testFilesFromDefault();
    void testMultiple();
    void testErrorNoFile();
    void testErrorBadKeys();
};

#endif
