/**
  *  \file u/t_game_map_info.hpp
  *  \brief Tests for game::map::info
  */
#ifndef C2NG_U_T_GAME_MAP_INFO_HPP
#define C2NG_U_T_GAME_MAP_INFO_HPP

#include <cxxtest/TestSuite.h>

class TestGameMapInfoBrowser : public CxxTest::TestSuite {
 public:
    void testNull();
    void testEmptyTotals();
    void testEmptyMinerals();
    void testEmptyPlanets();
    void testEmptyColony();
    void testEmptyStarbases();
    void testEmptyStarships();
    void testEmptyCapital();
    void testEmptyStarchart();
    void testEmptyWeapons();
    void testSampleTotals();
    void testSampleMinerals();
    void testSampleMineralsMinedT();
    void testSamplePlanets();
    void testSamplePlanetsExperience();
    void testSamplePlanetsByNativeRace();
    void testSampleColony();
    void testSampleColonyTopSupplies();
    void testSampleStarbases();
    void testSampleStarships();
    void testSampleStarshipsByHullName();
    void testSampleCapital();
    void testSampleStarchart();
    void testSampleWeapons();
    void testSampleWeaponsOnlyTorps();
    void testOptionsTotals();
    void testOptionsMinerals();
    void testOptionsMineralsMinedT();
    void testOptionsPlanets();
    void testOptionsColony();
    void testOptionsStarbases();
    void testOptionsStarbasesShipsByMass();
    void testOptionsStarships();
    void testOptionsStarshipsByMass();
    void testOptionsCapital();
    void testOptionsStarchart();
    void testOptionsWeapons();
};

class TestGameMapInfoInfo : public CxxTest::TestSuite {
 public:
    void testEmpireZeroSize();
    void testEmpireZeroUnit();
    void testEmpireZeroSameX();
    void testEmpireDifferent();
    void testEmpireWrap();
    void testEmpireWrap2();
    void testExperience();
    void testPlanetExperience();
};

class TestGameMapInfoLinkBuilder : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameMapInfoMission : public CxxTest::TestSuite {
 public:
    void testRenderChunnelFailureReasons();
    void testRenderShipPredictorUsedProperties();
    void testRenderShipPredictorUsedProperties2();
    void testRenderShipPredictorUsedProperties3();
};

class TestGameMapInfoNullLinkBuilder : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameMapInfoScriptLinkBuilder : public CxxTest::TestSuite {
 public:
    void testPlanet();
    void testSearch();
};

class TestGameMapInfoTypes : public CxxTest::TestSuite {
 public:
};

#endif
