/**
  *  \file u/t_gfx.hpp
  *  \brief Tests for gfx
  */
#ifndef C2NG_U_T_GFX_HPP
#define C2NG_U_T_GFX_HPP

#include <cxxtest/TestSuite.h>

class TestGfxCanvas : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxEngine : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxEventConsumer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxFillPattern : public CxxTest::TestSuite {
 public:
    void testInit();
    void testOperators();
    void testPredefined();
};

class TestGfxNullCanvas : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxNullEngine : public CxxTest::TestSuite {
 public:
    void testTimers();
    void testEvents();
};

class TestGfxPoint : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxRectangle : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxResourceProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxTimer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGfxTimerQueue : public CxxTest::TestSuite {
 public:
    void test1();
    void test2();
};

#endif
