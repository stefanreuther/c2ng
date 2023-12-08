/**
  *  \file u/t_game_map_planet.cpp
  *  \brief Test for game::map::Planet
  */

#include "game/map/planet.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "game/map/configuration.hpp"
#include "game/parser/messageinformation.hpp"

/** Test AutobuildSettings object. */
void
TestGameMapPlanet::testAutobuildSettings()
{
    game::map::Planet::AutobuildSettings t;

    // Needs to be properly default-initialized to "unknown"
    TS_ASSERT_EQUALS(t.goal[0].isValid(), false);
    TS_ASSERT_EQUALS(t.speed[0].isValid(), false);
}

/** Test copying. */
void
TestGameMapPlanet::testCopy()
{
    game::map::Planet t(19);
    t.setPlayability(game::map::Planet::Playable);
    t.setAutobuildGoal(game::MineBuilding, 333);
    t.setAutobuildSpeed(game::MineBuilding, 77);
    t.messages().add(3);

    game::map::Planet t2(t);
    TS_ASSERT_EQUALS(t2.getPlayability(), game::map::Planet::Playable);
    TS_ASSERT_EQUALS(t2.getAutobuildGoal(game::MineBuilding), 333);
    TS_ASSERT_EQUALS(t2.getAutobuildSpeed(game::MineBuilding), 77);
    TS_ASSERT_EQUALS(t2.messages().get().size(), 1U);
}

/** Test isKnownToHaveNatives() status. */
void
TestGameMapPlanet::testKnownToHaveNatives()
{
    game::map::Planet t(19);
    t.setPosition(game::map::Point(1000, 1000));
    t.setKnownToHaveNatives(true);

    afl::string::NullTranslator tx;
    afl::sys::Log log;
    t.internalCheck(game::map::Configuration(), game::PlayerSet_t(), 10, tx, log);

    TS_ASSERT(t.isKnownToHaveNatives());
    TS_ASSERT(t.hasAnyPlanetData());
}

/** Test handling of mi_PlanetAddedN. */
void
TestGameMapPlanet::testAddMineral()
{
    using game::Element;

    game::map::Planet t(19);
    t.setOreGround(Element::Neutronium, 100);
    t.setOreGround(Element::Tritanium,  200);
    t.setOreGround(Element::Duranium,   300);
    t.setOreGround(Element::Molybdenum, 400);

    game::parser::MessageInformation info(game::parser::MessageInformation::Planet, 19, 55);
    info.addValue(game::parser::mi_PlanetAddedN, 10);
    info.addValue(game::parser::mi_PlanetAddedT, 20);
    info.addValue(game::parser::mi_PlanetAddedD, 30);
    info.addValue(game::parser::mi_PlanetAddedM, 40);

    t.addMessageInformation(info);

    TS_ASSERT_EQUALS(t.getOreGround(Element::Neutronium).orElse(-1), 110);
    TS_ASSERT_EQUALS(t.getOreGround(Element::Tritanium).orElse(-1), 220);
    TS_ASSERT_EQUALS(t.getOreGround(Element::Duranium).orElse(-1), 330);
    TS_ASSERT_EQUALS(t.getOreGround(Element::Molybdenum).orElse(-1), 440);
}

