/**
  *  \file u/t_game_msg.hpp
  *  \brief Tests for game::msg
  */
#ifndef C2NG_U_T_GAME_MSG_HPP
#define C2NG_U_T_GAME_MSG_HPP

#include <cxxtest/TestSuite.h>

class TestGameMsgBrowser : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testUnfiltered();
    void testAllFiltered();
    void testSummary();
    void testSearch();
};

class TestGameMsgConfiguration : public CxxTest::TestSuite {
 public:
    void testIt();
    void testLoad();
    void testSave();
    void testSaveEmpty();
};

class TestGameMsgFile : public CxxTest::TestSuite {
 public:
    void testWriteSingle();
    void testWriteMulti();
    void testWriteDifferentTurns();
    void testLoad();
    void testLoadEmpty();
    void testLoadUndelimited();
    void testLoadTurn();
};

class TestGameMsgFormat : public CxxTest::TestSuite {
 public:
    void testFormatMessage();
    void testQuoteMessageForReply();
};

class TestGameMsgInbox : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testHeaders();
    void testSort();
    void testReceive();
    void testAutoReceive();
    void testReceiveErrors();
    void testPrimaryLink();
};

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

class TestGameMsgSubsetMailbox : public CxxTest::TestSuite {
 public:
    void testIt();
};

#endif
