/**
  *  \file u/t_game_map.hpp
  *  \brief Tests for game::map
  */
#ifndef C2NG_U_T_GAME_MAP_HPP
#define C2NG_U_T_GAME_MAP_HPP

#include <cxxtest/TestSuite.h>

class TestGameMapCircularObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapConfiguration : public CxxTest::TestSuite {
 public:
    void testFlat();
    void testFlatSmall();
    void testFlatOffset();
    void testWrapped();
    void testWrappedSmall();
};

class TestGameMapMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectCursor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectList : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testContent();
};

class TestGameMapObjectReference : public CxxTest::TestSuite {
 public:
    void testCompare();
    void testAccessor();
};

class TestGameMapPoint : public CxxTest::TestSuite {
 public:
    void testBasics();
};

class TestGameMapViewport : public CxxTest::TestSuite {
 public:
    void testRectangle();
};

#endif
