/**
  *  \file test/game/map/anyplanettypetest.cpp
  *  \brief Test for game::map::AnyPlanetType
  */

#include "game/map/anyplanettype.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"

using game::map::ObjectVector;
using game::map::Planet;

AFL_TEST("game.map.AnyPlanetType", a)
{
    // ObjectVector
    ObjectVector<Planet> pv;

    // Environment for internalCheck
    afl::string::NullTranslator tx;
    game::map::Configuration config;
    afl::sys::Log log;

    // Blank planet object - not visible
    Planet* p1 = pv.create(100);
    p1->internalCheck(config, game::PlayerSet_t(3), 15, tx, log);

    // Planet with XY coordinates
    Planet* p2 = pv.create(200);
    p2->setPosition(game::map::Point(1200, 2000));
    p2->internalCheck(config, game::PlayerSet_t(3), 15, tx, log);

    // Planet with actual data
    Planet* p3 = pv.create(300);
    game::map::PlanetData pd3;
    pd3.owner = 7;
    p3->setPosition(game::map::Point(1300, 2000));
    p3->addCurrentPlanetData(pd3, game::PlayerSet_t(3));
    p3->internalCheck(config, game::PlayerSet_t(3), 15, tx, log);

    // Test
    game::map::AnyPlanetType testee(pv);
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(50));
    a.checkNull("02. getObjectByIndex", testee.getObjectByIndex(100));
    a.checkNonNull("03. getObjectByIndex", testee.getObjectByIndex(200));
    a.checkNonNull("04. getObjectByIndex", testee.getObjectByIndex(300));

    a.checkEqual("11. findNextIndex", testee.findNextIndex(100), 200);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(200), 300);
    a.checkEqual("13. findNextIndex", testee.findNextIndex(300), 0);
}
