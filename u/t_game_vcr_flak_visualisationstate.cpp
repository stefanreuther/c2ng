/**
  *  \file u/t_game_vcr_flak_visualisationstate.cpp
  *  \brief Test for game::vcr::flak::VisualisationState
  */

#include "game/vcr/flak/visualisationstate.hpp"

#include "t_game_vcr_flak.hpp"

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
void
TestGameVcrFlakVisualisationState::testInit()
{
    game::vcr::flak::VisualisationState testee;

    // Nothing to display
    TS_ASSERT(testee.objects().empty());
    TS_ASSERT(testee.ships().empty());
    TS_ASSERT(testee.fleets().empty());
    TS_ASSERT(testee.smoke().empty());
    TS_ASSERT(testee.beams().empty());
    TS_ASSERT_EQUALS(testee.getTime(), 0);

    // No animations
    TS_ASSERT(!testee.animate());

    // Sensible arena size
    TS_ASSERT(testee.getArenaSize() > 100);
    TS_ASSERT(testee.getGridSize() > 100);
}

/** Test bounds-check behaviour.
    A: create VisualisationState. Call modifier functions without previously calling creation functions.
    E: must not crash. */
void
TestGameVcrFlakVisualisationState::testBounds()
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
void
TestGameVcrFlakVisualisationState::testShip()
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships and fleets
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);

    // Verify
    TS_ASSERT(testee.ships().at(1) != 0);
    TS_ASSERT(testee.ships().at(2) != 0);
    TS_ASSERT_EQUALS(testee.ships().at(1)->player, 3);
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.x, 1000);
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.y, 2000);
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.z, 0);
    TS_ASSERT_EQUALS(testee.ships().at(1)->isPlanet, false);

    TS_ASSERT_EQUALS(testee.ships().at(2)->player, 4);
    TS_ASSERT_EQUALS(testee.ships().at(2)->pos.x, 1000);
    TS_ASSERT_EQUALS(testee.ships().at(2)->pos.y, -5000);
    TS_ASSERT_EQUALS(testee.ships().at(2)->pos.z, 250);        // ZSCALE has been applied
    TS_ASSERT_EQUALS(testee.ships().at(2)->isPlanet, true);

    TS_ASSERT(testee.getArenaSize() >= 5000);
    TS_ASSERT(testee.getArenaSize() <= 10000);

    TS_ASSERT_EQUALS(testee.getGridSize(), 5000);

    // Move and verify
    testee.moveShip(1, Position(1000, 1800, 0));
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.x, 1000);
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.y, 1800);
    TS_ASSERT_EQUALS(testee.ships().at(1)->pos.z, 0);

    // Kill and verify
    testee.killShip(1);
    TS_ASSERT_EQUALS(testee.ships().at(1)->isAlive, false);
    TS_ASSERT(!testee.smoke().empty());
}

/** Test fleet functions.
    A: create and verify fleets
    E: verify correct state */
void
TestGameVcrFlakVisualisationState::testFleet()
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships and fleets
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -5000, 4, 2, 1);

    // Verify
    TS_ASSERT(testee.fleets().at(0) != 0);
    TS_ASSERT(testee.fleets().at(1) != 0);

    TS_ASSERT_EQUALS(testee.fleets().at(0)->player, 3);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->firstShip, 1U);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->numShips, 1U);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->isAlive, true);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->x, 1000);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->y, 2000);

    TS_ASSERT_EQUALS(testee.fleets().at(1)->player, 4);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->firstShip, 2U);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->numShips, 1U);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->isAlive, true);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->x, 1000);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->y, -5000);

    TS_ASSERT(testee.getArenaSize() >= 5000);
    TS_ASSERT(testee.getArenaSize() <= 10000);

    // Set enemy and verify
    testee.setEnemy(0, 2);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->enemy, 2U);
    TS_ASSERT_EQUALS(testee.ships().at(1)->enemy, 2U);

    // Move and verify
    testee.moveFleet(1, 1000, -4000);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->x, 1000);
    TS_ASSERT_EQUALS(testee.fleets().at(1)->y, -4000);

    // Kill and verify
    testee.killFleet(0);
    TS_ASSERT_EQUALS(testee.fleets().at(0)->isAlive, false);
}

/** Test fighter functions.
    A: create and verify fighters
    E: verify correct state */
void
TestGameVcrFlakVisualisationState::testFighter()
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));

    // Add fighters
    testee.createFighter(30, Position(500, 600, 100), 5, 2);
    testee.createFighter(40, Position(-500, 600, 100), 4, 2);

    // Verify
    TS_ASSERT(testee.objects().at(30) != 0);
    TS_ASSERT(testee.objects().at(40) != 0);
    TS_ASSERT_EQUALS(testee.objects().at(30)->type, game::vcr::flak::VisualisationState::FighterObject);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.x, 500);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.y, 600);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.z, 2500);   // ZSCALE has been applied
    TS_ASSERT_EQUALS(testee.objects().at(30)->player, 5);
    TS_ASSERT_DELTA(testee.objects().at(30)->heading, -1.4817, 0.0001);

    TS_ASSERT_EQUALS(testee.objects().at(40)->type, game::vcr::flak::VisualisationState::FighterObject);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.x, -500);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.y, 600);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.z, 2500);   // ZSCALE has been applied
    TS_ASSERT_EQUALS(testee.objects().at(40)->player, 4);
    TS_ASSERT_DELTA(testee.objects().at(40)->heading, -1.3090, 0.0001);

    // Intermediate object
    TS_ASSERT(testee.objects().at(35) != 0);
    TS_ASSERT_EQUALS(testee.objects().at(35)->type, game::vcr::flak::VisualisationState::NoObject);

    // Move and verify
    testee.moveFighter(40, Position(-400, 500, 80), 2);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.x, -400);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.y, 500);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.z, 2000);   // ZSCALE has been applied
    TS_ASSERT_DELTA(testee.objects().at(40)->heading, -1.3215, 0.0001);

    // Land and verify
    testee.landFighter(40);
    TS_ASSERT_EQUALS(testee.objects().at(40)->type, game::vcr::flak::VisualisationState::NoObject);
    TS_ASSERT(testee.smoke().empty());

    // Kill and verify
    testee.killFighter(30);
    TS_ASSERT_EQUALS(testee.objects().at(30)->type, game::vcr::flak::VisualisationState::NoObject);
    TS_ASSERT(!testee.smoke().empty());
}

/** Test torpedo functions.
    A: create and verify torpedoes
    E: verify correct state */
void
TestGameVcrFlakVisualisationState::testTorpedo()
{
    game::vcr::flak::VisualisationState testee;

    // Add some ships
    testee.createShip(1, Position(1000, 2000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -5000, 10), makeShipInfo(4, true));

    // Add torpedoes
    testee.createTorpedo(30, Position(500, 600, 100), 5, 2);
    testee.createTorpedo(40, Position(-500, 600, 100), 4, 2);

    // Verify
    TS_ASSERT(testee.objects().at(30) != 0);
    TS_ASSERT(testee.objects().at(40) != 0);
    TS_ASSERT_EQUALS(testee.objects().at(30)->type, game::vcr::flak::VisualisationState::TorpedoObject);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.x, 500);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.y, 600);
    TS_ASSERT_EQUALS(testee.objects().at(30)->pos.z, 2500);   // ZSCALE has been applied
    TS_ASSERT_EQUALS(testee.objects().at(30)->player, 5);

    TS_ASSERT_EQUALS(testee.objects().at(40)->type, game::vcr::flak::VisualisationState::TorpedoObject);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.x, -500);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.y, 600);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.z, 2500);   // ZSCALE has been applied
    TS_ASSERT_EQUALS(testee.objects().at(40)->player, 4);

    // Intermediate object
    TS_ASSERT(testee.objects().at(35) != 0);
    TS_ASSERT_EQUALS(testee.objects().at(35)->type, game::vcr::flak::VisualisationState::NoObject);

    // Move and verify
    testee.moveTorpedo(40, Position(-400, 500, 80));
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.x, -400);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.y, 500);
    TS_ASSERT_EQUALS(testee.objects().at(40)->pos.z, 2000);   // ZSCALE has been applied

    // Miss and verify
    testee.missTorpedo(40);
    TS_ASSERT_EQUALS(testee.objects().at(40)->type, game::vcr::flak::VisualisationState::NoObject);
    TS_ASSERT(testee.smoke().empty());

    // Hit and verify
    testee.hitTorpedo(30, 2);
    TS_ASSERT_EQUALS(testee.objects().at(30)->type, game::vcr::flak::VisualisationState::NoObject);
    // For now, does not create smoke; killing the ship will.
}

/** Test beam functions.
    A: create and verify beams
    E: verify correct state */
void
TestGameVcrFlakVisualisationState::testBeam()
{
    // fireBeamFighterFighter
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
        TS_ASSERT(testee.beams().at(0) != 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.x, 500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.y, 600);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.z, 2500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.x, -500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.y, 600);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.z, 2500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->age, 0);
    }

    // fireBeamFighterShip
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
        TS_ASSERT(testee.beams().at(0) != 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.x, 500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.y, 600);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.z, 2500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.x, 1000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.y, -5000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.z, 250);
        TS_ASSERT_EQUALS(testee.beams().at(0)->age, 0);
    }

    // fireBeamShipFighter
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
        TS_ASSERT(testee.beams().at(0) != 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.x, 1000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.y, 2000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.z, 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.x, -500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.y, 600);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.z, 2500);
        TS_ASSERT_EQUALS(testee.beams().at(0)->age, 0);
    }

    // fireBeamShipShip
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
        TS_ASSERT(testee.beams().at(0) != 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.x, 1000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.y, 2000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->from.z, 0);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.x, 1000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.y, -5000);
        TS_ASSERT_EQUALS(testee.beams().at(0)->to.z, 250);
        TS_ASSERT_EQUALS(testee.beams().at(0)->age, 0);
    }
}

/** Test aging of smoke.
    A: create smoke by killing a ship
    E: verify smoke is generated and disappears after configured time */
void
TestGameVcrFlakVisualisationState::testSmokeAge()
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
    TS_ASSERT(testee.smoke().size() > 3);

    // Animate
    int n = 0;
    while (testee.animate()) {
        ++n;
        TS_ASSERT(!testee.smoke().empty());
        TS_ASSERT(n <= N);
    }
    TS_ASSERT_EQUALS(n, N-1);
}

/** Test aging of beams.
    A: create beam
    E: verify beam is generated and disappears after configured time */
void
TestGameVcrFlakVisualisationState::testBeamAge()
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
    TS_ASSERT_EQUALS(testee.beams().size(), 1U);

    // Animate
    int n = 0;
    while (testee.animate()) {
        ++n;
        TS_ASSERT(!testee.beams().empty());
        TS_ASSERT(n <= N);
    }
    TS_ASSERT_EQUALS(n, N-1);
}

/** Test ship turning.
    A: create two ships; set enemy
    E: verify that heading is adjusted towards final angle */
void
TestGameVcrFlakVisualisationState::testShipTurn()
{
    game::vcr::flak::VisualisationState testee;

    // Add ships and fleets
    testee.createShip(1, Position(1000, 1000, 0), makeShipInfo(3, false));
    testee.createShip(2, Position(1000, -1000, 0), makeShipInfo(4, true));
    testee.createFleet(0, 1000, 2000, 3, 1, 1);
    testee.createFleet(1, 1000, -1000, 4, 2, 1);
    testee.setEnemy(0, 2);

    // Initial angle of ship 1 is south-west (-0.75*pi)
    TS_ASSERT_DELTA(testee.ships().at(1)->heading, -2.3561, 0.0001);

    // Animate once: angle moves
    testee.animate();
    TS_ASSERT(testee.ships().at(1)->heading > -2.3562);

    // Animate: angle moves towards final value
    for (int i = 0; i < 100; ++i) {
        testee.animate();
    }

    // Final angle is south (-0.5*pi)
    TS_ASSERT_DELTA(testee.ships().at(1)->heading, -1.5707, 0.0001);
}

/** Test copying.
    A: create and populate a VisualisationState. Copy it.
    E: verify same content in both */
void
TestGameVcrFlakVisualisationState::testCopy()
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
    TS_ASSERT_EQUALS(testee.ships().at(2)->pos.y, -5000);
    TS_ASSERT_EQUALS(copy1.ships().at(2)->pos.y, -5000);
    TS_ASSERT_EQUALS(copy2.ships().at(2)->pos.y, -5000);

    TS_ASSERT_EQUALS(testee.beams().size(), 1U);
    TS_ASSERT_EQUALS(copy1.beams().size(), 1U);
    TS_ASSERT_EQUALS(copy2.beams().size(), 1U);
}

/** Test getTime().
    A: create VisualisationState. Call updateTime().
    E: verify result */
void
TestGameVcrFlakVisualisationState::testTime()
{
    game::vcr::flak::VisualisationState testee;
    testee.updateTime(777);

    TS_ASSERT_EQUALS(testee.getTime(), 777);
}
