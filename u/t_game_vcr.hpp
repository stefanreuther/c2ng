/**
  *  \file u/t_game_vcr.hpp
  *  \brief Tests for game::vcr
  */
#ifndef C2NG_U_T_GAME_VCR_HPP
#define C2NG_U_T_GAME_VCR_HPP

#include <cxxtest/TestSuite.h>

class TestGameVcrBattle : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrDatabase : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameVcrObject : public CxxTest::TestSuite {
 public:
    void testGetSet();
    void testAdd();
};

class TestGameVcrScore : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
