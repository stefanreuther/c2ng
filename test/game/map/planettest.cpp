/**
  *  \file test/game/map/planettest.cpp
  *  \brief Test for game::map::Planet
  */

#include "game/map/planet.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/parser/messageinformation.hpp"

/** Test AutobuildSettings object. */
AFL_TEST("game.map.Planet:AutobuildSettings", a)
{
    game::map::Planet::AutobuildSettings t;

    // Needs to be properly default-initialized to "unknown"
    a.checkEqual("01. goal",  t.goal[0].isValid(), false);
    a.checkEqual("02. speed", t.speed[0].isValid(), false);
}

/** Test copying. */
AFL_TEST("game.map.Planet:copy", a)
{
    game::map::Planet t(19);
    t.setPlayability(game::map::Planet::Playable);
    t.setAutobuildGoal(game::MineBuilding, 333);
    t.setAutobuildSpeed(game::MineBuilding, 77);
    t.messages().add(3);

    game::map::Planet t2(t);
    a.checkEqual("01. getPlayability", t2.getPlayability(), game::map::Planet::Playable);
    a.checkEqual("02. getAutobuildGoal", t2.getAutobuildGoal(game::MineBuilding), 333);
    a.checkEqual("03. getAutobuildSpeed", t2.getAutobuildSpeed(game::MineBuilding), 77);
    a.checkEqual("04. messages", t2.messages().get().size(), 1U);
}

/** Test isKnownToHaveNatives() status. */
AFL_TEST("game.map.Planet:isKnownToHaveNatives", a)
{
    game::map::Planet t(19);
    t.setPosition(game::map::Point(1000, 1000));
    t.setKnownToHaveNatives(true);

    afl::string::NullTranslator tx;
    afl::sys::Log log;
    t.internalCheck(game::map::Configuration(), game::PlayerSet_t(), 10, tx, log);

    a.check("01. isKnownToHaveNatives", t.isKnownToHaveNatives());
    a.check("02. hasAnyPlanetData", t.hasAnyPlanetData());
}

/** Test handling of mi_PlanetAddedN. */
AFL_TEST("game.map.Planet:addMessageInformation:add-mineral", a)
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

    a.checkEqual("01. Neutronium", t.getOreGround(Element::Neutronium).orElse(-1), 110);
    a.checkEqual("02. Tritanium",  t.getOreGround(Element::Tritanium).orElse(-1), 220);
    a.checkEqual("03. Duranium",   t.getOreGround(Element::Duranium).orElse(-1), 330);
    a.checkEqual("04. Molybdenum", t.getOreGround(Element::Molybdenum).orElse(-1), 440);
}
