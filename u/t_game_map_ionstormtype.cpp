/**
  *  \file u/t_game_map_ionstormtype.cpp
  *  \brief Test for game::map::IonStormType
  */

#include "game/map/ionstormtype.hpp"

#include "t_game_map.hpp"

using game::map::IonStorm;
using game::map::ObjectVector;

void
TestGameMapIonStormType::testIt()
{
    ObjectVector<IonStorm> sv;

    // Empty
    sv.create(10);

    // Non-empty
    IonStorm* i2 = sv.create(20);
    i2->setVoltage(10);
    i2->setRadius(90);
    i2->setPosition(game::map::Point(1000, 1000));

    // Non-empty
    IonStorm* i3 = sv.create(30);
    i3->setVoltage(10);
    i3->setRadius(90);
    i3->setPosition(game::map::Point(2000, 1000));

    // Test
    game::map::IonStormType testee(sv);
    TS_ASSERT(testee.getObjectByIndex(10) == 0);
    TS_ASSERT(testee.getObjectByIndex(20) != 0);
    TS_ASSERT(testee.getObjectByIndex(30) != 0);

    TS_ASSERT_EQUALS(testee.findNextIndex(0), 20);
    TS_ASSERT_EQUALS(testee.findNextIndex(20), 30);
    TS_ASSERT_EQUALS(testee.findNextIndex(30), 0);
}

