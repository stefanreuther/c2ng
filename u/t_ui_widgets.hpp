/**
  *  \file u/t_ui_widgets.hpp
  *  \brief Tests for ui::widgets
  */
#ifndef C2NG_U_T_UI_WIDGETS_HPP
#define C2NG_U_T_UI_WIDGETS_HPP

#include <cxxtest/TestSuite.h>

class TestUiWidgetsAlignedContainer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiWidgetsBaseButton : public CxxTest::TestSuite {
 public:
    void testKeyboard();
};

class TestUiWidgetsChart : public CxxTest::TestSuite {
 public:
    void testRender();
    void testRenderExtend();
    void testRenderSkip();
    void testRenderAntiAlias();
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

class TestUiWidgetsIconGrid : public CxxTest::TestSuite {
 public:
    void testInit();
    void testScroll();
    void testKeySingle();
    void testKeyMulti();
    void testScrollPageTop();
    void testInaccessible();
};

class TestUiWidgetsInputLine : public CxxTest::TestSuite {
 public:
    void testInsert();
    void testHandleKey();
};

class TestUiWidgetsRadioButton : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLifetime();
};

class TestUiWidgetsTreeListbox : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
