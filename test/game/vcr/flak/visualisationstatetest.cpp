/**
  *  \file test/game/vcr/flak/visualisationstatetest.cpp
  *  \brief Test for game::vcr::flak::VisualisationState
  */

#include "game/vcr/flak/visualisationstate.hpp"
#include "afl/test/testrunner.hpp"

using game::vcr::flak::Position;
using game::vcr::flak::Visualizer;

namespace {
    Visualizer::ShipInfo makeShipInfo(int player, bool isPlanet)
    {
        Visualizer::ShipInfo info;
        info.player = player;
        info.isPlanet = isPlanet;
        return info;
    }
}

/** Test initialisation.
    A: create VisualisationState
    E: verify initial state, no objects present */
AFL_TEST("game.vcr.flak.VisualisationState:init", a)
{
    game::vcr::flak::VisualisationState testee;

    // Nothing to display
    a.check("01. objects", testee.objects().empty());
    a.check("02. ships", testee.ships().empty());
    a.check("03. fleets", testee.fleets().empty());
    a.check("04. smoke", testee.smoke().empty());
    a.check("05. beams", testee.beams().empty());
    a.checkEqual("06. getTime", testee.getTime(), 0);

    // No animations
    a.check("11. animate", !testee.animate());

    // Sensible arena size
    a.checkGreaterThan("21. getArenaSize", testee.getArenaSize(), 100);
    a.checkGreaterThan("22. getGridSize", testee.getGridSize(), 100);
}

/** Test bounds-check behaviour.
    A: create VisualisationState. Call modifier functions without previously calling creation functions.
    E: must not crash. */
AFL_TEST_NOARG("game.vcr.flak.VisualisationState:bounds")
{
    game::vcr::flak::VisualisationState testee;
    testee.fireBeamFighterFighter(100, 200, true);
    testee.fireBeamFighterShip(100, 200, true);
    testee.fireBeamShipFighter(100, 17, 200, true);
    testee.fireBeamShipShip(100, 17, 200, true);

    testee.killFighter(100);
    testee.landFighter(100);
    testee.moveFighter(100, Position(1,1,1), 200);

    testee.setEnemy(100, 200);
    testee.killFleet(100);
    testee.moveFleet(100, 1000, 2000);

    testee.killShip(100);
    testee.moveShip(100, Position(1,1,1));

    testee.hitTorpedo(100, 200);
    testee.missTorpedo(100);
    testee.moveTorpedo(100, Position(1,1,1));
}

/** Test ship functions.
    A: create and verify ships
    E: verify correct state */
AFL_TEST("game.vcr.flak.VisualisationState:ship", a)
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships and fleets
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);

    // Verify
    a.checkNonNull("01. ships",  testee.ships().at(1));
    a.checkNonNull("02. ships",  testee.ships().at(2));
    a.checkEqual("03. player",   testee.ships().at(1)->player, 3);
    a.checkEqual("04. pos x",    testee.ships().at(1)->pos.x, 1000);
    a.checkEqual("05. pos y",    testee.ships().at(1)->pos.y, 2000);
    a.checkEqual("06. pos z",    testee.ships().at(1)->pos.z, 0);
    a.checkEqual("07. isPlanet", testee.ships().at(1)->isPlanet, false);

    a.checkEqual("11. player",   testee.ships().at(2)->player, 4);
    a.checkEqual("12. pos x",    testee.ships().at(2)->pos.x, 1000);
    a.checkEqual("13. pos y",    testee.ships().at(2)->pos.y, -5000);
    a.checkEqual("14. pos z",    testee.ships().at(2)->pos.z, 250);        // ZSCALE has been applied
    a.checkEqual("15. isPlanet", testee.ships().at(2)->isPlanet, true);

    a.checkGreaterEqual("21. getArenaSize", testee.getArenaSize(), 5000);
    a.checkLessEqual   ("22. getArenaSize", testee.getArenaSize(), 10000);

    a.checkEqual("31. getGridSize", testee.getGridSize(), 5000);

    // Move and verify
    testee.moveShip(1, Position(1000, 1800, 0));
    a.checkEqual("41. pos x", testee.ships().at(1)->pos.x, 1000);
    a.checkEqual("42. pos y", testee.ships().at(1)->pos.y, 1800);
    a.checkEqual("43. pos z", testee.ships().at(1)->pos.z, 0);

    // Kill and verify
    testee.killShip(1);
    a.checkEqual("51. isAlive", testee.ships().at(1)->isAlive, false);
    a.check("52. smoke", !testee.smoke().empty());
}

/** Test fleet functions.
    A: create and verify fleets
    E: verify correct state */
AFL_TEST("game.vcr.flak.VisualisationState:fleet", a)
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships and fleets
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);

    // Verify
    a.checkNonNull("01. fleets", testee.fleets().at(0));
    a.checkNonNull("02. fleets", testee.fleets().at(1));

    a.checkEqual("11. player",    testee.fleets().at(0)->player, 3);
    a.checkEqual("12. firstShip", testee.fleets().at(0)->firstShip, 1U);
    a.checkEqual("13. numShips",  testee.fleets().at(0)->numShips, 1U);
    a.checkEqual("14. isAlive",   testee.fleets().at(0)->isAlive, true);
    a.checkEqual("15. x",         testee.fleets().at(0)->x, 1000);
    a.checkEqual("16. y",         testee.fleets().at(0)->y, 2000);

    a.checkEqual("21. player",    testee.fleets().at(1)->player, 4);
    a.checkEqual("22. firstShip", testee.fleets().at(1)->firstShip, 2U);
    a.checkEqual("23. numShips",  testee.fleets().at(1)->numShips, 1U);
    a.checkEqual("24. isAlive",   testee.fleets().at(1)->isAlive, true);
    a.checkEqual("25. x",         testee.fleets().at(1)->x, 1000);
    a.checkEqual("26. y",         testee.fleets().at(1)->y, -5000);

    a.checkGreaterEqual("31. getArenaSize", testee.getArenaSize(), 5000);
    a.checkLessEqual   ("32. getArenaSize", testee.getArenaSize(), 10000);

    // Set enemy and verify
    testee.setEnemy(0, 2);
    a.checkEqual("41. enemy", testee.fleets().at(0)->enemy, 2U);
    a.checkEqual("42. enemy", testee.ships().at(1)->enemy, 2U);

    // Move and verify
    testee.moveFleet(1, 1000, -4000);
    a.checkEqual("51. x", testee.fleets().at(1)->x, 1000);
    a.checkEqual("52. y", testee.fleets().at(1)->y, -4000);

    // Kill and verify
    testee.killFleet(0);
    a.checkEqual("61. isAlive", testee.fleets().at(0)->isAlive, false);
}

/** Test fighter functions.
    A: create and verify fighters
    E: verify correct state */
AFL_TEST("game.vcr.flak.VisualisationState:fighter", a)
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));

    // Add fighters
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Verify
    a.checkNonNull("01. objects", testee.objects().at(30));
    a.checkNonNull("02. objects", testee.objects().at(40));
    a.checkEqual("03. type",      testee.objects().at(30)->type, game::vcr::flak::VisualisationState::FighterObject);
    a.checkEqual("04. pos x",     testee.objects().at(30)->pos.x, 500);
    a.checkEqual("05. pos y",     testee.objects().at(30)->pos.y, 600);
    a.checkEqual("06. pos z",     testee.objects().at(30)->pos.z, 2500);   // ZSCALE has been applied
    a.checkEqual("07. player",    testee.objects().at(30)->player, 5);
    a.checkNear("08. heading",    testee.objects().at(30)->heading, -1.4817, 0.0001);

    a.checkEqual("11. type",   testee.objects().at(40)->type, game::vcr::flak::VisualisationState::FighterObject);
    a.checkEqual("12. pos x",  testee.objects().at(40)->pos.x, -500);
    a.checkEqual("13. pos y",  testee.objects().at(40)->pos.y, 600);
    a.checkEqual("14. pos z",  testee.objects().at(40)->pos.z, 2500);   // ZSCALE has been applied
    a.checkEqual("15. player", testee.objects().at(40)->player, 4);
    a.checkNear("16. heading", testee.objects().at(40)->heading, -1.3090, 0.0001);

    // Intermediate object
    a.checkNonNull("21. objects", testee.objects().at(35));
    a.checkEqual("22. type", testee.objects().at(35)->type, game::vcr::flak::VisualisationState::NoObject);

    // Move and verify
    testee.moveFighter(40, Position(-400, 500, 80), 2);
    a.checkEqual("31. pos x", testee.objects().at(40)->pos.x, -400);
    a.checkEqual("32. pos y", testee.objects().at(40)->pos.y, 500);
    a.checkEqual("33. pos z", testee.objects().at(40)->pos.z, 2000);   // ZSCALE has been applied
    a.checkNear("34. heading", testee.objects().at(40)->heading, -1.3215, 0.0001);

    // Land and verify
    testee.landFighter(40);
    a.checkEqual("41. type", testee.objects().at(40)->type, game::vcr::flak::VisualisationState::NoObject);
    a.check("42. smoke", testee.smoke().empty());

    // Kill and verify
    testee.killFighter(30);
    a.checkEqual("51. type", testee.objects().at(30)->type, game::vcr::flak::VisualisationState::NoObject);
    a.check("52. smoke", !testee.smoke().empty());
}

/** Test torpedo functions.
    A: create and verify torpedoes
    E: verify correct state */
AFL_TEST("game.vcr.flak.VisualisationState:torpedo", a)
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));

    // Add torpedoes
    testee.createTorpedo(30, Position(500, 600, 100), 5, 2);
    testee.createTorpedo(40, Position(-500, 600, 100), 4, 2);

    // Verify
    a.checkNonNull("01. objects", testee.objects().at(30));
    a.checkNonNull("02. objects", testee.objects().at(40));
    a.checkEqual("03. type",      testee.objects().at(30)->type, game::vcr::flak::VisualisationState::TorpedoObject);
    a.checkEqual("04. pos x",     testee.objects().at(30)->pos.x, 500);
    a.checkEqual("05. pos y",     testee.objects().at(30)->pos.y, 600);
    a.checkEqual("06. pos z",     testee.objects().at(30)->pos.z, 2500);   // ZSCALE has been applied
    a.checkEqual("07. player",    testee.objects().at(30)->player, 5);

    a.checkEqual("11. type",   testee.objects().at(40)->type, game::vcr::flak::VisualisationState::TorpedoObject);
    a.checkEqual("12. pos x",  testee.objects().at(40)->pos.x, -500);
    a.checkEqual("13. pos y",  testee.objects().at(40)->pos.y, 600);
    a.checkEqual("14. pos z",  testee.objects().at(40)->pos.z, 2500);   // ZSCALE has been applied
    a.checkEqual("15. player", testee.objects().at(40)->player, 4);

    // Intermediate object
    a.checkNonNull("21. objects", testee.objects().at(35));
    a.checkEqual("22. type", testee.objects().at(35)->type, game::vcr::flak::VisualisationState::NoObject);

    // Move and verify
    testee.moveTorpedo(40, Position(-400, 500, 80));
    a.checkEqual("31. pos x", testee.objects().at(40)->pos.x, -400);
    a.checkEqual("32. pos y", testee.objects().at(40)->pos.y, 500);
    a.checkEqual("33. pos z", testee.objects().at(40)->pos.z, 2000);   // ZSCALE has been applied

    // Miss and verify
    testee.missTorpedo(40);
    a.checkEqual("41. type", testee.objects().at(40)->type, game::vcr::flak::VisualisationState::NoObject);
    a.check("42. smoke", testee.smoke().empty());

    // Hit and verify
    testee.hitTorpedo(30, 2);
    a.checkEqual("51. type", testee.objects().at(30)->type, game::vcr::flak::VisualisationState::NoObject);
    // For now, does not create smoke; killing the ship will.
}

/** Test beam functions.
    A: create and verify beams
    E: verify correct state */

// fireBeamFighterFighter
AFL_TEST("game.vcr.flak.VisualisationState:beam:fighter-fighter", a)
{
    // Add ships and fighters
    game::vcr::flak::VisualisationState testee;
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Fire beam
    testee.fireBeamFighterFighter(30, 40, true);

    // Verify
    a.checkNonNull("01. beams", testee.beams().at(0));
    a.checkEqual("02. from x",  testee.beams().at(0)->from.x, 500);
    a.checkEqual("03. from y",  testee.beams().at(0)->from.y, 600);
    a.checkEqual("04. from z",  testee.beams().at(0)->from.z, 2500);
    a.checkEqual("05. to x",    testee.beams().at(0)->to.x, -500);
    a.checkEqual("06. to y",    testee.beams().at(0)->to.y, 600);
    a.checkEqual("07. to z",    testee.beams().at(0)->to.z, 2500);
    a.checkEqual("08. age",     testee.beams().at(0)->age, 0);
}

// fireBeamFighterShip
AFL_TEST("game.vcr.flak.VisualisationState:beam:fighter-ship", a)
{
    // Add ships and fighters
    game::vcr::flak::VisualisationState testee;
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Fire beam
    testee.fireBeamFighterShip(30, 2, true);

    // Verify
    a.checkNonNull("01. beams", testee.beams().at(0));
    a.checkEqual("02. from x",  testee.beams().at(0)->from.x, 500);
    a.checkEqual("03. from y",  testee.beams().at(0)->from.y, 600);
    a.checkEqual("04. from z",  testee.beams().at(0)->from.z, 2500);
    a.checkEqual("05. to x",    testee.beams().at(0)->to.x, 1000);
    a.checkEqual("06. to y",    testee.beams().at(0)->to.y, -5000);
    a.checkEqual("07. to z",    testee.beams().at(0)->to.z, 250);
    a.checkEqual("08. age",     testee.beams().at(0)->age, 0);
}

// fireBeamShipFighter
AFL_TEST("game.vcr.flak.VisualisationState:beam:ship-fighter", a)
{
    // Add ships and fighters
    game::vcr::flak::VisualisationState testee;
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Fire beam
    testee.fireBeamShipFighter(1, 13, 40, true);

    // Verify
    a.checkNonNull("01. beams", testee.beams().at(0));
    a.checkEqual("02. from x",  testee.beams().at(0)->from.x, 1000);
    a.checkEqual("03. from y",  testee.beams().at(0)->from.y, 2000);
    a.checkEqual("04. from z",  testee.beams().at(0)->from.z, 0);
    a.checkEqual("05. to x",    testee.beams().at(0)->to.x, -500);
    a.checkEqual("06. to y",    testee.beams().at(0)->to.y, 600);
    a.checkEqual("07. to z",    testee.beams().at(0)->to.z, 2500);
    a.checkEqual("08. age",     testee.beams().at(0)->age, 0);
}

// fireBeamShipShip
AFL_TEST("game.vcr.flak.VisualisationState:beam:ship-ship", a)
{
    // Add ships and fighters
    game::vcr::flak::VisualisationState testee;
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Fire beam
    testee.fireBeamShipShip(1, 13, 2, true);

    // Verify
    a.checkNonNull("01. beams", testee.beams().at(0));
    a.checkEqual("02. from x",  testee.beams().at(0)->from.x, 1000);
    a.checkEqual("03. from y",  testee.beams().at(0)->from.y, 2000);
    a.checkEqual("04. from z",  testee.beams().at(0)->from.z, 0);
    a.checkEqual("05. to x",    testee.beams().at(0)->to.x, 1000);
    a.checkEqual("06. to y",    testee.beams().at(0)->to.y, -5000);
    a.checkEqual("07. to z",    testee.beams().at(0)->to.z, 250);
    a.checkEqual("08. age",     testee.beams().at(0)->age, 0);
}


/** Test aging of smoke.
    A: create smoke by killing a ship
    E: verify smoke is generated and disappears after configured time */
AFL_TEST("game.vcr.flak.VisualisationState:smoke", a)
{
    game::vcr::flak::VisualisationState testee;

    // Configure
    const int N = 20;
    testee.setMaxSmokeAge(N);

    // Add some ships and fleets; kill a ship
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);
    testee.killShip(1);

    // Verify: some smoke generated
    a.checkGreaterThan("01. smoke", testee.smoke().size(), 3U);

    // Animate
    int n = 0;
    while (testee.animate()) {
        ++n;
        a.check("11. smoke", !testee.smoke().empty());
        a.checkLessEqual("12. age", n, N);
    }
    a.checkEqual("13. age", n, N-1);
}

/** Test aging of beams.
    A: create beam
    E: verify beam is generated and disappears after configured time */
AFL_TEST("game.vcr.flak.VisualisationState:beam:aging", a)
{
    game::vcr::flak::VisualisationState testee;

    // Configure
    const int N = 20;
    testee.setMaxBeamAge(N);

    // Add some ships and fleets; fire a beam
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);
    testee.fireBeamShipShip(1, 17, 2, true);

    // Verify: beam generated
    a.checkEqual("01. beams", testee.beams().size(), 1U);

    // Animate
    int n = 0;
    while (testee.animate()) {
        ++n;
        a.check("11. beams", !testee.beams().empty());
        a.checkLessEqual("12. age", n, N);
    }
    a.checkEqual("13. age", n, N-1);
}

/** Test ship turning.
    A: create two ships; set enemy
    E: verify that heading is adjusted towards final angle */
AFL_TEST("game.vcr.flak.VisualisationState:ship:turn", a)
{
    game::vcr::flak::VisualisationState testee;

    // Add ships and fleets
    testee.createShip(1, Position(1000, 1000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -1000, 0), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -1000, 4, 2, 1);
    testee.setEnemy(0, 2);

    // Initial angle of ship 1 is south-west (-0.75*pi)
    a.checkNear("01. heading", testee.ships().at(1)->heading, -2.3561, 0.0001);

    // Animate once: angle moves
    testee.animate();
    a.checkGreaterThan("11. heading", testee.ships().at(1)->heading, -2.3562);

    // Animate: angle moves towards final value
    for (int i = 0; i < 100; ++i) {
        testee.animate();
    }

    // Final angle is south (-0.5*pi)
    a.checkNear("21. heading", testee.ships().at(1)->heading, -1.5707, 0.0001);
}

/** Test copying.
    A: create and populate a VisualisationState. Copy it.
    E: verify same content in both */
AFL_TEST("game.vcr.flak.VisualisationState:copy", a)
{
    game::vcr::flak::VisualisationState testee;
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);
    testee.fireBeamShipShip(1, 17, 2, true);

    game::vcr::flak::VisualisationState copy1;
    copy1 = testee;

    game::vcr::flak::VisualisationState copy2(testee);

    // Verify
    a.checkEqual("01. ship", testee.ships().at(2)->pos.y, -5000);
    a.checkEqual("02. ship", copy1.ships().at(2)->pos.y, -5000);
    a.checkEqual("03. ship", copy2.ships().at(2)->pos.y, -5000);

    a.checkEqual("11. beam", testee.beams().size(), 1U);
    a.checkEqual("12. beam", copy1.beams().size(), 1U);
    a.checkEqual("13. beam", copy2.beams().size(), 1U);
}

/** Test getTime().
    A: create VisualisationState. Call updateTime().
    E: verify result */
AFL_TEST("game.vcr.flak.VisualisationState:time", a)
{
    game::vcr::flak::VisualisationState testee;
    testee.updateTime(777);

    a.checkEqual("01. getTime", testee.getTime(), 777);
}
