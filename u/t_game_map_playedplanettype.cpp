/**
  *  \file u/t_game_map_playedplanettype.cpp
  *  \brief Test for game::map::PlayedPlanetType
  */

#include "game/map/playedplanettype.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"

using game::map::ObjectVector;
using game::map::Planet;

void
TestGameMapPlayedPlanetType::testIt()
{
    // ObjectVector
    ObjectVector<Planet> pv;

    // Environment for internalCheck
    afl::string::NullTranslator tx;
    game::map::Configuration config;
    afl::sys::Log log;

    // Blank planet object - not visible
    Planet* p1 = pv.create(100);
    p1->internalCheck(config, tx, log);

    // Planet with XY coordinates
    Planet* p2 = pv.create(200);
    p2->setPosition(game::map::Point(1200, 2000));
    p2->internalCheck(config, tx, log);

    // Planet with actual data
    Planet* p3 = pv.create(300);
    game::map::PlanetData pd3;
    pd3.owner = 7;
    p3->setPosition(game::map::Point(1300, 2000));
    p3->addCurrentPlanetData(pd3, game::PlayerSet_t(3));
    p3->setPlayability(game::map::Object::Playable);
    p3->internalCheck(config, tx, log);

    // Test
    game::map::PlayedPlanetType testee(pv);
    TS_ASSERT(testee.getObjectByIndex(50) == 0);
    TS_ASSERT(testee.getObjectByIndex(100) == 0);
    TS_ASSERT(testee.getObjectByIndex(200) == 0);
    TS_ASSERT(testee.getObjectByIndex(300) != 0);

    TS_ASSERT_EQUALS(testee.findNextIndex(100), 300);
    TS_ASSERT_EQUALS(testee.findNextIndex(300), 0);
}

