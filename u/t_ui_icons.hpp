/**
  *  \file u/t_ui_icons.hpp
  *  \brief Tests for ui::icons
  */
#ifndef C2NG_U_T_UI_ICONS_HPP
#define C2NG_U_T_UI_ICONS_HPP

#include <cxxtest/TestSuite.h>

class TestUiIconsColorTile : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testNoFrame();
    void testResize();
    void testFrameType();
};

class TestUiIconsHBox : public CxxTest::TestSuite {
 public:
    void testDefault();
    void testTop();
    void testPad();
    void testRight();
};

class TestUiIconsIcon : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestUiIconsSpacer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiIconsVBox : public CxxTest::TestSuite {
 public:
    void testDefault();
    void testParam();
};

#endif
