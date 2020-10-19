/**
  *  \file u/t_game_msg.hpp
  *  \brief Tests for game::msg
  */
#ifndef C2NG_U_T_GAME_MSG_HPP
#define C2NG_U_T_GAME_MSG_HPP

#include <cxxtest/TestSuite.h>

class TestGameMsgMailbox : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMsgOutbox : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMerge();
    void testAddDelete();
    void testModify();
    void testOutOfRange();
};

#endif
