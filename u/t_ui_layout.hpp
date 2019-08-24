/**
  *  \file u/t_ui_layout.hpp
  *  \brief Tests for ui::layout
  */
#ifndef C2NG_U_T_UI_LAYOUT_HPP
#define C2NG_U_T_UI_LAYOUT_HPP

#include <cxxtest/TestSuite.h>

class TestUiLayoutGrid : public CxxTest::TestSuite {
 public:
    void testFixed();
    void testEmpty();
};

class TestUiLayoutInfo : public CxxTest::TestSuite {
 public:
    void testInit();
    void testInitFixed();
    void testInitIgnored();
    void testAnd();
    void testMake();
};

#endif
