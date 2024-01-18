/**
  *  \file test/game/map/anyshiptypetest.cpp
  *  \brief Test for game::map::AnyShipType
  */

#include "game/map/anyshiptype.hpp"
#include "afl/test/testrunner.hpp"

using game::map::ObjectVector;
using game::map::Ship;

AFL_TEST("game.map.AnyShipType", a)
{
    ObjectVector<Ship> sv;

    // Blank ship
    Ship* s1 = sv.create(100);
    s1->internalCheck(game::PlayerSet_t(5), 15);

    // Visible ship
    Ship* s2 = sv.create(200);
    s2->addShipXYData(game::map::Point(1000, 1200), 5, 100, game::PlayerSet_t(4));
    s2->internalCheck(game::PlayerSet_t(4), 15);

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
    game::map::AnyShipType testee(sv);
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(50));
    a.checkNull("02. getObjectByIndex", testee.getObjectByIndex(100));
    a.checkNonNull("03. getObjectByIndex", testee.getObjectByIndex(200));
    a.checkNonNull("04. getObjectByIndex", testee.getObjectByIndex(300));

    a.checkEqual("11. findNextIndex", testee.findNextIndex(100), 200);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(200), 300);
    a.checkEqual("13. findNextIndex", testee.findNextIndex(300), 0);
}
