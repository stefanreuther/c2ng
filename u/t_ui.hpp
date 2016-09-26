/**
  *  \file u/t_ui.hpp
  *  \brief Tests for ui
  */
#ifndef C2NG_U_T_UI_HPP
#define C2NG_U_T_UI_HPP

#include <cxxtest/TestSuite.h>

class TestUiColorScheme : public CxxTest::TestSuite {
 public:
    void testBackground();
    void testColor();
};

class TestUiGroup : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiInvisibleWidget : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiWidget : public CxxTest::TestSuite {
 public:
    void testDeathFocus();
    void testRemoveFocus();
};

#endif
