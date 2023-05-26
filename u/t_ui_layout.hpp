/**
  *  \file u/t_ui_layout.hpp
  *  \brief Tests for ui::layout
  */
#ifndef C2NG_U_T_UI_LAYOUT_HPP
#define C2NG_U_T_UI_LAYOUT_HPP

#include <cxxtest/TestSuite.h>

class TestUiLayoutAxisLayout : public CxxTest::TestSuite {
 public:
    void testData();
    void testLayout();
};

class TestUiLayoutGrid : public CxxTest::TestSuite {
 public:
    void testFixed();
    void testEmpty();
};

class TestUiLayoutHBox : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
    void testSingle();
};

class TestUiLayoutInfo : public CxxTest::TestSuite {
 public:
    void testInit();
    void testInitFixed();
    void testInitIgnored();
    void testAnd();
    void testMake();
};

class TestUiLayoutManager : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUiLayoutVBox : public CxxTest::TestSuite {
 public:
    void testIt();
    void testEmpty();
    void testSingle();
};

#endif
