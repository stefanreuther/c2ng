/**
  *  \file u/t_game_sim_classresult.cpp
  *  \brief Test for game::sim::ClassResult
  */

#include "game/sim/classresult.hpp"

#include "t_game_sim.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/planet.hpp"
#include "afl/base/staticassert.hpp"
#include "game/types.hpp"
#include "game/vcr/classic/database.hpp"

static_assert(game::MAX_PLAYERS < 99, "MAX_PLAYERS");

void
TestGameSimClassResult::testIt()
{
    // Environment
    game::sim::Setup setup;
    setup.addShip()->setOwner(4);
    setup.addShip()->setOwner(2);
    setup.addShip()->setOwner(99);
    setup.addShip()->setOwner(0);
    setup.addPlanet()->setOwner(4);

    game::sim::Result res;
    res.battles = new game::vcr::classic::Database();

    // Constructor
    game::sim::ClassResult testee(setup, res);

    // Verify
    TS_ASSERT_EQUALS(testee.getClass().get(2), 1);
    TS_ASSERT_EQUALS(testee.getClass().get(4), 2);
    TS_ASSERT_EQUALS(testee.getClass().get(99), 0);     // out-of-range, not counted
    TS_ASSERT_EQUALS(testee.getClass().get(0), 1);

    TS_ASSERT_EQUALS(testee.getSampleBattle(), res.battles);
    TS_ASSERT_EQUALS(testee.getWeight(), 1);

    TS_ASSERT_EQUALS(testee.isSameClass(testee), true);

    // Change weight
    testee.changeWeight(4, 8);
    TS_ASSERT_EQUALS(testee.getWeight(), 2);
}

void
TestGameSimClassResult::testMulti()
{
    // Some setups
    game::sim::Setup setup1;
    setup1.addShip()->setOwner(4);
    setup1.addShip()->setOwner(2);
    setup1.addPlanet()->setOwner(4);

    game::sim::Setup setup2;
    setup2.addShip()->setOwner(2);
    setup2.addShip()->setOwner(4);
    setup2.addPlanet()->setOwner(4);

    game::sim::Setup setup3;
    setup3.addShip()->setOwner(3);
    setup3.addShip()->setOwner(2);
    setup3.addPlanet()->setOwner(4);

    // Results
    game::sim::Result res1; res1.battles = new game::vcr::classic::Database();
    game::sim::Result res2; res2.battles = new game::vcr::classic::Database();
    game::sim::Result res3; res3.battles = new game::vcr::classic::Database();

    // ClassResults
    game::sim::ClassResult cr1(setup1, res1);
    game::sim::ClassResult cr2(setup2, res2);
    game::sim::ClassResult cr3(setup3, res3);

    // Verify compatibility
    TS_ASSERT_EQUALS(cr1.isSameClass(cr1), true);
    TS_ASSERT_EQUALS(cr1.isSameClass(cr2), true);
    TS_ASSERT_EQUALS(cr1.isSameClass(cr3), false);

    TS_ASSERT_EQUALS(cr2.isSameClass(cr1), true);
    TS_ASSERT_EQUALS(cr2.isSameClass(cr2), true);
    TS_ASSERT_EQUALS(cr2.isSameClass(cr3), false);

    TS_ASSERT_EQUALS(cr3.isSameClass(cr1), false);
    TS_ASSERT_EQUALS(cr3.isSameClass(cr2), false);
    TS_ASSERT_EQUALS(cr3.isSameClass(cr3), true);

    // Add
    TS_ASSERT_EQUALS(cr1.getWeight(), 1);
    TS_ASSERT_EQUALS(cr2.getWeight(), 1);
    cr1.addSameClassResult(cr2);

    TS_ASSERT_EQUALS(cr1.getWeight(), 2);
    TS_ASSERT_EQUALS(cr2.getWeight(), 1);
    TS_ASSERT_EQUALS(cr1.getSampleBattle(), res2.battles);
}

