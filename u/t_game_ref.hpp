/**
  *  \file u/t_game_ref.hpp
  *  \brief Tests for game::ref
  */
#ifndef C2NG_U_T_GAME_REF_HPP
#define C2NG_U_T_GAME_REF_HPP

#include <cxxtest/TestSuite.h>

class TestGameRefFleetList : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testSort1();
    void testSort2();
    void testAdd();
};

class TestGameRefHistoryShipList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSort1();
    void testSort2();
};

class TestGameRefHistoryShipSelection : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testModeSet();
    void testBuildList();
    void testBuildListHist();
    void testBuildListHist2();
};

class TestGameRefNullPredicate : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameRefSortPredicate : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testThen();
};

#endif
