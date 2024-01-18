/**
  *  \file test/game/map/playedbasetypetest.cpp
  *  \brief Test for game::map::PlayedBaseType
  */

#include "game/map/playedbasetype.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"

using game::map::ObjectVector;
using game::map::Planet;

AFL_TEST("game.map.PlayedBaseType", a)
{
    // ObjectVector
    ObjectVector<Planet> pv;

    // Environment for internalCheck
    afl::string::NullTranslator tx;
    game::map::Configuration config;
    afl::sys::Log log;

    // Some data
    game::map::PlanetData pd;
    pd.owner = 7;

    game::map::BaseData bd;
    bd.numBaseDefensePosts = 10;

    game::PlayerSet_t ps(3);

    // Blank planet object - not visible
    Planet* p1 = pv.create(1);
    p1->internalCheck(config, ps, 15, tx, log);

    // Planet with XY coordinates - not visible
    Planet* p2 = pv.create(2);
    p2->setPosition(game::map::Point(1200, 2000));
    p2->internalCheck(config, ps, 15, tx, log);

    // Planet with actual data - not visible, no base
    Planet* p3 = pv.create(3);
    p3->setPosition(game::map::Point(1300, 2000));
    p3->addCurrentPlanetData(pd, ps);
    p3->setPlayability(game::map::Object::Playable);
    p3->internalCheck(config, ps, 15, tx, log);

    // Planet with base - visible, no base
    Planet* p4 = pv.create(4);
    p4->setPosition(game::map::Point(1400, 2000));
    p4->addCurrentPlanetData(pd, ps);
    p4->addCurrentBaseData(bd, ps);
    p4->setPlayability(game::map::Object::Playable);
    p4->internalCheck(config, ps, 15, tx, log);

    // Another planet with base - visible, no base
    Planet* p5 = pv.create(5);
    p5->setPosition(game::map::Point(1500, 2000));
    p5->addCurrentPlanetData(pd, ps);
    p5->addCurrentBaseData(bd, ps);
    p5->setPlayability(game::map::Object::Playable);
    p5->internalCheck(config, ps, 15, tx, log);

    // Test
    game::map::PlayedBaseType testee(pv);
    a.checkNull("01. getObjectByIndex", testee.getObjectByIndex(1));
    a.checkNull("02. getObjectByIndex", testee.getObjectByIndex(2));
    a.checkNull("03. getObjectByIndex", testee.getObjectByIndex(3));
    a.checkNonNull("04. getObjectByIndex", testee.getObjectByIndex(4));
    a.checkNonNull("05. getObjectByIndex", testee.getObjectByIndex(5));

    a.checkEqual("11. findNextIndex", testee.findNextIndex(0), 4);
    a.checkEqual("12. findNextIndex", testee.findNextIndex(4), 5);
    a.checkEqual("13. findNextIndex", testee.findNextIndex(5), 0);
}
