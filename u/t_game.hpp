/**
  *  \file u/t_game.hpp
  *  \brief Tests for game
  */
#ifndef C2NG_U_T_GAME_HPP
#define C2NG_U_T_GAME_HPP

#include <cxxtest/TestSuite.h>

class TestGameAuthCache : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMulti();
    void testFail();
};

class TestGameBattleOrderRule : public CxxTest::TestSuite {
 public:
    void testGetShipBattleOrder();
    void testGetPlanetBattleOrder();
};

class TestGameCargoContainer : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testValidImpossible();
    void testInitial();
    void testOverload();
};

class TestGameCargoSpec : public CxxTest::TestSuite {
 public:
    void testParse();
    void testParseError();
    void testAdd();
    void testSubtract();
    void testMult();
    void testCompare();
    void testMixedCompare();
    void testDivide1();
    void testDivide2();
    void testToString();
    void testSellSuppliesIfNeeded();
};

class TestGameElement : public CxxTest::TestSuite {
 public:
    void testOperator();
    void testTorpedo();
    void testIteration();
    void testName();
};

class TestGameException : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameExperienceLevelSet : public CxxTest::TestSuite {
 public:
    void testPreconditions();
    void testFormat();
};

class TestGameExtra : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameExtraContainer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameExtraIdentifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameGame : public CxxTest::TestSuite {
 public:
    void testRef();
};

class TestGameHistoryTurn : public CxxTest::TestSuite {
 public:
    void testSet();
    void testSuccess();
    void testFail();
};

class TestGameHistoryTurnList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGap();
};

class TestGameHostVersion : public CxxTest::TestSuite {
 public:
    void testFormat();
    void testAccessor();
    void testVersion();
    void testProperties();
    void testSetImpliedHostConfiguration();
};

class TestGameInterpreterInterface : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameLimits : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGamePlayer : public CxxTest::TestSuite {
 public:
    void testSet();
    void testInit();
    void testChange();
    void testOriginal();
};

class TestGamePlayerArray : public CxxTest::TestSuite {
 public:
    void testArray();
};

class TestGamePlayerBitMatrix : public CxxTest::TestSuite {
 public:
    void testMatrix();
};

class TestGamePlayerList : public CxxTest::TestSuite {
 public:
    void testExpand();
    void testIteration();
    void testSetup();
    void testChar();
    void testNotify();
    void testGetName();
};

class TestGamePlayerSet : public CxxTest::TestSuite {
 public:
    void testFormat();
    void testFormatPlayerHostSet();
};

class TestGameRegistrationKey : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameRoot : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSearchQuery : public CxxTest::TestSuite {
 public:
    void testCompileExpression();
    void testErrors();
    void testLocation();
    void testAccessor();
};

class TestGameShipBuildOrder : public CxxTest::TestSuite {
 public:
    void testIt();
    void testComparison();
    void testCanonicalize();
};

class TestGameSpecificationLoader : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameStringVerifier : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameTeamSettings : public CxxTest::TestSuite {
 public:
    void testInit();
    void testSet();
    void testModify();
    void testViewpoint();
    void testLoadSave();
};

class TestGameTimestamp : public CxxTest::TestSuite {
 public:
    void testInit();
    void testRelation();
};

class TestGameTurn : public CxxTest::TestSuite {
 public:
    void testIt();
    void testNotify();
};

class TestGameTurnLoader : public CxxTest::TestSuite {
 public:
    void testInterface();
    void testDefault();
};

class TestGameTypes : public CxxTest::TestSuite {
 public:
    void testNativeRace();
    void testShipyardAction();
    void testPlanetaryBuilding();
    void testIntegerProperty();
    void testLongProperty();
    void testNegativeProperty();
    void testStringProperty();
};

class TestGameUnitScoreDefinitionList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCopy();
};

class TestGameUnitScoreList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCopy();
    void testMerge();
};

#endif
