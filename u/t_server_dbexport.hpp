/**
  *  \file u/t_server_dbexport.hpp
  *  \brief Tests for server::dbexport
  */
#ifndef C2NG_U_T_SERVER_DBEXPORT_HPP
#define C2NG_U_T_SERVER_DBEXPORT_HPP

#include <cxxtest/TestSuite.h>

class TestServerDbexportDbExporter : public CxxTest::TestSuite {
 public:
    void testTypes();
    void testStrings();
    void testLargeList();
    void testManyList();
    void testLargeSet();
    void testManySet();
    void testLargeHash();
    void testManyHash();
};

#endif
