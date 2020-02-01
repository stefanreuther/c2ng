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

class TestGameMapBeamUpShipTransfer : public CxxTest::TestSuite {
 public:
    void testParse();
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
};

class TestGameMapExplosion : public CxxTest::TestSuite {
 public:
    void testName();
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
};

class TestGameMapMapObject : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapMarkingVector : public CxxTest::TestSuite {
 public:
    void testInit();
    void testSetGet();
    void testCopy();
    void testExecute();
    void testExecuteSize();
    void testExecuteOp();
    void testExecuteError();
};

class TestGameMapMarkings : public CxxTest::TestSuite {
 public:
    void testInit();
    void testCopy();
    void testExecute();
    void testSetLayer();
};

class TestGameMapMinefield : public CxxTest::TestSuite {
 public:
    void testUnitsAfterDecayHost();
    void testUnitsAfterDecayPHost();
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
};

class TestGameMapPlanetInfo : public CxxTest::TestSuite {
 public:
    void testPackPlanetMineralInfo();
    void testPackPlanetMineralInfoMineOverride();
    void testPackPlanetMineralInfoEmpty();
    void testDescribePlanetClimate();
    void testDescribePlanetClimateEmpty();
    void testDescribePlanetClimateDifferent();
    void testDescribePlanetNatives();
    void testDescribePlanetNativesEmpty();
    void testDescribePlanetColony();
    void testDescribePlanetColonyEmpty();
    void testDescribePlanetColonyRGA();
    void testDescribePlanetBuildingEffects();
    void testDescribePlanetBuildingEffectsEmpty();
    void testDescribePlanetDefenseEffects();
    void testPackGroundDefenseInfo();
};

class TestGameMapPlanetPredictor : public CxxTest::TestSuite {
 public:
    void testPHost();
    void testHost();
    void testGrowthPHostTholian();
    void testGrowthHostTholian();
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

class TestGameMapShip : public CxxTest::TestSuite {
 public:
    void testInit();
};

class TestGameMapShipPredictor : public CxxTest::TestSuite {
 public:
    void testFuelUsageHost();
    void testFuelUsagePHost();
};

class TestGameMapShipStorage : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapViewport : public CxxTest::TestSuite {
 public:
    void testRectangle();
};

#endif
