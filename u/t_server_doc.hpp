/**
  *  \file u/t_server_doc.hpp
  *  \brief Tests for server::doc
  */
#ifndef C2NG_U_T_SERVER_DOC_HPP
#define C2NG_U_T_SERVER_DOC_HPP

#include <cxxtest/TestSuite.h>

class TestServerDocDocumentationImpl : public CxxTest::TestSuite {
 public:
    void testGetBlob();
    void testNodeAccess();
};

#endif
