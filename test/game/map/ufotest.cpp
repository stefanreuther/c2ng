/**
  *  \file test/game/map/ufotest.cpp
  *  \brief Test for game::map::Ufo
  */

#include "game/map/ufo.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/parser/messageinformation.hpp"
#include "game/parser/messagevalue.hpp"
#include "game/test/interpreterinterface.hpp"

/** Simple accessor test. */
AFL_TEST("game.map.Ufo:accessor", a)
{
    game::map::Point pt;
    int owner;
    int radius;
    int32_t radius2;

    // Verify initial state
    game::map::Ufo t(77);
    a.checkEqual("01. getId", t.getId(), 77);
    a.checkEqual("02. isStoredInHistory", t.isStoredInHistory(), false);
    a.checkEqual("03. isSeenThisTurn", t.isSeenThisTurn(), false);
    a.checkEqual("04. isValid", t.isValid(), false);
    a.checkEqual("05. getPosition", t.getPosition().get(pt), false);
    a.checkEqual("06. getRadius", t.getRadius().get(radius), false);
    a.checkEqual("07. getRadiusSquared", t.getRadiusSquared().get(radius2), false);

    a.checkEqual("11. getOwner", t.getOwner().get(owner), true);
    a.checkEqual("12. owner", owner, 0);

    // Set it
    t.setColorCode(3);
    t.setWarpFactor(7);
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
    a.checkEqual("21. getColorCode",   t.getColorCode(), 3);
    a.checkEqual("22. getWarpFactor",  t.getWarpFactor().orElse(-1), 7);
    a.checkEqual("23. getHeading",     t.getHeading().orElse(-1), 240);
    a.checkEqual("24. getPlanetRange", t.getPlanetRange().orElse(-1), 200);
    a.checkEqual("25. getShipRange",   t.getShipRange().orElse(-1), 150);
    a.checkEqual("26. getTypeCode",    t.getTypeCode().orElse(-1), 1200);
    a.checkEqual("27. getInfo1",       t.getInfo1(), "top");
    a.checkEqual("28. getInfo2",       t.getInfo2(), "bottom");
    a.checkEqual("29. getRealId",      t.getRealId(),4711);
    a.checkEqual("30. getPlainName",   t.getPlainName(), "Area 51");
    a.checkEqual("31. getName",        t.getName(game::PlainName, tx, iface), "Area 51");
    a.checkEqual("32. getName",        t.getName(game::LongName, tx, iface), "Ufo #77: Area 51");
    a.checkEqual("33. getPosition",    t.getPosition().get(pt), true);
    a.checkEqual("34. getX", pt.getX(), 1000);
    a.checkEqual("35. getY", pt.getY(), 1400);

    a.checkEqual("41. getRadius", t.getRadius().get(radius), true);
    a.checkEqual("42. radius", radius, 25);

    a.checkEqual("51. getRadiusSquared", t.getRadiusSquared().get(radius2), true);
    a.checkEqual("52. radius2", radius2, 625);
}

/** Test connect/disconnect. */
AFL_TEST("game.map.Ufo:connect", a)
{
    game::map::Ufo u1(1), u2(2), u3(3);
    game::map::Ufo* null = 0;

    // Initial state
    a.checkEqual("01", u1.getOtherEnd(), null);
    a.checkEqual("02", u2.getOtherEnd(), null);
    a.checkEqual("03", u3.getOtherEnd(), null);

    // Connect
    u1.connectWith(u2);
    a.checkEqual("11", u1.getOtherEnd(), &u2);
    a.checkEqual("12", u2.getOtherEnd(), &u1);
    a.checkEqual("13", u3.getOtherEnd(), null);

    // Reconnect
    u2.connectWith(u3);
    a.checkEqual("21", u1.getOtherEnd(), null);
    a.checkEqual("22", u2.getOtherEnd(), &u3);
    a.checkEqual("23", u3.getOtherEnd(), &u2);

    // Disconnect
    u3.disconnect();
    a.checkEqual("31", u1.getOtherEnd(), null);
    a.checkEqual("32", u2.getOtherEnd(), null);
    a.checkEqual("33", u3.getOtherEnd(), null);
}

AFL_TEST("game.map.Ufo:setMovementVector", a)
{
    namespace gp = game::parser;
    game::map::Ufo testee(10);
    game::map::Configuration mapConfig;

    // Scan ufo in turn 5
    gp::MessageInformation info(gp::MessageInformation::Ufo, 10, 5);
    info.addValue(gp::mi_Type, 33);
    info.addValue(gp::mi_Color, 7);
    info.addValue(gp::mi_X, 1000);
    info.addValue(gp::mi_Y, 2000);
    testee.addMessageInformation(info);

    // Guess movement 7 turns later
    testee.setMovementVector(game::map::Point(4, 5));
    testee.postprocess(12, mapConfig);

    game::map::Point pt;
    a.checkEqual("01. getPosition", testee.getPosition().get(pt), true);

    a.checkEqual("11. getX", pt.getX(), 1000 + 4*7);
    a.checkEqual("12. getY", pt.getY(), 2000 + 5*7);
}
