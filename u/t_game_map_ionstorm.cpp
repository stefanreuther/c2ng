/**
  *  \file u/t_game_map_ionstorm.cpp
  *  \brief Test for game::map::IonStorm
  */

#include "game/map/ionstorm.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
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
        i.setSpeed(6);
        i.setHeading(225);
        i.setIsGrowing(true);
    }
}

/** Simple setter/getter test.
    A: create an ion storm. Use setters/getters.
    E: expected initial/stored values returned */
void
TestGameMapIonStorm::testIt()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    int n;
    game::map::Point pt;
    int32_t d;

    // Test initial state
    game::map::IonStorm i(3);
    TS_ASSERT_EQUALS(i.getName(game::PlainName, tx, iface), "Ion storm #3");
    TS_ASSERT_EQUALS(i.getId(), 3);
    TS_ASSERT_EQUALS(i.getOwner().get(n), true);
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(i.getPosition().get(pt), false);
    TS_ASSERT_EQUALS(i.getRadius().get(n), false);
    TS_ASSERT_EQUALS(i.getRadiusSquared().get(d), false);
    TS_ASSERT_EQUALS(i.getClass().get(n), false);
    TS_ASSERT_EQUALS(i.getVoltage().get(n), false);
    TS_ASSERT_EQUALS(i.getHeading().get(n), false);
    TS_ASSERT_EQUALS(i.getSpeed().get(n), false);
    TS_ASSERT(!i.isGrowing());
    TS_ASSERT(!i.isActive());

    // Populate it
    configureIonStorm(i);

    // Verify
    TS_ASSERT_EQUALS(i.getName(game::PlainName, tx, iface), "Klothilde");
    TS_ASSERT_EQUALS(i.getName(game::LongName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getId(), 3);
    TS_ASSERT_EQUALS(i.getOwner().get(n), true);
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(i.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 2001);
    TS_ASSERT_EQUALS(pt.getY(), 3014);
    TS_ASSERT_EQUALS(i.getRadius().get(n), true);
    TS_ASSERT_EQUALS(n, 40);
    TS_ASSERT_EQUALS(i.getRadiusSquared().get(d), true);
    TS_ASSERT_EQUALS(d, 1600);
    TS_ASSERT_EQUALS(i.getClass().get(n), true);
    TS_ASSERT_EQUALS(n, 4);
    TS_ASSERT_EQUALS(i.getVoltage().get(n), true);
    TS_ASSERT_EQUALS(n, 180);
    TS_ASSERT_EQUALS(i.getHeading().get(n), true);
    TS_ASSERT_EQUALS(n, 225);
    TS_ASSERT_EQUALS(i.getSpeed().get(n), true);
    TS_ASSERT_EQUALS(n, 6);
    TS_ASSERT(i.isGrowing());
    TS_ASSERT(i.isActive());
}

/** Test addMessageInformation to clear a storm.
    A: call addMessageInformation with voltage=0
    E: ion storm no longer active */
void
TestGameMapIonStorm::testMessageInfoClear()
{
    game::map::IonStorm i(3);
    configureIonStorm(i);

    gp::MessageInformation info(gp::MessageInformation::IonStorm, i.getId(), 99);
    info.addValue(gp::mi_IonVoltage, 0);
    i.addMessageInformation(info);

    TS_ASSERT(!i.isActive());
}

/** Test addMessageInformation, minimum data case.
    A: call addMessageInformation with voltage, x, y, radius.
    E: ion storm updated, old data kept */
void
TestGameMapIonStorm::testMessageInfoMin()
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

    TS_ASSERT_EQUALS(i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 900);
    TS_ASSERT_EQUALS(pt.getY(), 1800);
    TS_ASSERT_EQUALS(i.getRadius().get(n), true);
    TS_ASSERT_EQUALS(n, 70);
    TS_ASSERT_EQUALS(i.getClass().orElse(-1), 1);
    TS_ASSERT_EQUALS(i.getVoltage().orElse(-1), 20);
    TS_ASSERT_EQUALS(i.getHeading().orElse(-1), 225);
    TS_ASSERT_EQUALS(i.getSpeed().orElse(-1), 6);
    TS_ASSERT_EQUALS(i.isGrowing(), false);
    TS_ASSERT_EQUALS(i.isActive(), true);
}

/** Test addMessageInformation, maximum data case.
    A: call addMessageInformation with all data.
    E: ion storm updated with all data. */
void
TestGameMapIonStorm::testMessageInfoMax()
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
    info.addValue(gp::mi_Speed, 7);
    info.addValue(gp::mi_IonStatus, 1);
    info.addValue(gp::ms_Name, "Wilma");
    i.addMessageInformation(info);

    TS_ASSERT_EQUALS(i.getName(game::DetailedName, tx, iface), "Ion storm #3: Wilma");
    TS_ASSERT_EQUALS(i.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 1111);
    TS_ASSERT_EQUALS(pt.getY(), 2222);
    TS_ASSERT_EQUALS(i.getRadius().get(n), true);
    TS_ASSERT_EQUALS(n, 33);
    TS_ASSERT_EQUALS(i.getClass().orElse(-1), 1);
    TS_ASSERT_EQUALS(i.getVoltage().orElse(-1), 20);
    TS_ASSERT_EQUALS(i.getHeading().orElse(-1), 44);
    TS_ASSERT_EQUALS(i.getSpeed().orElse(-1), 7);
    TS_ASSERT_EQUALS(i.isGrowing(), true);
    TS_ASSERT_EQUALS(i.isActive(), true);
}

/** Test addMessageInformation, missing data case.
    A: call addMessageInformation with no y coordinate.
    E: ion storm not changed */
void
TestGameMapIonStorm::testMessageInfoMissing()
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

    TS_ASSERT_EQUALS(i.getName(game::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getPosition().get(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 2001);
    TS_ASSERT_EQUALS(pt.getY(), 3014);
    TS_ASSERT_EQUALS(i.getRadius().get(n), true);
    TS_ASSERT_EQUALS(n, 40);
    TS_ASSERT_EQUALS(i.getClass().orElse(-1), 4);
    TS_ASSERT_EQUALS(i.getVoltage().orElse(-1), 180);
    TS_ASSERT_EQUALS(i.getHeading().orElse(-1), 225);
    TS_ASSERT_EQUALS(i.getSpeed().orElse(-1), 6);
    TS_ASSERT_EQUALS(i.isGrowing(), true);
    TS_ASSERT_EQUALS(i.isActive(), true);
}

/** Test getForecast(), empty data case.
    A: create empty ion storm. Call getForecast().
    E: empty forecast returned. */
void
TestGameMapIonStorm::testForecastEmpty()
{
    game::map::IonStorm i(3);
    game::map::IonStorm::Forecast_t fs;
    i.getForecast(fs);
    TS_ASSERT_EQUALS(fs.size(), 0U);
}

/** Test getForecast(), normal data case.
    A: create empty ion storm. Call getForecast().
    E: empty forecast returned. */
void
TestGameMapIonStorm::testForecastNormal()
{
    game::map::IonStorm i(3);
    configureIonStorm(i);

    game::map::IonStorm::Forecast_t fs;
    i.getForecast(fs);
    TS_ASSERT_DIFFERS(fs.size(), 0U);

    // Start with some uncertain value
    TS_ASSERT_DIFFERS(fs.front().uncertainity, 0);

    // End with certain value, matching current position
    TS_ASSERT_EQUALS(fs.back().uncertainity, 0);
    TS_ASSERT_EQUALS(fs.back().center.getX(), 2001);
    TS_ASSERT_EQUALS(fs.back().center.getY(), 3014);
}

