/**
  *  \file u/t_game_score.hpp
  *  \brief Tests for game::score
  */
#ifndef C2NG_U_T_GAME_SCORE_HPP
#define C2NG_U_T_GAME_SCORE_HPP

#include <cxxtest/TestSuite.h>

class TestGameScoreChartBuilder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testTeam();
    void testCumulative();
    void testSparse();
};

class TestGameScoreCompoundScore : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCompare();
};

class TestGameScoreLoader : public CxxTest::TestSuite {
 public:
    void testSimple();
    void testErrors();
    void testLoadOld();
    void testLoadOldErrors();
    void testSave();
};

class TestGameScoreScoreBuilderBase : public CxxTest::TestSuite {
 public:
    void testSpecials();
};

class TestGameScoreScoreId : public CxxTest::TestSuite {
 public:
};

class TestGameScoreStructures : public CxxTest::TestSuite {
 public:
};

class TestGameScoreTableBuilder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testTeams();
};

class TestGameScoreTurnScore : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameScoreTurnScoreList : public CxxTest::TestSuite {
 public:
    void testSchema();
    void testDescription();
    void testTurn();
    void testDescriptionConstructor();
    void testAddMessageInformationComplete();
    void testAddMessageInformationJustId();
    void testAddMessageInformationJustName();
    void testAddMessageInformationJustNameNew();
};

#endif
