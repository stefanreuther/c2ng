/**
  *  \file u/t_game_ref.hpp
  *  \brief Tests for game::ref
  */
#ifndef C2NG_U_T_GAME_REF_HPP
#define C2NG_U_T_GAME_REF_HPP

#include <cxxtest/TestSuite.h>

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
