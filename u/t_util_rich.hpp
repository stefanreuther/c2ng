/**
  *  \file u/t_util_rich.hpp
  *  \brief Tests for util::rich
  */
#ifndef C2NG_U_T_UTIL_RICH_HPP
#define C2NG_U_T_UTIL_RICH_HPP

#include <cxxtest/TestSuite.h>

class TestUtilRichAttribute : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRichColorAttribute : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRichLinkAttribute : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRichParser : public CxxTest::TestSuite {
 public:
    void testParseXml();
};

class TestUtilRichStyleAttribute : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRichText : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilRichVisitor : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
