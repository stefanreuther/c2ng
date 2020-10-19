/**
  *  \file u/t_game_map_ufo.cpp
  *  \brief Test for game::map::Ufo
  */

#include "game/map/ufo.hpp"

#include "t_game_map.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/interpreterinterface.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/parser/messagevalue.hpp"
#include "game/map/configuration.hpp"

/** Simple accessor test. */
void
TestGameMapUfo::testAccessor()
{
    game::map::Point pt;
    int owner;
    int radius;
    int32_t radius2;

    // Verify initial state
    game::map::Ufo t(77);
    TS_ASSERT_EQUALS(t.getId(), 77);
    TS_ASSERT_EQUALS(t.isStoredInHistory(), false);
    TS_ASSERT_EQUALS(t.isSeenThisTurn(), false);
    TS_ASSERT_EQUALS(t.isValid(), false);
    TS_ASSERT_EQUALS(t.getPosition(pt), false);
    TS_ASSERT_EQUALS(t.getRadius(radius), false);
    TS_ASSERT_EQUALS(t.getRadiusSquared(radius2), false);

    TS_ASSERT_EQUALS(t.getOwner(owner), true);
    TS_ASSERT_EQUALS(owner, 0);

    // Set it
    t.setColorCode(3);
    t.setSpeed(7);
    t.setHeading(240);
    t.setPlanetRange(200);
    t.setShipRange(150);
    t.setTypeCode(1200);
    t.setInfo1("top");
    t.setInfo2("bottom");
    t.setRealId(4711);
    t.setName("Area 51");
    t.setPosition(game::map::Point(1000, 1400));
    t.setRadius(25);

    // Verify
    game::test::InterpreterInterface iface;
    afl::string::NullTranslator tx;
    TS_ASSERT_EQUALS(t.getColorCode(), 3);
    TS_ASSERT_EQUALS(t.getSpeed().orElse(-1), 7);
    TS_ASSERT_EQUALS(t.getHeading().orElse(-1), 240);
    TS_ASSERT_EQUALS(t.getPlanetRange().orElse(-1), 200);
    TS_ASSERT_EQUALS(t.getShipRange().orElse(-1), 150);
    TS_ASSERT_EQUALS(t.getTypeCode().orElse(-1), 1200);
    TS_ASSERT_EQUALS(t.getInfo1(), "top");
    TS_ASSERT_EQUALS(t.getInfo2(), "bottom");
    TS_ASSERT_EQUALS(t.getRealId(),4711);
    TS_ASSERT_EQUALS(t.getPlainName(), "Area 51");
    TS_ASSERT_EQUALS(t.getName(game::PlainName, tx, iface), "Area 51");
    TS_ASSERT_EQUALS(t.getName(game::LongName, tx, iface), "Ufo #77: Area 51");
    TS_ASSERT_EQUALS(t.getPosition(pt), true);
    TS_ASSERT_EQUALS(pt.getX(), 1000);
    TS_ASSERT_EQUALS(pt.getY(), 1400);

    TS_ASSERT_EQUALS(t.getRadius(radius), true);
    TS_ASSERT_EQUALS(radius, 25);

    TS_ASSERT_EQUALS(t.getRadiusSquared(radius2), true);
    TS_ASSERT_EQUALS(radius2, 625);
}

/** Test connect/disconnect. */
void
TestGameMapUfo::testConnect()
{
    game::map::Ufo u1(1), u2(2), u3(3);
    game::map::Ufo* null = 0;

    // Initial state
    TS_ASSERT_EQUALS(u1.getOtherEnd(), null);
    TS_ASSERT_EQUALS(u2.getOtherEnd(), null);
    TS_ASSERT_EQUALS(u3.getOtherEnd(), null);

    // Connect
    u1.connectWith(u2);
    TS_ASSERT_EQUALS(u1.getOtherEnd(), &u2);
    TS_ASSERT_EQUALS(u2.getOtherEnd(), &u1);
    TS_ASSERT_EQUALS(u3.getOtherEnd(), null);

    // Reconnect
    u2.connectWith(u3);
    TS_ASSERT_EQUALS(u1.getOtherEnd(), null);
    TS_ASSERT_EQUALS(u2.getOtherEnd(), &u3);
    TS_ASSERT_EQUALS(u3.getOtherEnd(), &u2);

    // Disconnect
    u3.disconnect();
    TS_ASSERT_EQUALS(u1.getOtherEnd(), null);
    TS_ASSERT_EQUALS(u2.getOtherEnd(), null);
    TS_ASSERT_EQUALS(u3.getOtherEnd(), null);
}

void
TestGameMapUfo::testMovementPrediction()
{
    namespace gp = game::parser;
    game::map::Ufo testee(10);
    game::map::Configuration mapConfig;

    // Scan ufo in turn 5
    gp::MessageInformation info(gp::MessageInformation::Ufo, 10, 5);
    info.addValue(gp::mi_Type, 33);
    info.addValue(gp::mi_UfoColor, 7);
    info.addValue(gp::mi_X, 1000);
    info.addValue(gp::mi_Y, 2000);
    testee.addMessageInformation(info);

    // Guess movement 7 turns later
    testee.setMovementVector(game::map::Point(4, 5));
    testee.postprocess(12, mapConfig);

    game::map::Point pt;
    TS_ASSERT_EQUALS(testee.getPosition(pt), true);

    TS_ASSERT_EQUALS(pt.getX(), 1000 + 4*7);
    TS_ASSERT_EQUALS(pt.getY(), 2000 + 5*7);
}

