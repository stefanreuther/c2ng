/**
  *  \file u/t_ui_widgets.hpp
  *  \brief Tests for ui::widgets
  */
#ifndef C2NG_U_T_UI_WIDGETS_HPP
#define C2NG_U_T_UI_WIDGETS_HPP

#include <cxxtest/TestSuite.h>

class TestUiWidgetsAbstractButton : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiWidgetsCheckbox : public CxxTest::TestSuite {
 public:
    void testCycle();
    void testLifetime();
};

class TestUiWidgetsFocusIterator : public CxxTest::TestSuite {
 public:
    void testTab();
    void testEmpty();
    void testDisabled();
    void testDisabledWrap();
    void testDisabledHome();
    void testVertical();
    void testVerticalWrap();
    void testVerticalTab();
    void testOther();
};

class TestUiWidgetsInputLine : public CxxTest::TestSuite {
 public:
    void testInsert();
};

class TestUiWidgetsRadioButton : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLifetime();
};

#endif
