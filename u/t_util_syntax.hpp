/**
  *  \file u/t_util_syntax.hpp
  *  \brief Tests for util::syntax
  */
#ifndef C2NG_U_T_UTIL_SYNTAX_HPP
#define C2NG_U_T_UTIL_SYNTAX_HPP

#include <cxxtest/TestSuite.h>

class TestUtilSyntaxCHighlighter : public CxxTest::TestSuite {
 public:
    void testPreproc();
    void testString();
    void testIdentifiers();
    void testComment();
    void testC();
    void testCXX();
    void testJS();
    void testJava();
};

class TestUtilSyntaxFactory : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSyntaxFormat : public CxxTest::TestSuite {
 public:
    void testHeader();
};

class TestUtilSyntaxHighlighter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUtilSyntaxIniHighlighter : public CxxTest::TestSuite {
 public:
    void testComments();
    void testSections();
    void testAssignment();
};

class TestUtilSyntaxKeywordTable : public CxxTest::TestSuite {
 public:
    void testAccess();
    void testLoadErrors();
    void testLoad();
};

class TestUtilSyntaxNullHighlighter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUtilSyntaxScriptHighlighter : public CxxTest::TestSuite {
 public:
    void testString();
    void testDeclarations();
    void testCommands();
};

class TestUtilSyntaxSegment : public CxxTest::TestSuite {
 public:
    void testSet();
    void testStartFinish();
};

#endif
