/**
  *  \file u/t_game_map.hpp
  *  \brief Tests for game::map
  */
#ifndef C2NG_U_T_GAME_MAP_HPP
#define C2NG_U_T_GAME_MAP_HPP

#include <cxxtest/TestSuite.h>

class TestGameMapBaseStorage : public CxxTest::TestSuite {
 public:
    void testAccess();
    void testValid();
    void testClear();
};

class TestGameMapBeamUpPlanetTransfer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testCommand();
};

class TestGameMapBeamUpShipTransfer : public CxxTest::TestSuite {
 public:
    void testParse();
    void testCommand();
};

class TestGameMapBeamupShipTransfer : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapChunnelMission : public CxxTest::TestSuite {
 public:
    void testRangesPHost();
    void testRangesTHost();
    void testAbilities();
    void testCombinationAbilities();
};

class TestGameMapCircularObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapConfiguration : public CxxTest::TestSuite {
 public:
    void testFlat();
    void testFlatSmall();
    void testFlatOffset();
    void testWrapped();
    void testWrappedSmall();
    void testCircular();
};

class TestGameMapExplosion : public CxxTest::TestSuite {
 public:
    void testInit();
    void testName();
    void testMergeFail();
    void testMergeSuccess();
};

class TestGameMapExplosionType : public CxxTest::TestSuite {
 public:
    void testInit();
    void testIteration();
    void testAddMessageInformation();
};

class TestGameMapFleetMember : public CxxTest::TestSuite {
 public:
    void testSetFleetName();
    void testSetWaypoint();
    void testSetWarpFactor();
    void testSetMission();
    void testSetMissionToIntercept();
    void testSetMissionFromIntercept();
    void testSetFleetNumberFail();
    void testSetFleetNumberSuccess();
    void testSetFleetNumberDropLeader();
    void testSetFleetNumberDropMember();
    void testSetFleetNumberMoveMember();
    void testSetMissionTow();
    void testSetMissionTowOther();
    void testSetMissionTowInvalid();
    void testIsMissionLocked();
    void testIsMissionLockedMutex();
    void testSetFleetNumberForeign();
};

class TestGameMapIonStorm : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMessageInfoClear();
    void testMessageInfoMin();
    void testMessageInfoMax();
    void testMessageInfoMissing();
};

class TestGameMapLocation : public CxxTest::TestSuite {
 public:
    void testPoint();
    void testRef();
    void testUniv();
};

class TestGameMapLocationReverter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameMapLocker : public CxxTest::TestSuite {
 public:
    void testPoint();
    void testPointLimit();
    void testNull();
    void testFiltered();
    void testPlanets();
    void testShips();
    void testUfos();
    void testMinefields();
    void testDrawings();
    void testExplosions();
    void testWrap();
    void testCircular();
};

class TestGameMapMinefield : public CxxTest::TestSuite {
 public:
    void testUnitsAfterDecayHost();
    void testUnitsAfterDecayPHost();
    void testInit();
    void testInitEmpty();
    void testAddReport();
    void testGetPassRate();
};

class TestGameMapMinefieldMission : public CxxTest::TestSuite {
 public:
    void testInit();
    void testLayEmptyShip();
    void testLayFreighter();
    void testLayOther();
    void testLayNormal();
    void testLayNormalDisabled();
    void testLayRobot();
    void testLayDropFCode();
    void testLayDropFCodeDisallowed();
    void testLayDropFCodeInapplicable();
    void testLayIdentityFCode();
    void testLayIdentityFCodeRobot();
    void testLayWeb();
    void testLayWebDisabled();
    void testLayWebWrongRace();
    void testLayExtended();
    void testLayWebExtended();
    void testLayInExtended();
    void testLayWebInExtended();
    void testLayExtendHost();
    void testLayExtendHostFail();
    void testLayExtendPHost();
    void testLayExtendId();
    void testLayExtendIdMissing();
    void testLayExtendIdMismatch();
    void testScoopEmpty();
    void testScoopFreighter();
    void testScoopFCode();
    void testScoopFCodeDisabled();
    void testScoopFCodeUnregistered();
    void testScoopFCodeNoBeamsHost();
    void testScoopFCodeNoBeamsPHost();
    void testScoopMission();
    void testScoopMissionUnregistered();
};

class TestGameMapMinefieldType : public CxxTest::TestSuite {
 public:
    void testInit();
    void testIteration();
    void testAddMessageInformation();
    void testAddMessageInformationFull();
    void testAddMessageInformationMinUpdate();
    void testAddMessageInformationMinFail();
    void testErase();
    void testAllMinefieldsKnown();
};

class TestGameMapMovementPredictor : public CxxTest::TestSuite {
 public:
    void testCombinations();
    void testMovement();
    void testInterceptLoop();
};

class TestGameMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectCursor : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapObjectCursorFactory : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameMapPlanet : public CxxTest::TestSuite {
 public:
    void testAutobuildSettings();
};

class TestGameMapPlanetFormula : public CxxTest::TestSuite {
 public:
    void testGetColonistChange();
    void testTaxSeriesTHost();
    void testTaxSeriesPHost();
    void testTemperatureSeriesFedTHost();
    void testTemperatureSeriesFedPHost();
    void testTemperatureSeriesCryTHost();
    void testTemperatureSeriesCryPHost();
    void testBuildingSeriesTHost();
    void testBuildingSeriesPHost();
    void testNativeTaxSeriesTHost();
    void testNativeTaxSeriesPHost();
    void testNativeTaxBuildingSeriesTHost();
    void testNativeTaxBuildingSeriesPHost();
    void testBuildingLimitSeries();
    void testMaxColonistSeriesNormalTHost();
    void testMaxColonistSeriesNormalPHost();
    void testMaxColonistSeriesRebelTHost();
    void testMaxColonistSeriesRebelPHost();
    void testMaxColonistSeriesKlingonTHost();
    void testMaxColonistSeriesKlingonPHost();
    void testMaxColonistSeriesCrystalTHost();
    void testMaxColonistSeriesCrystalPHost();
    void testMaxColonistSeriesCrystalSinTemp();
};

class TestGameMapPlanetInfo : public CxxTest::TestSuite {
 public:
    void testPackPlanetMineralInfo();
    void testPackPlanetMineralInfoMineOverride();
    void testPackPlanetMineralInfoEmpty();
    void testDescribePlanetClimate();
    void testDescribePlanetClimateFormat();
    void testDescribePlanetClimateEmpty();
    void testDescribePlanetClimateDifferent();
    void testDescribePlanetClimateDeath();
    void testDescribePlanetClimateUnowned();
    void testDescribePlanetNatives();
    void testDescribePlanetNativesEmpty();
    void testDescribePlanetNativesAged();
    void testDescribePlanetNativesUnowned();
    void testDescribePlanetNativesUnownedBorg();
    void testDescribePlanetColony();
    void testDescribePlanetColonyEmpty();
    void testDescribePlanetColonyRGA();
    void testDescribePlanetColonyGroundAttack();
    void testDescribePlanetColonyAged();
    void testDescribePlanetBuildingEffects();
    void testDescribePlanetBuildingEffectsEmpty();
    void testDescribePlanetDefenseEffects();
    void testPrepareUnloadInfo();
    void testPackGroundDefenseInfo();
};

class TestGameMapPlanetPredictor : public CxxTest::TestSuite {
 public:
    void testPHost();
    void testHost();
    void testGrowthPHostTholian();
    void testGrowthHostTholian();
};

class TestGameMapPlanetStorage : public CxxTest::TestSuite {
 public:
    void testPlanet();
};

class TestGameMapPoint : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testModify();
    void testOperators();
    void testParse();
    void testParseFail();
    void testCompare();
    void testDistance();
};

class TestGameMapRenderList : public CxxTest::TestSuite {
 public:
    void testRead();
    void testReadInsnOnly();
    void testReplay();
    void testReplayAgain();
};

class TestGameMapReverter : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameMapSelections : public CxxTest::TestSuite {
 public:
    void testInit();
    void testCopy();
    void testExecute();
    void testSetLayer();
    void testCurrent();
    void testExecuteAll();
    void testExecuteAllShip();
};

class TestGameMapSelectionVector : public CxxTest::TestSuite {
 public:
    void testInit();
    void testSetGet();
    void testCopy();
    void testExecute();
    void testExecuteSize();
    void testExecuteOp();
    void testExecuteError();
};

class TestGameMapShip : public CxxTest::TestSuite {
 public:
    void testInit();
};

class TestGameMapShipData : public CxxTest::TestSuite {
 public:
    void testGetShipMassEmpty();
    void testGetShipMassFreighter();
    void testGetShipMassCapital();
    void testGetShipMassNoHull();
    void testGetShipMassNoBeam();
    void testGetShipMassNoLauncher();
    void testIsTransferActiveEmpty();
    void testIsTransferActiveFull();
    void testIsTransferActivePart();
};

class TestGameMapShipPredictor : public CxxTest::TestSuite {
 public:
    void testErrorCases();
    void testFuelUsageHost();
    void testFuelUsagePHost();
    void testAlchemy();
    void testRefinery();
    void testAriesRefinery();
    void testMovement();
    void testMovement2();
    void testDamage();
    void testTorpedoes();
};

class TestGameMapShipStorage : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapShipTransporter : public CxxTest::TestSuite {
 public:
    void testNames();
};

class TestGameMapUfo : public CxxTest::TestSuite {
 public:
    void testAccessor();
    void testConnect();
    void testMovementPrediction();
};

class TestGameMapUfoType : public CxxTest::TestSuite {
 public:
    void testLoadUfo();
    void testLoadWormhole();
    void testLoadBoth();
    void testLoadHistory();
    void testMovementGuessing();
    void testMovementGuessing2();
    void testMovementGuessing3();
    void testIteration();
};

class TestGameMapViewport : public CxxTest::TestSuite {
 public:
    void testRectangle();
};

#endif
