/**
  *  \file test/game/map/ionstormtypetest.cpp
  *  \brief Test for game::map::IonStormType
  */

#include "game/map/ionstormtype.hpp"
#include "afl/test/testrunner.hpp"

using game::map::IonStorm;
using game::map::ObjectVector;

AFL_TEST("game.map.IonStormType", a)
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
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(10));
    a.checkNonNull("02. getObjectByIndex", testee.getObjectByIndex(20));
    a.checkNonNull("03. getObjectByIndex", testee.getObjectByIndex(30));

    a.checkEqual("11. findNextIndex", testee.findNextIndex(0), 20);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(20), 30);
    a.checkEqual("13. findNextIndex", testee.findNextIndex(30), 0);
}
