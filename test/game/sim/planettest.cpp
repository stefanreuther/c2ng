/**
  *  \file test/game/sim/planettest.cpp
  *  \brief Test for game::sim::Planet
  */

#include "game/sim/planet.hpp"

#include "afl/test/testrunner.hpp"
#include "game/sim/configuration.hpp"

#include "objecttest.hpp"

/** Test getter/setter. */
AFL_TEST("game.sim.Planet:basics", a)
{
    game::sim::Planet t;

    // Initial state
    a.checkEqual("01. getDefense", t.getDefense(), 10);
    a.checkEqual("02. getBaseDefense", t.getBaseDefense(), 10);
    a.checkEqual("03. getBaseBeamTech", t.getBaseBeamTech(), 0);
    a.checkEqual("04. getBaseTorpedoTech", t.getBaseTorpedoTech(), 1);
    a.checkEqual("05. getNumBaseFighters", t.getNumBaseFighters(), 0);
    a.checkEqual("06. getNumBaseTorpedoes", t.getNumBaseTorpedoes(-1), 0); // out of range
    a.checkEqual("07. getNumBaseTorpedoes", t.getNumBaseTorpedoes(0), 0);  // out of range
    a.checkEqual("08. getNumBaseTorpedoes", t.getNumBaseTorpedoes(1), 0);  // in range
    a.checkEqual("09. getNumBaseTorpedoes", t.getNumBaseTorpedoes(10), 0); // in range
    a.checkEqual("10. getNumBaseTorpedoes", t.getNumBaseTorpedoes(11), 0); // out of range
    a.check("11. hasBase", !t.hasBase());

    // Get/Set
    t.markClean();
    t.setDefense(61);
    a.checkEqual("21. getDefense", t.getDefense(), 61);
    a.check("22. isDirty", t.isDirty());

    t.markClean();
    t.setBaseDefense(50);
    a.checkEqual("31. getBaseDefense", t.getBaseDefense(), 50);
    a.check("32. isDirty", t.isDirty());

    t.markClean();                             // repeated -> no change signal
    t.setBaseDefense(50);
    a.checkEqual("41. getBaseDefense", t.getBaseDefense(), 50);
    a.check("42. isDirty", !t.isDirty());

    t.markClean();
    t.setBaseBeamTech(9);
    a.checkEqual("51. getBaseBeamTech", t.getBaseBeamTech(), 9);
    a.check("52. hasBase", t.hasBase());
    a.check("53. isDirty", t.isDirty());

    t.markClean();
    t.setBaseTorpedoTech(4);
    a.checkEqual("61. getBaseTorpedoTech", t.getBaseTorpedoTech(), 4);
    a.check("62. isDirty", t.isDirty());

    t.markClean();
    t.setNumBaseFighters(40);
    a.checkEqual("71. getNumBaseFighters", t.getNumBaseFighters(), 40);
    a.check("72. isDirty", t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(-1, 10);
    a.checkEqual("81", t.getNumBaseTorpedoes(-1), 0); // out of range
    a.check("82. isDirty", !t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(0, 10);
    a.checkEqual("91", t.getNumBaseTorpedoes(0), 0); // out of range
    a.check("92. isDirty", !t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(1, 10);
    a.checkEqual("101", t.getNumBaseTorpedoes(1), 10); // in range
    a.check("102. isDirty", t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(10, 3);
    a.checkEqual("111", t.getNumBaseTorpedoes(10), 3); // in range
    a.check("112. isDirty", t.isDirty());

    t.markClean();
    t.setNumBaseTorpedoes(11, 9);
    a.checkEqual("121", t.getNumBaseTorpedoes(11), 0); // out of range
    a.check("122. isDirty", !t.isDirty());

    verifyObject(a, t);
}

/** Test hasAbility(). */
AFL_TEST("game.sim.Planet:hasAbility", a)
{
    game::config::HostConfiguration config;
    game::spec::ShipList shipList;
    game::sim::Planet t;
    game::sim::Configuration opts;

    // Lizards don't...
    t.setOwner(2);
    a.check("01. Lizard", !t.hasAbility(game::sim::TripleBeamKillAbility, opts, shipList, config));

    // ...but Pirates do have this ability.
    t.setOwner(5);
    a.check("11. Pirate", t.hasAbility(game::sim::TripleBeamKillAbility, opts, shipList, config));
}

/** Test getNumBaseTorpedoesAsType(). */
AFL_TEST("game.sim.Planet:getNumBaseTorpedoesAsType", a)
{
    // Make a ship list
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::sim::Planet::NUM_TORPEDO_TYPES; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(4, 4);
    testee.setNumBaseTorpedoes(10, 1);
    // total cost: 126

    a.checkEqual("01", testee.getNumBaseTorpedoesAsType(1, shipList), 126);
    a.checkEqual("02", testee.getNumBaseTorpedoesAsType(2, shipList),  63);
    a.checkEqual("03", testee.getNumBaseTorpedoesAsType(3, shipList),  42);
    a.checkEqual("04", testee.getNumBaseTorpedoesAsType(10, shipList), 12);
}

/** Test getNumBaseTorpedoesAsType, zero cost. */
AFL_TEST("game.sim.Planet:getNumBaseTorpedoesAsType:zero-cost", a)
{
    // Make a ship list
    game::spec::ShipList shipList;
    for (int i = 1; i <= game::sim::Planet::NUM_TORPEDO_TYPES; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }
    shipList.launchers().get(3)->cost().set(game::spec::Cost::Money, 0);

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(3, 10);
    // total cost: 100

    a.checkEqual("01", testee.getNumBaseTorpedoesAsType(1, shipList), 100);
    a.checkEqual("02", testee.getNumBaseTorpedoesAsType(2, shipList),  50);
    a.checkEqual("03", testee.getNumBaseTorpedoesAsType(3, shipList), 100);
}

/** Test getNumBaseTorpedoesAsType, partial ship list. */
AFL_TEST("game.sim.Planet:getNumBaseTorpedoesAsType:partial-ship-list", a)
{
    // Make a ship list with just 5 torpedo types
    game::spec::ShipList shipList;
    for (int i = 1; i <= 5; ++i) {
        game::spec::TorpedoLauncher* tl = shipList.launchers().create(i);
        tl->cost().set(game::spec::Cost::Money, i);
    }

    // Tester
    game::sim::Planet testee;
    testee.setBaseBeamTech(10);
    testee.setNumBaseTorpedoes(1, 100);
    testee.setNumBaseTorpedoes(4, 4);
    testee.setNumBaseTorpedoes(10, 1);
    // total cost: 116

    a.checkEqual("01", testee.getNumBaseTorpedoesAsType(1, shipList), 116);
    a.checkEqual("02", testee.getNumBaseTorpedoesAsType(2, shipList),  58);
    a.checkEqual("03", testee.getNumBaseTorpedoesAsType(3, shipList),  38);
    a.checkEqual("04", testee.getNumBaseTorpedoesAsType(10, shipList), 116);
}
