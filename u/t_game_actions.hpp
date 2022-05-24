/**
  *  \file u/t_game_actions.hpp
  *  \brief Tests for game::actions
  */
#ifndef C2NG_U_T_GAME_ACTIONS_HPP
#define C2NG_U_T_GAME_ACTIONS_HPP

#include <cxxtest/TestSuite.h>

class TestGameActionsBaseBuildAction : public CxxTest::TestSuite {
 public:
    void testError();
    void testSuccess();
    void testCostSummary();
};

class TestGameActionsBaseBuildExecutor : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameActionsBaseFixRecycle : public CxxTest::TestSuite {
 public:
    void testNoBase();
    void testEmpty();
    void testNormal();
    void testSet();
    void testSetFail();
};

class TestGameActionsBuildAmmo : public CxxTest::TestSuite {
 public:
    void testFail();
    void testSuccess();
    void testLimitCapacity();
    void testLimitResource();
    void testLimitKey();
    void testNoLimitKey();
    void testLimitKeyDowngrade();
    void testLimitKeyDowngradeNoListener();
    void testLimitTechCost();
    void testSellNoReverter();
    void testSellReverter();
    void testInvalidTypes();
    void testDifferentContainers();
    void testIsValidCombination();
};

class TestGameActionsBuildParts : public CxxTest::TestSuite {
 public:
    void testBuild();
    void testAdd();
    void testModify();
    void testModifyTech();
    void testRevert();
    void testShipBuild();
    void testShipBuildMin();
    void testShipBuildOther();
};

class TestGameActionsBuildShip : public CxxTest::TestSuite {
 public:
    void testError();
    void testSuccess();
    void testNoBeams();
    void testInitialTech();
    void testTechUpgrade();
    void testTechUpgradeFail();
    void testUseParts();
    void testUsePartsPartial();
    void testPreexistingOrder();
    void testForeignShip();
    void testTechDisabled();
    void testModify();
    void testBadId();
    void testBadHull();
    void testBadEngine();
    void testBadBeam();
    void testBadLauncher();
    void testCostSummary();
};

class TestGameActionsBuildStarbase : public CxxTest::TestSuite {
 public:
    void testError();
    void testErrorNullOp();
    void testNormal();
    void testModify();
    void testConfigChange();
    void testConfigChangeSignal();
    void testTooExpensive();
};

class TestGameActionsBuildStructures : public CxxTest::TestSuite {
 public:
    void testError();
    void testSuccess();
    void testModify();
    void testMulti();
    void testResourceLimit();
    void testAutoBuild();
    void testAutoBuildGroup();
    void testBuildFailure();
};

class TestGameActionsCargoCostAction : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testMissingMineral();
    void testMissingMoney();
    void testMissingSupplies();
    void testMissingLotsOfMoney();
    void testMultiModification();
    void testUnderlayingChange();
    void testReservedAmount();
    void testReservedMoney();
    void testReservedMuchMoney();
};

class TestGameActionsCargoTransfer : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testUnloadNoSource();
    void testUnloadNoTarget();
    void testUnloadMultipleTarget();
    void testUnloadNormal();
    void testUnloadSell();
    void testLimitRoom();
    void testLimitTypes();
    void testSupplySale();
    void testOverloadEmpty();
    void testOverloadBefore();
    void testOverloadAfter();
    void testTemporary();
    void testCargoSpec();
    void testCargoSpecSupplySale();
    void testHoldSpace();
    void testMoveExt();
    void testMoveExtReverse();
    void testDistributeEqually();
    void testDistributeFreeSpace();
    void testDistributeProportionally();
    void testMoveAll();
};

class TestGameActionsCargoTransferSetup : public CxxTest::TestSuite {
 public:
    void testInit();
    void testCreateNonexistant();
    void testOwnShipOwnShip();
    void testOwnShipOwnShipDirect();
    void testOwnShipAlliedShip();
    void testOwnShipAlliedShipDirect();
    void testOwnShipForeignShip();
    void testForeignShipOwnShip();
    void testForeignShipForeignShip();
    void testOwnShipForeignShipConflict();
    void testOwnShipForeignShipAutoCancel();
    void testShipMismatch();
    void testJettisonNormal();
    void testJettisonFail();
    void testJettisonFailPlanet();
    void testJettisonDirect();
    void testOwnPlanetOwnShip();
    void testOwnPlanetOwnShipDirect();
    void testOwnPlanetAlliedShip();
    void testOwnPlanetForeignShip();
    void testForeignPlanetOwnShip();
    void testForeignShipOwnPlanet();
    void testOwnPlanetForeignShipConflict();
};

class TestGameActionsChangeBuildQueue : public CxxTest::TestSuite {
 public:
    void testBasic();
    void testIncrease();
    void testDecrease();
    void testPBPs();
    void testClone();
    void testPlannedBuild();
};

class TestGameActionsChangeShipFriendlyCode : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testAvoidNew();
    void testAvoidOld();
    void testAvoidRevert();
};

class TestGameActionsCloneShip : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testNormalPayTech();
    void testNormalPayNone();
    void testCanBuild();
    void testTholianHost();
    void testTholianPHost();
    void testTechLimit();
    void testRemoteOwner();
    void testUnclonable();
    void testFriendlyCodeGood();
    void testFriendlyCodeBad();
    void testFriendlyCodeOpen();
    void testNoConflict();
    void testBuildConflict();
    void testCloneConflict();
    void testCloneConflictSelf();
    void testCloneConflictMore();
    void testCloneOnce();
    void testFleet();
};

class TestGameActionsConvertSupplies : public CxxTest::TestSuite {
 public:
    void testNormal();
    void testReserved();
    void testBuy();
};

class TestGameActionsMultiTransferSetup : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testError();
    void testNormal();
    void testNoCargo();
    void testNoPeer();
    void testFleet();
    void testTypeMismatch();
};

class TestGameActionsPreconditions : public CxxTest::TestSuite {
 public:
    void testShip();
    void testPlanet();
    void testBase();
    void testSession();
};

class TestGameActionsRemoteControlAction : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testOwn();
    void testOwnDrop();
    void testOwnDisabled();
    void testOwnControlled();
    void testForeign();
    void testForeignDisabled();
    void testForeignThird();
    void testForeignControlled();
};

class TestGameActionsTaxationAction : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testNormalTim();
    void testIncomeLimit();
    void testChangeRevenue();
    void testModifyRevert();
    void testSafeTax();
    void testSetNumBuildings();
    void testDescribe();
    void testDescribeHiss();
};

class TestGameActionsTechUpgrade : public CxxTest::TestSuite {
 public:
    void testFail();
    void testSimple();
    void testRevertable();
    void testRevertableChange();
    void testRevertableShip();
    void testRevertableNoSignal();
};

class TestGameActionsbuildParts : public CxxTest::TestSuite {
 public:
    void testBuildMultiCommit();
};

#endif
