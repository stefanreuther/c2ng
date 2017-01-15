/**
  *  \file u/t_ui_res.hpp
  *  \brief Tests for ui::res
  */
#ifndef C2NG_U_T_UI_RES_HPP
#define C2NG_U_T_UI_RES_HPP

#include <cxxtest/TestSuite.h>

class TestUiResImageLoader : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiResManager : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLoad();
    void testRemove();
};

class TestUiResProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestUiResResId : public CxxTest::TestSuite {
 public:
    void testMake();
    void testGeneralize();
    void testMatch();
};

#endif
