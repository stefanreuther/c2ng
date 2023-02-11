/**
  *  \file u/t_game_map_anyshiptype.cpp
  *  \brief Test for game::map::AnyShipType
  */

#include "game/map/anyshiptype.hpp"

#include "t_game_map.hpp"

using game::map::ObjectVector;
using game::map::Ship;

void
TestGameMapAnyShipType::testIt()
{
    ObjectVector<Ship> sv;

    // Blank ship
    Ship* s1 = sv.create(100);
    s1->internalCheck();

    // Visible ship
    Ship* s2 = sv.create(200);
    s2->addShipXYData(game::map::Point(1000, 1200), 5, 100, game::PlayerSet_t(5));
    s2->internalCheck();

    // Played ship
    Ship* s3 = sv.create(300);
    game::map::ShipData sd3;
    sd3.owner = 4;
    sd3.x = 1000;
    sd3.y = 1300;
    s3->addCurrentShipData(sd3, game::PlayerSet_t(4));
    s3->setPlayability(game::map::Object::Playable);
    s3->internalCheck();

    // Test
    game::map::AnyShipType testee(sv);
    TS_ASSERT(testee.getObjectByIndex(50) == 0);
    TS_ASSERT(testee.getObjectByIndex(100) == 0);
    TS_ASSERT(testee.getObjectByIndex(200) != 0);
    TS_ASSERT(testee.getObjectByIndex(300) != 0);

    TS_ASSERT_EQUALS(testee.findNextIndex(100), 200);
    TS_ASSERT_EQUALS(testee.findNextIndex(200), 300);
    TS_ASSERT_EQUALS(testee.findNextIndex(300), 0);
}

