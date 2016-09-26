/**
  *  \file u/t_game_browser.hpp
  *  \brief Tests for game::browser
  */
#ifndef C2NG_U_T_GAME_BROWSER_HPP
#define C2NG_U_T_GAME_BROWSER_HPP

#include <cxxtest/TestSuite.h>

class TestGameBrowserAccount : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testPersistent();
    void testEncode();
};

class TestGameBrowserFolder : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameBrowserHandler : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameBrowserHandlerList : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameBrowserUnsupportedAccountFolder : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
