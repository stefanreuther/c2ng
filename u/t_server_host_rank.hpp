/**
  *  \file u/t_server_host_rank.hpp
  *  \brief Tests for server::host::rank
  */
#ifndef C2NG_U_T_SERVER_HOST_RANK_HPP
#define C2NG_U_T_SERVER_HOST_RANK_HPP

#include <cxxtest/TestSuite.h>

class TestServerHostRankLevelHandler : public CxxTest::TestSuite {
 public:
    void testTurnSubmission();
    void testTurnMiss();
    void testDropTurn0();
    void testDropScoreless();
    void testDropZeroScore();
    void testDropMidScore();
    void testDropHighScore();
    void testRankPoints();
    void testPromote();
    void testDemote();
};

class TestServerHostRankRank : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testNotPlaying();
    void testTurnOverLimit();
    void testTurnOverLimit2();
    void testPartial();
    void testPartial2();
};

class TestServerHostRankRefereeFileReader : public CxxTest::TestSuite {
 public:
    void testIt();
    void testFile();
};

class TestServerHostRankScoreFileReader : public CxxTest::TestSuite {
 public:
    void testPackScore();
    void testParse();
    void testFile();
};

class TestServerHostRankVictory : public CxxTest::TestSuite {
 public:
    void testRankingBasic();
    void testRankingShort();
    void testRankingOrder();
    void testRankingReplacement();
    void testRankingDifferent();
    void testRankingLate();
    void testRankingUndo();
    void testRankingPredef();
    void testScoreCondition();
};

#endif
