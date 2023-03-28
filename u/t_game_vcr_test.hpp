/**
  *  \file u/t_game_vcr_test.hpp
  *  \brief Tests for game::vcr::test
  */
#ifndef C2NG_U_T_GAME_VCR_TEST_HPP
#define C2NG_U_T_GAME_VCR_TEST_HPP

#include <cxxtest/TestSuite.h>

class TestGameVcrTestBattle : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGroups();
};

class TestGameVcrTestDatabase : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
