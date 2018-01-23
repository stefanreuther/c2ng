/**
  *  \file u/t_game_map_ionstorm.cpp
  *  \brief Test for game::map::IonStorm
  */

#include "game/map/ionstorm.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/interpreterinterface.hpp"
#include "game/test/interpreterinterface.hpp"

/** Simple setter/getter test. */
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
    TS_ASSERT_EQUALS(i.getName(game::map::Object::PlainName, tx, iface), "Ion storm #3");
    TS_ASSERT_EQUALS(i.getId(), 3);
    TS_ASSERT_EQUALS(i.getOwner(n), true);
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(i.getPosition(pt), false);
    TS_ASSERT_EQUALS(i.getRadius(n), false);
    TS_ASSERT_EQUALS(i.getRadiusSquared(d), false);
    TS_ASSERT_EQUALS(i.getClass().get(n), false);
    TS_ASSERT_EQUALS(i.getVoltage().get(n), false);
    TS_ASSERT_EQUALS(i.getHeading().get(n), false);
    TS_ASSERT_EQUALS(i.getSpeed().get(n), false);
    TS_ASSERT(!i.isGrowing());
    TS_ASSERT(!i.isActive());

    // Populate it
    i.setName("Klothilde");
    i.setPosition(game::map::Point(2001, 3014));
    i.setRadius(40);
    i.setVoltage(180);
    i.setSpeed(6);
    i.setHeading(225);
    i.setIsGrowing(true);

    // Verify
    TS_ASSERT_EQUALS(i.getName(game::map::Object::PlainName, tx, iface), "Klothilde");
    TS_ASSERT_EQUALS(i.getName(game::map::Object::LongName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getName(game::map::Object::DetailedName, tx, iface), "Ion storm #3: Klothilde");
    TS_ASSERT_EQUALS(i.getId(), 3);
    TS_ASSERT_EQUALS(i.getOwner(n), true);
    TS_ASSERT_EQUALS(n, 0);
    TS_ASSERT_EQUALS(i.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 2001);
    TS_ASSERT_EQUALS(pt.getY(), 3014);
    TS_ASSERT_EQUALS(i.getRadius(n), true);
    TS_ASSERT_EQUALS(n, 40);
    TS_ASSERT_EQUALS(i.getRadiusSquared(d), true);
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

