/**
  *  \file u/t_game_sim.hpp
  *  \brief Tests for game::sim
  */
#ifndef C2NG_U_T_GAME_SIM_HPP
#define C2NG_U_T_GAME_SIM_HPP

#include <cxxtest/TestSuite.h>
#include "game/sim/object.hpp"

class TestGameSimAbility : public CxxTest::TestSuite {
 public:
    void testIt();
    void testToString();
};

class TestGameSimClassResult : public CxxTest::TestSuite {
 public:
    void testIt();
    void testMulti();
};

class TestGameSimConfiguration : public CxxTest::TestSuite {
 public:
    void testIt();
    void testConfig();
    void testToString();
    void testCopyFrom();
    void testGetNext();
};

class TestGameSimFleetCost : public CxxTest::TestSuite {
 public:
    void testEmpty();
    void testNormal();
    void testTechCost();
    void testEnums();
};

class TestGameSimGameInterface : public CxxTest::TestSuite {
 public:
    void testInterface();
};

class TestGameSimLoader : public CxxTest::TestSuite {
 public:
    void testV0();
    void testV1();
    void testV2();
    void testV3();
    void testV4();
    void testV5();
    void testError();
    void testSaveDefault();
    void testSaveRating();
    void testSaveIntercept();
    void testSaveFlags();
};

class TestGameSimObject : public CxxTest::TestSuite {
 public:
    void testIt();
    void testSetRandom();
    void testRandom();
    void testCopy();

    static void verifyObject(game::sim::Object& t);
};

class TestGameSimPlanet : public CxxTest::TestSuite {
 public:
    void testIt();
    void testAbility();
    void testCost();
    void testCostZero();
    void testCostPartial();
};

class TestGameSimResult : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSimResultList : public CxxTest::TestSuite {
 public:
    void testIt();
    void testIncrease();
    void testDecrease();
    void testMultipleClasses();
    void testDescribe();
    void testDescribe2();
    void testToString();
};

class TestGameSimRun : public CxxTest::TestSuite {
 public:
    void testHost();
    void testHostBig();
    void testHostNoTorps();
    void testHostBalance();
    void testHostMaster();
    void testHostPlanet();
    void testHostIntercept();
    void testHostMulti();
    void testHostESB();
    void testPHost();
    void testPHostBig();
    void testPHostPlanet();
    void testPHostPlanetTubes();
    void testPHostIntercept();
    void testPHostMulti();
    void testShipCommander();
    void testShipDeactivated();
    void testShipAllied();
    void testShipPassive();
    void testShipNotEnemy();
    void testShipEnemy();
    void testShipPersistentEnemy();
    void testShipCloaked();
    void testShipFriendlyCodeMatch();
    void testShipNoFuel();
    void testShipCloakedFighterBays();
    void testShipCloakedFighterBaysNT();
    void testShipSquadron();
    void testPlanetDeactivated();
    void testPlanetCloaked();
    void testPlanetFriendlyCodeMatch();
    void testPlanetAllied();
    void testPlanetNotAggressive();
    void testPlanetNotEnemy();
    void testPlanetImmuneRace();
    void testPlanetBird();
    void testPlanetPrimaryEnemy();
    void testPlanetNuk();
    void testFLAK();
    void testFLAKESB();
    void testFLAKMulti();
    void testOrderHostShip();
    void testOrderHostPlanet();
    void testOrderPHostShip();
    void testOrderPHostPlanet();
    void testShieldGenerator();
};

class TestGameSimRunner : public CxxTest::TestSuite {
 public:
    void testRegression1();
    void testRegression2();
    void testInterrupt();
};

class TestGameSimSession : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSimSessionExtra : public CxxTest::TestSuite {
 public:
    void testIt();
    void testAlliances();
};

class TestGameSimSetup : public CxxTest::TestSuite {
 public:
    void testObj();
    void testShip();
    void testShipList();
    void testRandom();
    void testListener();
    void testMerge();
    void testFindUnused();
    void testReplicate();
    void testCopy();
    void testSetSequential();
    void testSort();
    void testAddData();
    void testSetFlags();
    void testGetInvolved();
};

class TestGameSimShip : public CxxTest::TestSuite {
 public:
    void testIt();
    void testName();
    void testShipList();
    void testAbilities();
    void testAggressive();
};

class TestGameSimSort : public CxxTest::TestSuite {
 public:
    void testIt();
};

class TestGameSimStructures : public CxxTest::TestSuite {
 public:
    void testHeader();
};

class TestGameSimTransfer : public CxxTest::TestSuite {
 public:
    void testCopyFromEmptyShip();
    void testCopyFromShip();
    void testCopyToShip();
    void testCopyToMismatchingShip();
    void testCopyToShipWithFighters();
    void testCopyToShipWithTorps();
    void testCopyFromEmptyPlanet();
    void testCopyFromPlanet();
    void testCopyFromBase();
    void testCopyToPlanet();
    void testCopyToMismatchingPlanet();
    void testCopyShipFromBattle();
    void testCopyPlanetFromBattle();
};

class TestGameSimUnitResult : public CxxTest::TestSuite {
 public:
    void testShip();
    void testShip2();
    void testPlanet();
    void testMulti();
};

#endif
