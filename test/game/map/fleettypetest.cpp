/**
  *  \file test/game/map/fleettypetest.cpp
  *  \brief Test for game::map::FleetType
  */

#include "game/map/fleettype.hpp"
#include "afl/test/testrunner.hpp"

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

AFL_TEST("game.map.FleetType", a)
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
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(10));   // not in fleet
    a.checkNull("02. getObjectByIndex", testee.getObjectByIndex(20));   // fleet member, not a leader
    a.checkNonNull("03. getObjectByIndex", testee.getObjectByIndex(30));   // fleet leader
    a.checkNull("04. getObjectByIndex", testee.getObjectByIndex(40));   // not in fleet
    a.checkNonNull("05. getObjectByIndex", testee.getObjectByIndex(50));   // fleet leader

    a.checkEqual("11. findNextIndex", testee.findNextIndex(0), 30);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(30), 50);
    a.checkEqual("13. findNextIndex", testee.findNextIndex(50), 0);
}
