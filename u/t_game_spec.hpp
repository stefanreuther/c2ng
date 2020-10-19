/**
  *  \file u/t_game_spec.hpp
  */
#ifndef C2NG_U_T_GAME_SPEC_HPP
#define C2NG_U_T_GAME_SPEC_HPP

#include <cxxtest/TestSuite.h> 

class TestGameSpecBaseComponentVector : public CxxTest::TestSuite {
 public:
    void testIt();
    void testOutOfRange();
};

class TestGameSpecBasicHullFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testExplain();
};

class TestGameSpecBasicHullFunctionList : public CxxTest::TestSuite {
 public:
    void testIO();
    void testMatch();
    void testMatchLoop();
    void testMatchUnterminated();
    void testErrors();
    void testBug342();
};

class TestGameSpecBeam : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDerivedInformation();
};

class TestGameSpecComponent : public CxxTest::TestSuite {
 public:
    void testData();
    void testName();
};

class TestGameSpecComponentNameProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecComponentVector : public CxxTest::TestSuite {
 public:
    void testCreate();
    void testName();
};

class TestGameSpecCost : public CxxTest::TestSuite {
 public:
    void testParse();
    void testAdd();             // +=
    void testSubtract();        // -=
    void testMult();            // *=, *
    void testCompare();         // ==, !=
    void testEnough();          // isEnoughFor()
    void testGetMaxAmount();
    void testDivi();
    void testFormat();
};

class TestGameSpecEngine : public CxxTest::TestSuite {
 public:
    void testFuelDefaults();
    void testFuel();
};

class TestGameSpecFighter : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecFriendlyCode : public CxxTest::TestSuite {
 public:
    void testFCode();
    void testFCodeFail();
    void testData();
    void testDescription();
    void testWorksOn();
    void testWorksOnShip();
    void testIsPermitted();
};

class TestGameSpecFriendlyCodeList : public CxxTest::TestSuite {
 public:
    void testNumeric();
    void testRandom();
    void testContainer();
    void testSpecial();
    void testGenerateRandom();
    void testUniversalMF();
    void testGenerateRandomLoop();
    void testGenerateRandomBlock();
    void testLoad();
    void testSort();
    void testSyntaxErrors();
    void testPessimistic();
    void testPack();
    void testLoadExtraDup();
};

class TestGameSpecHull : public CxxTest::TestSuite {
 public:
    void testIt();
    void testHullFunctions();
    void testFuelUsage();
    void testCloakFuelUsage();
    void testMineHitDamage();
};

class TestGameSpecHullAssignmentList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testPlayerRace();
};

class TestGameSpecHullFunction : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCompare();
    void testGetDefault();
};

class TestGameSpecHullFunctionAssignmentList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testGetPlayerImplied();
    void testMerged();
    void testDefaulted();
    void testRemove();
    void testSequence();
};

class TestGameSpecHullFunctionList : public CxxTest::TestSuite {
 public:
    void testSimplify();
    void testSort();
    void testIt();
    void testSortLevels();
    void testSimplifyEmpty();
    void testSimplifySingle();
    void testSimplifyMerge();
    void testSimplifyRace();
    void testSimplifyRace2();
    void testSimplifyNotRace();
    void testSimplifyGeneral();
    void testSimplifyNullAssignment();
};

class TestGameSpecMission : public CxxTest::TestSuite {
 public:
    void testData();
    void testConstruct();
    void testDefault();
};

class TestGameSpecMissionList : public CxxTest::TestSuite {
 public:
    void testMissionIni();
    void testMissionIniRaces();
    void testLoad();
    void testAddMerge();
    void testAddMergeLetters();
    void testAddMergeLetters2();
};

class TestGameSpecModifiedHullFunctionList : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecNullComponentNameProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecRacialAbilityList : public CxxTest::TestSuite {
 public:
    void testConfigAbilities();
    void testCategories();
    void testShip();
    void testFilter();
    void testOrigin();
};

class TestGameSpecShipList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testRacialAbilities();
    void testRacialAbilitiesSSD();
    void testRacialAbilitiesSparse();
    void testRacialAbilitiesOne();
    void testRacialAbilitiesEmpty();
    void testRacialAbilitiesFail();
    void testGetHullFunctions();
    void testFindRacialAbilitiesMany();
    void testFindRacialAbilitiesHoley();
};

class TestGameSpecStandardComponentNameProvider : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecTorpedo : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSpecTorpedoLauncher : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDerivedInformation();
};

class TestGameSpecWeapon : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDeathRay();
};

#endif
