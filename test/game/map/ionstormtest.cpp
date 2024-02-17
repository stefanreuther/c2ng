/**
  *  \file test/game/map/ionstormtest.cpp
  *  \brief Test for game::map::IonStorm
  */

#include "game/map/ionstorm.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/interpreterinterface.hpp"
#include "game/test/interpreterinterface.hpp"

namespace gp = game::parser;

namespace {
    void configureIonStorm(game::map::IonStorm& i)
    {
        i.setName("Klothilde");
        i.setPosition(game::map::Point(2001, 3014));
        i.setRadius(40);
        i.setVoltage(180);
        i.setWarpFactor(6);
        i.setHeading(225);
        i.setIsGrowing(true);
    }
}

/** Simple setter/getter test.
    A: create an ion storm. Use setters/getters.
    E: expected initial/stored values returned */
AFL_TEST("game.map.IonStorm", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    int n;
    game::map::Point pt;
    int32_t d;

    // Test initial state
    game::map::IonStorm i(3);
    a.checkEqual("01. getName",          i.getName(game::PlainName, tx, iface), "Ion storm #3");
    a.checkEqual("02. getId",            i.getId(), 3);
    a.checkEqual("03. getOwner",         i.getOwner().get(n), true);
    a.checkEqual("04. owner",            n, 0);
    a.checkEqual("05. getPosition",      i.getPosition().get(pt), false);
    a.checkEqual("06. getRadius",        i.getRadius().get(n), false);
    a.checkEqual("07. getRadiusSquared", i.getRadiusSquared().get(d), false);
    a.checkEqual("08. getClass",         i.getClass().get(n), false);
    a.checkEqual("09. getVoltage",       i.getVoltage().get(n), false);
    a.checkEqual("10. getHeading",       i.getHeading().get(n), false);
    a.checkEqual("11. getWarpFactor",    i.getWarpFactor().get(n), false);
    a.check("12. isGrowing",            !i.isGrowing());
    a.check("13. isActive",             !i.isActive());
    a.checkEqual("14. getName/1",        i.getName(tx), "Ion storm #3");

    // Populate it
    configureIonStorm(i);

    // Verify
    a.checkEqual("21. getName",          i.getName(game::PlainName, tx, iface), "Klothilde");
    a.checkEqual("22. getName",          i.getName(game::LongName, tx, iface), "Ion storm #3: Klothilde");
    a.checkEqual("23. getName",          i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    a.checkEqual("24. getId",            i.getId(), 3);
    a.checkEqual("25. getOwner",         i.getOwner().get(n), true);
    a.checkEqual("26. owner",            n, 0);
    a.checkEqual("27. getPosition",      i.getPosition().get(pt), true);
    a.checkEqual("28. x",                pt.getX(), 2001);
    a.checkEqual("29. y",                pt.getY(), 3014);
    a.checkEqual("30. getRadius",        i.getRadius().get(n), true);
    a.checkEqual("31. radius",           n, 40);
    a.checkEqual("32. getRadiusSquared", i.getRadiusSquared().get(d), true);
    a.checkEqual("33. radius2",          d, 1600);
    a.checkEqual("34. getClass",         i.getClass().get(n), true);
    a.checkEqual("35. class",            n, 4);
    a.checkEqual("36. getVoltage",       i.getVoltage().get(n), true);
    a.checkEqual("37. voltags",          n, 180);
    a.checkEqual("38. getHeading",       i.getHeading().get(n), true);
    a.checkEqual("39. heading",          n, 225);
    a.checkEqual("40. getWarpFactor",    i.getWarpFactor().get(n), true);
    a.checkEqual("41. warp",             n, 6);
    a.check("42. isGrowing",             i.isGrowing());
    a.check("43. isActive",              i.isActive());
    a.checkEqual("44. getName/1",        i.getName(tx), "Klothilde");
}

/** Test addMessageInformation to clear a storm.
    A: call addMessageInformation with voltage=0
    E: ion storm no longer active */
AFL_TEST("game.map.IonStorm:addMessageInformation:clear", a)
{
    game::map::IonStorm i(3);
    configureIonStorm(i);

    gp::MessageInformation info(gp::MessageInformation::IonStorm, i.getId(), 99);
    info.addValue(gp::mi_IonVoltage, 0);
    i.addMessageInformation(info);

    a.check("isActive", !i.isActive());
}

/** Test addMessageInformation, minimum data case.
    A: call addMessageInformation with voltage, x, y, radius.
    E: ion storm updated, old data kept */
AFL_TEST("game.map.IonStorm:addMessageInformation:min", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    int n;
    game::map::Point pt;

    game::map::IonStorm i(3);
    configureIonStorm(i);

    gp::MessageInformation info(gp::MessageInformation::IonStorm, i.getId(), 99);
    info.addValue(gp::mi_IonVoltage, 20);
    info.addValue(gp::mi_X, 900);
    info.addValue(gp::mi_Radius, 70);
    info.addValue(gp::mi_Y, 1800);
    i.addMessageInformation(info);

    a.checkEqual("01. getName",       i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    a.checkEqual("02. getPosition",   i.getPosition().get(pt), true);
    a.checkEqual("03. x",             pt.getX(), 900);
    a.checkEqual("04. y",             pt.getY(), 1800);
    a.checkEqual("05. getRadius",     i.getRadius().get(n), true);
    a.checkEqual("06. radius",        n, 70);
    a.checkEqual("07. getClass",      i.getClass().orElse(-1), 1);
    a.checkEqual("08. getVoltage",    i.getVoltage().orElse(-1), 20);
    a.checkEqual("09. getHeading",    i.getHeading().orElse(-1), 225);
    a.checkEqual("10. getWarpFactor", i.getWarpFactor().orElse(-1), 6);
    a.checkEqual("11",                i.isGrowing(), false);
    a.checkEqual("12",                i.isActive(), true);
}

/** Test addMessageInformation, maximum data case.
    A: call addMessageInformation with all data.
    E: ion storm updated with all data. */
AFL_TEST("game.map.IonStorm:addMessageInformation:max", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    int n;
    game::map::Point pt;

    game::map::IonStorm i(3);
    configureIonStorm(i);

    gp::MessageInformation info(gp::MessageInformation::IonStorm, i.getId(), 99);
    info.addValue(gp::mi_IonVoltage, 20);
    info.addValue(gp::mi_X, 1111);
    info.addValue(gp::mi_Y, 2222);
    info.addValue(gp::mi_Radius, 33);
    info.addValue(gp::mi_Heading, 44);
    info.addValue(gp::mi_WarpFactor, 7);
    info.addValue(gp::mi_IonStatus, 1);
    info.addValue(gp::ms_Name, "Wilma");
    i.addMessageInformation(info);

    a.checkEqual("01. getName",       i.getName(game::DetailedName, tx, iface), "Ion storm #3: Wilma");
    a.checkEqual("02. getPosition",   i.getPosition().get(pt), true);
    a.checkEqual("03. x",             pt.getX(), 1111);
    a.checkEqual("04. y",             pt.getY(), 2222);
    a.checkEqual("05. getRadius",     i.getRadius().get(n), true);
    a.checkEqual("06. radius",        n, 33);
    a.checkEqual("07. getClass",      i.getClass().orElse(-1), 1);
    a.checkEqual("08. getVoltage",    i.getVoltage().orElse(-1), 20);
    a.checkEqual("09. getHeading",    i.getHeading().orElse(-1), 44);
    a.checkEqual("10. getWarpFactor", i.getWarpFactor().orElse(-1), 7);
    a.checkEqual("11. isGrowing",     i.isGrowing(), true);
    a.checkEqual("12. isActive",      i.isActive(), true);
}

/** Test addMessageInformation, missing data case.
    A: call addMessageInformation with no y coordinate.
    E: ion storm not changed */
AFL_TEST("game.map.IonStorm:addMessageInformation:missing-y", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    int n;
    game::map::Point pt;

    game::map::IonStorm i(3);
    configureIonStorm(i);

    gp::MessageInformation info(gp::MessageInformation::IonStorm, i.getId(), 99);
    info.addValue(gp::mi_IonVoltage, 20);
    info.addValue(gp::mi_X, 900);
    info.addValue(gp::mi_Radius, 70);
    i.addMessageInformation(info);

    a.checkEqual("01. getName",       i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    a.checkEqual("02. getPosition",   i.getPosition().get(pt), true);
    a.checkEqual("03. x",             pt.getX(), 2001);
    a.checkEqual("04. y",             pt.getY(), 3014);
    a.checkEqual("05. getRadius",     i.getRadius().get(n), true);
    a.checkEqual("06. radius",        n, 40);
    a.checkEqual("07. getClass",      i.getClass().orElse(-1), 4);
    a.checkEqual("08. getVoltage",    i.getVoltage().orElse(-1), 180);
    a.checkEqual("09. getHeading",    i.getHeading().orElse(-1), 225);
    a.checkEqual("10. getWarpFactor", i.getWarpFactor().orElse(-1), 6);
    a.checkEqual("11. isGrowing",     i.isGrowing(), true);
    a.checkEqual("12. isActive",      i.isActive(), true);
}

/** Test getForecast(), empty data case.
    A: create empty ion storm. Call getForecast().
    E: empty forecast returned. */
AFL_TEST("game.map.IonStorm:getForecast:empty", a)
{
    game::map::IonStorm i(3);
    game::map::IonStorm::Forecast_t fs;
    i.getForecast(fs);
    a.checkEqual("size", fs.size(), 0U);
}

/** Test getForecast(), normal data case.
    A: create empty ion storm. Call getForecast().
    E: empty forecast returned. */
AFL_TEST("game.map.IonStorm:getForecast:normal", a)
{
    game::map::IonStorm i(3);
    configureIonStorm(i);

    game::map::IonStorm::Forecast_t fs;
    i.getForecast(fs);
    a.checkDifferent("01. size", fs.size(), 0U);

    // Start with some uncertain value
    a.checkDifferent("11. uncertainity", fs.front().uncertainity, 0);

    // End with certain value, matching current position
    a.checkEqual("21. uncertainity", fs.back().uncertainity, 0);
    a.checkEqual("22. x", fs.back().center.getX(), 2001);
    a.checkEqual("23. y", fs.back().center.getY(), 3014);
}
