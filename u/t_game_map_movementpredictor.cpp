/**
  *  \file u/t_game_map_movementpredictor.cpp
  *  \brief Test for game::map::MovementPredictor
  */

#include "game/map/movementpredictor.hpp"

#include "t_game_map.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/io/internaldirectory.hpp"
#include "afl/string/format.hpp"
#include "game/root.hpp"
#include "game/spec/mission.hpp"
#include "game/test/registrationkey.hpp"
#include "game/test/specificationloader.hpp"
#include "game/test/stringverifier.hpp"
#include "game/turn.hpp"
#include "afl/charset/utf8charset.hpp"

using game::map::Point;
using game::map::Ship;
using game::spec::Mission;

namespace {
    // Config
    const int HullId = 12;
    const int EngineId = 3;
    const int Fuel = 200;
    const int Owner = 2;

    void addSpec(game::spec::ShipList& shipList)
    {
        game::spec::Hull* pHull = shipList.hulls().create(HullId);
        pHull->setMaxFuel(Fuel);
        pHull->setMaxCrew(100);
        pHull->setMass(100);
        pHull->setNumEngines(2);

        game::spec::Engine* pEngine = shipList.engines().create(EngineId);
        pEngine->setTechLevel(5);
    }

    Ship* addShip(game::map::Universe& univ, int id)
    {
        Ship* pShip = univ.ships().create(id);
        game::map::ShipData data;
        data.owner                     = Owner;
        data.friendlyCode              = "hi";
        data.x                         = 1000;
        data.y                         = 1000;
        data.waypointDX                = 1;
        data.waypointDY                = 0;
        data.engineType                = EngineId;
        data.hullType                  = HullId;
        data.beamType                  = 0;
        data.launcherType              = 0;
        data.mission                   = 0;
        data.missionTowParameter       = 0;
        data.missionInterceptParameter = 0;
        data.warpFactor                = 3;

        pShip->addCurrentShipData(data, game::PlayerSet_t(Owner));
        pShip->internalCheck();
        pShip->setPlayability(game::map::Object::Playable);
        return pShip;
    }
}


/** Brute force combination test.
    This tests all combinations of 5 ships intercepting or towing each other (11^5 = 161051 combinations).
    The idea is to trigger loop resolution bugs: this triggers on #371, and found #374. */
void
TestGameMapMovementPredictor::testCombinations()
{
    const int NumShips = 5;

    // Root
    game::Root root(afl::io::InternalDirectory::create("<game>"),
                    *new game::test::SpecificationLoader(),
                    game::HostVersion(),
                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::test::RegistrationKey::Unregistered, 6)),
                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                    std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                    game::Root::Actions_t());

    // Ship list
    game::spec::ShipList shipList;
    addSpec(shipList);

    // All combinations: Nothing vs. Tow each ship vs. Intercept each ship
    const int32_t Radix = NumShips*2 + 1;
    const int32_t Limit = Radix*Radix*Radix*Radix*Radix;   // Radix**NumShips
    static_assert(NumShips == 5, "Limit assumes NumShips=5");

    for (int32_t iteration = 0; iteration < Limit; ++iteration) {
        // Game: set up the ships
        game::Game game;
        game::map::Universe& univ = game.currentTurn().universe();
        String_t iterationName = afl::string::Format("#%d", iteration);
        int32_t selector = iteration;
        for (int i = 1; i <= NumShips; ++i) {
            // Pick selector
            int thisSelector = selector % Radix;
            selector /= Radix;

            // Create the ship
            Ship* pShip = addShip(univ, i);
            if (thisSelector == 0) {
                iterationName += afl::string::Format(", %d passive", i);
            } else if (thisSelector <= NumShips) {
                iterationName += afl::string::Format(", %d tows %d", i, thisSelector);
                pShip->setMission(Mission::msn_Tow, 0, thisSelector);
            } else {
                iterationName += afl::string::Format(", %d intercepts %d", i, thisSelector - NumShips);
                pShip->setMission(Mission::msn_Intercept, thisSelector - NumShips, i);
            }
        }

        // Testee
        game::map::MovementPredictor testee;
        TSM_ASSERT_THROWS_NOTHING(iterationName.c_str(), testee.computeMovement(univ, game, shipList, root));

        // We cannot verify much in a general way. Just verify that we can get all positions.
        for (int i = 1; i <= NumShips; ++i) {
            TSM_ASSERT(iterationName.c_str(), testee.getShipPosition(i).isValid());
        }
    }
}

/** Test some regular movement.
    This actually verifies the movements. */
void
TestGameMapMovementPredictor::testMovement()
{
    // Root
    game::Root root(afl::io::InternalDirectory::create("<game>"),
                    *new game::test::SpecificationLoader(),
                    game::HostVersion(),
                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::test::RegistrationKey::Unregistered, 6)),
                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                    std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                    game::Root::Actions_t());

    // Ship list
    game::spec::ShipList shipList;
    addSpec(shipList);

    // First ship: move by [0,15]
    game::Game game;
    game::map::Universe& univ = game.currentTurn().universe();
    Ship* p1 = addShip(univ, 1);
    p1->setWaypoint(Point(1000, 1015));
    p1->setWarpFactor(3);

    // Second ship: move by [10,0]
    Ship* p2 = addShip(univ, 2);
    p2->setWaypoint(Point(1010, 1000));
    p2->setWarpFactor(4);

    // Third ship: intercept second at warp 3
    Ship* p3 = addShip(univ, 3);
    p3->setWaypoint(Point(1000, 1000));
    p3->setMission(Mission::msn_Intercept, 2, 0);
    p3->setWarpFactor(3);

    // Fourth ship: intercept second at warp 4
    Ship* p4 = addShip(univ, 4);
    p4->setWaypoint(Point(1000, 1000));
    p4->setMission(Mission::msn_Intercept, 2, 0);
    p4->setWarpFactor(4);

    // Simulate and verify
    game::map::MovementPredictor testee;
    TS_ASSERT_THROWS_NOTHING(testee.computeMovement(univ, game, shipList, root));

    Point pt;
    TS_ASSERT(testee.getShipPosition(1).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1000, 1009));
    TS_ASSERT(testee.getShipPosition(2).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1010, 1000));
    TS_ASSERT(testee.getShipPosition(3).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1009, 1000));
    TS_ASSERT(testee.getShipPosition(4).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1010, 1000));

    TS_ASSERT(!testee.getShipPosition(0).isValid());
    TS_ASSERT(!testee.getShipPosition(5).isValid());
    TS_ASSERT(!testee.getShipPosition(32700).isValid());
}

/** Test intercept loop resolution. */
void
TestGameMapMovementPredictor::testInterceptLoop()
{
    // Root
    game::Root root(afl::io::InternalDirectory::create("<game>"),
                    *new game::test::SpecificationLoader(),
                    game::HostVersion(),
                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::test::RegistrationKey::Unregistered, 6)),
                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                    std::auto_ptr<afl::charset::Charset>(new afl::charset::Utf8Charset()),
                    game::Root::Actions_t());

    // Ship list
    game::spec::ShipList shipList;
    addSpec(shipList);

    // Ship 1: intercept 2 at warp 3 (will not reach centroid point)
    game::Game game;
    game::map::Universe& univ = game.currentTurn().universe();
    Ship* p1 = addShip(univ, 1);
    p1->setPosition(Point(1000, 1000));
    p1->setWaypoint(Point(1000, 1000));
    p1->setMission(Mission::msn_Intercept, 2, 0);
    p1->setWarpFactor(3);

    // Ship 2: intercept 1 at warp 4
    Ship* p2 = addShip(univ, 2);
    p2->setPosition(Point(1020, 1000));
    p2->setWaypoint(Point(1020, 1000));
    p2->setMission(Mission::msn_Intercept, 1, 0);
    p2->setWarpFactor(4);

    // Ship 3: intercept 1 at warp 9
    Ship* p3 = addShip(univ, 3);
    p3->setPosition(Point(1000, 1010));
    p3->setWaypoint(Point(1000, 1010));
    p3->setMission(Mission::msn_Intercept, 1, 0);
    p3->setWarpFactor(9);

    // Simulate and verify
    game::map::MovementPredictor testee;
    TS_ASSERT_THROWS_NOTHING(testee.computeMovement(univ, game, shipList, root));

    Point pt;
    TS_ASSERT(testee.getShipPosition(1).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1009, 1000));
    TS_ASSERT(testee.getShipPosition(2).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1010, 1000));
    TS_ASSERT(testee.getShipPosition(3).get(pt));
    TS_ASSERT_EQUALS(pt, Point(1009, 1000));
}
