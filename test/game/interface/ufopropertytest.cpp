/**
  *  \file test/game/interface/ufopropertytest.cpp
  *  \brief Test for game::interface::UfoProperty
  */

#include "game/interface/ufoproperty.hpp"

#include "afl/data/booleanvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"
#include "game/map/ufo.hpp"
#include "game/test/interpreterinterface.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** Test general properties. */
AFL_TEST("game.interface.UfoProperty:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    // Make an Ufo
    game::map::Ufo ufo(51);
    ufo.setColorCode(7);
    ufo.setWarpFactor(2);
    ufo.setHeading(135);
    ufo.setPlanetRange(200);
    ufo.setShipRange(150);
    ufo.setTypeCode(2000);
    ufo.setRealId(9000);
    ufo.setPosition(game::map::Point(1500, 1200));
    ufo.setRadius(12);
    ufo.setMovementVector(game::map::Point(-4, 4));
    ufo.setName("Secret");
    ufo.setInfo1("USS Rosswell");
    ufo.setInfo2("New Mexico");
    ufo.postprocess(42, game::map::Configuration());

    // Verify properties
    verifyNewInteger(a("iupColorEGA"),      getUfoProperty(ufo, game::interface::iupColorEGA,      tx, iface), 7);
    verifyNewInteger(a("iupColorPCC"),      getUfoProperty(ufo, game::interface::iupColorPCC,      tx, iface), 2);
    verifyNewInteger(a("iupHeadingInt"),    getUfoProperty(ufo, game::interface::iupHeadingInt,    tx, iface), 135);
    verifyNewString (a("iupHeadingName"),   getUfoProperty(ufo, game::interface::iupHeadingName,   tx, iface), "SE");
    verifyNewInteger(a("iupId"),            getUfoProperty(ufo, game::interface::iupId,            tx, iface), 51);
    verifyNewInteger(a("iupId2"),           getUfoProperty(ufo, game::interface::iupId2,           tx, iface), 9000);
    verifyNewString (a("iupInfo1"),         getUfoProperty(ufo, game::interface::iupInfo1,         tx, iface), "USS Rosswell");
    verifyNewString (a("iupInfo2"),         getUfoProperty(ufo, game::interface::iupInfo2,         tx, iface), "New Mexico");
    verifyNewBoolean(a("iupKeepFlag"),      getUfoProperty(ufo, game::interface::iupKeepFlag,      tx, iface), false);
    verifyNewInteger(a("iupLastScan"),      getUfoProperty(ufo, game::interface::iupLastScan,      tx, iface), 0);
    verifyNewInteger(a("iupLocX"),          getUfoProperty(ufo, game::interface::iupLocX,          tx, iface), 1500);
    verifyNewInteger(a("iupLocY"),          getUfoProperty(ufo, game::interface::iupLocY,          tx, iface), 1200);
    verifyNewBoolean(a("iupMarked"),        getUfoProperty(ufo, game::interface::iupMarked,        tx, iface), false);
    verifyNewInteger(a("iupMoveDX"),        getUfoProperty(ufo, game::interface::iupMoveDX,        tx, iface), -4);
    verifyNewInteger(a("iupMoveDY"),        getUfoProperty(ufo, game::interface::iupMoveDY,        tx, iface), 4);
    verifyNewString (a("iupName"),          getUfoProperty(ufo, game::interface::iupName,          tx, iface), "Secret");
    verifyNewInteger(a("iupRadius"),        getUfoProperty(ufo, game::interface::iupRadius,        tx, iface), 12);
    verifyNewInteger(a("iupSpeedInt"),      getUfoProperty(ufo, game::interface::iupSpeedInt,      tx, iface), 2);
    verifyNewString (a("iupSpeedName"),     getUfoProperty(ufo, game::interface::iupSpeedName,     tx, iface), "Warp 2");
    verifyNewInteger(a("iupType"),          getUfoProperty(ufo, game::interface::iupType,          tx, iface), 2000);
    verifyNewInteger(a("iupVisiblePlanet"), getUfoProperty(ufo, game::interface::iupVisiblePlanet, tx, iface), 200);
    verifyNewInteger(a("iupVisibleShip"),   getUfoProperty(ufo, game::interface::iupVisibleShip,   tx, iface), 150);

    // Changeable properties
    {
        afl::data::IntegerValue iv(1);
        setUfoProperty(ufo, game::interface::iupMoveDX, &iv);
        a.checkEqual("01. getMovementVector", ufo.getMovementVector(), game::map::Point(1, 4));
    }
    {
        afl::data::IntegerValue iv(5);
        setUfoProperty(ufo, game::interface::iupMoveDY, &iv);
        a.checkEqual("02. getMovementVector", ufo.getMovementVector(), game::map::Point(1, 5));
    }
    {
        afl::data::IntegerValue iv(777);
        setUfoProperty(ufo, game::interface::iupId2, &iv);
        a.checkEqual("03. getRealId", ufo.getRealId(), 777);
    }
    {
        afl::data::BooleanValue bv(true);
        setUfoProperty(ufo, game::interface::iupKeepFlag, &bv);
        a.checkEqual("04. isStoredInHistory", ufo.isStoredInHistory(), true);
    }

    // Out of range
    {

        afl::data::IntegerValue iv(10000);
        AFL_CHECK_THROWS(a("11. range error"), setUfoProperty(ufo, game::interface::iupMoveDX, &iv), interpreter::Error);
    }

    // Type error
    {
        afl::data::StringValue sv("X");
        AFL_CHECK_THROWS(a("21. type error"), setUfoProperty(ufo, game::interface::iupMoveDX, &sv), interpreter::Error);
    }

    // Not assignable
    {
        afl::data::IntegerValue iv(100);
        AFL_CHECK_THROWS(a("31. not assignable"), setUfoProperty(ufo, game::interface::iupId, &iv), interpreter::Error);
    }
}

/** Test properties of empty Ufo. */
AFL_TEST("game.interface.UfoProperty:empty", a)
{
    // Environment
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    // Make an Ufo
    game::map::Ufo ufo(51);

    // Verify properties
    // This documents some "fields are empty" vs. "fields have default value" choices that are pretty arbitrary and could in principle change.
    verifyNewInteger(a("iupColorEGA"),      getUfoProperty(ufo, game::interface::iupColorEGA,      tx, iface), 0);
    verifyNewInteger(a("iupColorPCC"),      getUfoProperty(ufo, game::interface::iupColorPCC,      tx, iface), 0);
    verifyNewNull   (a("iupHeadingInt"),    getUfoProperty(ufo, game::interface::iupHeadingInt,    tx, iface));
    verifyNewNull   (a("iupHeadingName"),   getUfoProperty(ufo, game::interface::iupHeadingName,   tx, iface));
    verifyNewInteger(a("iupId"),            getUfoProperty(ufo, game::interface::iupId,            tx, iface), 51);
    verifyNewInteger(a("iupId2"),           getUfoProperty(ufo, game::interface::iupId2,           tx, iface), 0);
    verifyNewString (a("iupInfo1"),         getUfoProperty(ufo, game::interface::iupInfo1,         tx, iface), "");
    verifyNewString (a("iupInfo2"),         getUfoProperty(ufo, game::interface::iupInfo2,         tx, iface), "");
    verifyNewBoolean(a("iupKeepFlag"),      getUfoProperty(ufo, game::interface::iupKeepFlag,      tx, iface), false);
    verifyNewInteger(a("iupLastScan"),      getUfoProperty(ufo, game::interface::iupLastScan,      tx, iface), 0);
    verifyNewNull   (a("iupLocX"),          getUfoProperty(ufo, game::interface::iupLocX,          tx, iface));
    verifyNewNull   (a("iupLocY"),          getUfoProperty(ufo, game::interface::iupLocY,          tx, iface));
    verifyNewBoolean(a("iupMarked"),        getUfoProperty(ufo, game::interface::iupMarked,        tx, iface), false);
    verifyNewInteger(a("iupMoveDX"),        getUfoProperty(ufo, game::interface::iupMoveDX,        tx, iface), 0);
    verifyNewInteger(a("iupMoveDY"),        getUfoProperty(ufo, game::interface::iupMoveDY,        tx, iface), 0);
    verifyNewString (a("iupName"),          getUfoProperty(ufo, game::interface::iupName,          tx, iface), "");
    verifyNewNull   (a("iupRadius"),        getUfoProperty(ufo, game::interface::iupRadius,        tx, iface));
    verifyNewNull   (a("iupSpeedInt"),      getUfoProperty(ufo, game::interface::iupSpeedInt,      tx, iface));
    verifyNewNull   (a("iupSpeedName"),     getUfoProperty(ufo, game::interface::iupSpeedName,     tx, iface));
    verifyNewNull   (a("iupType"),          getUfoProperty(ufo, game::interface::iupType,          tx, iface));
    verifyNewNull   (a("iupVisiblePlanet"), getUfoProperty(ufo, game::interface::iupVisiblePlanet, tx, iface));
    verifyNewNull   (a("iupVisibleShip"),   getUfoProperty(ufo, game::interface::iupVisibleShip,   tx, iface));
}
