/**
  *  \file u/t_game_map_fleettype.cpp
  *  \brief Test for game::map::FleetType
  */

#include "game/map/fleettype.hpp"

#include "t_game_map.hpp"

using game::map::ObjectVector;
using game::map::Ship;

namespace {
    void addShip(ObjectVector<Ship>& sv, game::Id_t shipId, int fleetNumber)
    {
        Ship* sh = sv.create(shipId);
        game::map::ShipData sd;
        sd.owner = 4;
        sd.x = 1000;
        sd.y = 1300;
        sh->addCurrentShipData(sd, game::PlayerSet_t(4));
        sh->setPlayability(game::map::Object::Playable);
        sh->internalCheck(game::PlayerSet_t(4), 10);
        sh->setFleetNumber(fleetNumber);
    }
}

void
TestGameMapFleetType::testIt()
{
    // Setup
    ObjectVector<Ship> sv;
    addShip(sv, 10, 0);
    addShip(sv, 20, 30);
    addShip(sv, 30, 30);
    addShip(sv, 40, 0);
    addShip(sv, 50, 50);

    // Test
    game::map::FleetType testee(sv);
    TS_ASSERT(testee.getObjectByIndex(10) == 0);   // not in fleet
    TS_ASSERT(testee.getObjectByIndex(20) == 0);   // fleet member, not a leader
    TS_ASSERT(testee.getObjectByIndex(30) != 0);   // fleet leader
    TS_ASSERT(testee.getObjectByIndex(40) == 0);   // not in fleet
    TS_ASSERT(testee.getObjectByIndex(50) != 0);   // fleet leader

    TS_ASSERT_EQUALS(testee.findNextIndex(0), 30);
    TS_ASSERT_EQUALS(testee.findNextIndex(30), 50);
    TS_ASSERT_EQUALS(testee.findNextIndex(50), 0);
}

