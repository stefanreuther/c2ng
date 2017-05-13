/**
  *  \file u/t_ui_res.hpp
  *  \brief Tests for ui::res
  */
#ifndef C2NG_U_T_UI_RES_HPP
#define C2NG_U_T_UI_RES_HPP

#include <cxxtest/TestSuite.h>

class TestUiResCCImageLoader : public CxxTest::TestSuite {
 public:
    void testCompressedCD();
    void testUncompressedGFX();
    void testCompressedCC();
};

class TestUiResEngineImageLoader : public CxxTest::TestSuite {
 public:
    void testOK();
    void testFail();
};

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
    void testOpen();
};

class TestUiResResId : public CxxTest::TestSuite {
 public:
    void testMake();
    void testGeneralize();
    void testMatch();
};

#endif
