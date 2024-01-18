/**
  *  \file test/game/map/playedshiptypetest.cpp
  *  \brief Test for game::map::PlayedShipType
  */

#include "game/map/playedshiptype.hpp"
#include "afl/test/testrunner.hpp"

using game::map::ObjectVector;
using game::map::Ship;

namespace {
    void addShip(ObjectVector<Ship>& sv, int id, int numBeams)
    {
        Ship* sh = sv.create(id);
        game::map::ShipData sd;
        sd.owner = 4;
        sd.x = 1000;
        sd.y = 1300;
        sd.beamType = numBeams != 0 ? 10 : 0;
        sd.numBeams = numBeams;
        sh->addCurrentShipData(sd, game::PlayerSet_t(4));
        sh->setPlayability(game::map::Object::Playable);
        sh->internalCheck(game::PlayerSet_t(4), 15);
    }
}

AFL_TEST("game.map.PlayedShipType:basics", a)
{
    ObjectVector<Ship> sv;

    // Blank ship
    Ship* s1 = sv.create(100);
    s1->internalCheck(game::PlayerSet_t(5), 15);

    // Visible ship
    Ship* s2 = sv.create(200);
    s2->addShipXYData(game::map::Point(1000, 1200), 5, 100, game::PlayerSet_t(5));
    s2->internalCheck(game::PlayerSet_t(5), 15);

    // Played ship
    Ship* s3 = sv.create(300);
    game::map::ShipData sd3;
    sd3.owner = 4;
    sd3.x = 1000;
    sd3.y = 1300;
    s3->addCurrentShipData(sd3, game::PlayerSet_t(4));
    s3->setPlayability(game::map::Object::Playable);
    s3->internalCheck(game::PlayerSet_t(4), 15);

    // Test
    game::map::PlayedShipType testee(sv);
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(50));
    a.checkNull("02. getObjectByIndex", testee.getObjectByIndex(100));
    a.checkNull("03. getObjectByIndex", testee.getObjectByIndex(200));
    a.checkNonNull("04. getObjectByIndex", testee.getObjectByIndex(300));

    a.checkEqual("11. findNextIndex", testee.findNextIndex(100), 300);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(300), 0);
}

AFL_TEST("game.map.PlayedShipType:countCapitalShips", a)
{
    ObjectVector<Ship> sv;
    addShip(sv, 10, 0);
    addShip(sv, 20, 1);
    addShip(sv, 30, 2);
    addShip(sv, 40, 0);
    addShip(sv, 50, 5);
    addShip(sv, 60, 0);
    addShip(sv, 70, 7);

    game::map::PlayedShipType testee(sv);
    a.checkEqual("", testee.countCapitalShips(), 4);
}
