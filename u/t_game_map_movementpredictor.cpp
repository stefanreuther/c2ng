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

/** Brute force combination test.
    This tests all combinations of 5 ships intercepting or towing each other (11^5 = 161051 combinations).
    The idea is to trigger loop resolution bugs: this triggers on #371, and found #374. */
void
TestGameMapMovementPredictor::testCombinations()
{
    // Config
    const int HullId = 12;
    const int EngineId = 3;
    const int Fuel = 200;
    const int NumShips = 5;
    const int Owner = 2;

    // Root
    game::Root root(afl::io::InternalDirectory::create("<game>"),
                    *new game::test::SpecificationLoader(),
                    game::HostVersion(),
                    std::auto_ptr<game::RegistrationKey>(new game::test::RegistrationKey(game::test::RegistrationKey::Unregistered, 6)),
                    std::auto_ptr<game::StringVerifier>(new game::test::StringVerifier()),
                    game::Root::Actions_t());


    // Ship list
    game::spec::ShipList shipList;
    game::spec::Hull* pHull = shipList.hulls().create(HullId);
    pHull->setMaxFuel(Fuel);
    pHull->setMaxCrew(100);
    pHull->setMass(100);
    pHull->setNumEngines(2);

    game::spec::Engine* pEngine = shipList.engines().create(EngineId);
    pEngine->setTechLevel(5);

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
            game::map::Ship* pShip = univ.ships().create(i);
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
            if (thisSelector == 0) {
                iterationName += afl::string::Format(", %d passive", i);
            } else if (thisSelector <= NumShips) {
                iterationName += afl::string::Format(", %d tows %d", i, thisSelector);
                data.mission = game::spec::Mission::msn_Tow;
                data.missionTowParameter = thisSelector;
            } else {
                iterationName += afl::string::Format(", %d intercepts %d", i, thisSelector - NumShips);
                data.mission = game::spec::Mission::msn_Intercept;
                data.missionInterceptParameter = thisSelector - NumShips;
            }
            pShip->addCurrentShipData(data, game::PlayerSet_t(Owner));
            pShip->internalCheck();
            pShip->setPlayability(game::map::Object::Playable);
        }

        // Testee
        game::map::MovementPredictor testee;
        TSM_ASSERT_THROWS_NOTHING(iterationName.c_str(), testee.computeMovement(univ, game, shipList, root));

        // We cannot verify much in a general way. Just verify that we can get all positions.
        for (int i = 1; i <= NumShips; ++i) {
            game::map::Point pt;
            TSM_ASSERT_EQUALS(iterationName.c_str(), testee.getShipPosition(i, pt), true);
        }
    }
}

