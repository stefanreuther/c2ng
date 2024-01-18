/**
  *  \file test/game/sim/classresulttest.cpp
  *  \brief Test for game::sim::ClassResult
  */

#include "game/sim/classresult.hpp"

#include "afl/base/staticassert.hpp"
#include "afl/test/testrunner.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/types.hpp"
#include "game/vcr/classic/database.hpp"

static_assert(game::MAX_PLAYERS < 99, "MAX_PLAYERS");

AFL_TEST("game.sim.ClassResult:basics", a)
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
    a.checkEqual("01. getClass", testee.getClass().get(2), 1);
    a.checkEqual("02. getClass", testee.getClass().get(4), 2);
    a.checkEqual("03. getClass", testee.getClass().get(99), 0);     // out-of-range, not counted
    a.checkEqual("04. getClass", testee.getClass().get(0), 1);

    a.checkEqual("11. getSampleBattle", testee.getSampleBattle().get(), res.battles.get());
    a.checkEqual("12. getWeight", testee.getWeight(), 1);

    a.checkEqual("21. isSameClass", testee.isSameClass(testee), true);

    // Change weight
    testee.changeWeight(4, 8);
    a.checkEqual("31. getWeight", testee.getWeight(), 2);
}

AFL_TEST("game.sim.ClassResult:multiple", a)
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
    a.checkEqual("01. isSameClass", cr1.isSameClass(cr1), true);
    a.checkEqual("02. isSameClass", cr1.isSameClass(cr2), true);
    a.checkEqual("03. isSameClass", cr1.isSameClass(cr3), false);

    a.checkEqual("11. isSameClass", cr2.isSameClass(cr1), true);
    a.checkEqual("12. isSameClass", cr2.isSameClass(cr2), true);
    a.checkEqual("13. isSameClass", cr2.isSameClass(cr3), false);

    a.checkEqual("21. isSameClass", cr3.isSameClass(cr1), false);
    a.checkEqual("22. isSameClass", cr3.isSameClass(cr2), false);
    a.checkEqual("23. isSameClass", cr3.isSameClass(cr3), true);

    // Add
    a.checkEqual("31. getWeight", cr1.getWeight(), 1);
    a.checkEqual("32. getWeight", cr2.getWeight(), 1);
    cr1.addSameClassResult(cr2);

    a.checkEqual("41. getWeight", cr1.getWeight(), 2);
    a.checkEqual("42. getWeight", cr2.getWeight(), 1);
    a.checkEqual("43. getSampleBattle", cr1.getSampleBattle().get(), res2.battles.get());
}
