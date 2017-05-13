/**
  *  \file u/t_game_maint.hpp
  *  \brief Tests for game::maint
  */
#ifndef C2NG_U_T_GAME_MAINT_HPP
#define C2NG_U_T_GAME_MAINT_HPP

#include <cxxtest/TestSuite.h>

class TestGameMaintDifficultyRater : public CxxTest::TestSuite {
 public:
    void testSimple();
};

class TestGameMaintSweeper : public CxxTest::TestSuite {
 public:
    void testScan();
    void testRemove();
    void testRemoveLast();
    void testRemoveDB();
    void testRemoveDBLast();
};

#endif
