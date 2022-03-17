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

class TestGameMapBoundingBox : public CxxTest::TestSuite {
 public:
    void testInit();
    void testAddPoint();
    void testAddCircle();
    void testAddDrawing();
    void testAddUniverse();
    void testAddWrappedUfo();
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
    void testFlatImage();
    void testFlatSmall();
    void testFlatOffset();
    void testFlatOffsetImage();
    void testWrapped();
    void testWrappedImage();
    void testWrappedSmall();
    void testCircular();
    void testCircularImage();
    void testInitFromConfig();
    void testInitFromConfigWrap();
    void testInitFromBadConfig();
    void testSaveToConfig();
    void testSaveToConfigWrap();
    void testSaveToConfigFull();
    void testSaveToConfigUser();
};

class TestGameMapDrawing : public CxxTest::TestSuite {
 public:
    void testInit();
    void testDistance();
    void testDistanceWrap();
};

class TestGameMapDrawingContainer : public CxxTest::TestSuite {
 public:
    void testIt();
    void testErase();
    void testEraseExpired();
    void testFindNearest();
    void testEraseAdjacent();
    void testColorAdjacent();
    void testTagAdjacent();
    void testFindMarker();
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
    void testForecastEmpty();
    void testForecastNormal();
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
    void testWarpWell();
};

class TestGameMapMessageLink : public CxxTest::TestSuite {
 public:
    void testIt();
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
    void testBrowse();
    void testBrowseUnmarked();
    void testBrowseHere();
};

class TestGameMapObjectCursorFactory : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameMapObjectObserver : public CxxTest::TestSuite {
 public:
    void testNull();
    void testNormal();
};

class TestGameMapObjectType : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testUnit();
    void testSparseEmpty();
    void testSparseUnit();
    void testNormal();
    void testPartial();
    void testFindNearest();
};

class TestGameMapPlanet : public CxxTest::TestSuite {
 public:
    void testAutobuildSettings();
    void testCopy();
};

class TestGameMapPlanetEffectors : public CxxTest::TestSuite {
 public:
    void testIt();
    void testDescribe();
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
    void testPreparePlanetEffectors();
};

class TestGameMapPlanetPredictor : public CxxTest::TestSuite {
 public:
    void testPHost();
    void testHost();
    void testGrowthPHostTholian();
    void testGrowthHostTholian();
    void testGrowthMaxHost();
    void testGrowthRebelHost();
    void testGrowthHumanoidHost();
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

class TestGameMapRangeSet : public CxxTest::TestSuite {
 public:
    void testInit();
    void testAdd();
    void testAddConcentric();
    void testAddMultiple();
    void testAddObjectType();
    void testClear();
};

class TestGameMapRenderList : public CxxTest::TestSuite {
 public:
    void testRead();
    void testReadInsnOnly();
    void testReplay();
    void testReplayAgain();
};

class TestGameMapRenderOptions : public CxxTest::TestSuite {
 public:
    void testSet();
    void testTransfer();
    void testTranslation();
    void testCopy();
    void testKey();
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
    void testMarkListCurrent();
    void testMarkListOther();
    void testSetRelative();
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
    void testGetOptimumWarp();
    void testGetOptimumWarpErrorCases();
};

class TestGameMapShipStorage : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapShipTransporter : public CxxTest::TestSuite {
 public:
    void testNames();
};

class TestGameMapTypedObjectType : public CxxTest::TestSuite {
 public:
    void testInterface();
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

class TestGameMapUniverse : public CxxTest::TestSuite {
 public:
    void testBasics();
    void testGetObject();
    void testFind();
};

class TestGameMapViewport : public CxxTest::TestSuite {
 public:
    void testRectangle();
};

#endif
