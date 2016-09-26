/**
  *  \file u/t_ui_widgets.hpp
  *  \brief Tests for ui::widgets
  */
#ifndef C2NG_U_T_UI_WIDGETS_HPP
#define C2NG_U_T_UI_WIDGETS_HPP

#include <cxxtest/TestSuite.h>

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

#endif
