/**
  *  \file u/t_game_interface_ufoproperty.cpp
  *  \brief Test for game::interface::UfoProperty
  */

#include "game/interface/ufoproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/nulltranslator.hpp"
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
void
TestGameInterfaceUfoProperty::testIt()
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
    verifyNewInteger("iupColorEGA",      getUfoProperty(ufo, game::interface::iupColorEGA,      tx, iface), 7);
    verifyNewInteger("iupColorPCC",      getUfoProperty(ufo, game::interface::iupColorPCC,      tx, iface), 2);
    verifyNewInteger("iupHeadingInt",    getUfoProperty(ufo, game::interface::iupHeadingInt,    tx, iface), 135);
    verifyNewString ("iupHeadingName",   getUfoProperty(ufo, game::interface::iupHeadingName,   tx, iface), "SE");
    verifyNewInteger("iupId",            getUfoProperty(ufo, game::interface::iupId,            tx, iface), 51);
    verifyNewInteger("iupId2",           getUfoProperty(ufo, game::interface::iupId2,           tx, iface), 9000);
    verifyNewString ("iupInfo1",         getUfoProperty(ufo, game::interface::iupInfo1,         tx, iface), "USS Rosswell");
    verifyNewString ("iupInfo2",         getUfoProperty(ufo, game::interface::iupInfo2,         tx, iface), "New Mexico");
    verifyNewBoolean("iupKeepFlag",      getUfoProperty(ufo, game::interface::iupKeepFlag,      tx, iface), false);
    verifyNewInteger("iupLastScan",      getUfoProperty(ufo, game::interface::iupLastScan,      tx, iface), 0);
    verifyNewInteger("iupLocX",          getUfoProperty(ufo, game::interface::iupLocX,          tx, iface), 1500);
    verifyNewInteger("iupLocY",          getUfoProperty(ufo, game::interface::iupLocY,          tx, iface), 1200);
    verifyNewBoolean("iupMarked",        getUfoProperty(ufo, game::interface::iupMarked,        tx, iface), false);
    verifyNewInteger("iupMoveDX",        getUfoProperty(ufo, game::interface::iupMoveDX,        tx, iface), -4);
    verifyNewInteger("iupMoveDY",        getUfoProperty(ufo, game::interface::iupMoveDY,        tx, iface), 4);
    verifyNewString ("iupName",          getUfoProperty(ufo, game::interface::iupName,          tx, iface), "Secret");
    verifyNewInteger("iupRadius",        getUfoProperty(ufo, game::interface::iupRadius,        tx, iface), 12);
    verifyNewInteger("iupSpeedInt",      getUfoProperty(ufo, game::interface::iupSpeedInt,      tx, iface), 2);
    verifyNewString ("iupSpeedName",     getUfoProperty(ufo, game::interface::iupSpeedName,     tx, iface), "Warp 2");
    verifyNewInteger("iupType",          getUfoProperty(ufo, game::interface::iupType,          tx, iface), 2000);
    verifyNewInteger("iupVisiblePlanet", getUfoProperty(ufo, game::interface::iupVisiblePlanet, tx, iface), 200);
    verifyNewInteger("iupVisibleShip",   getUfoProperty(ufo, game::interface::iupVisibleShip,   tx, iface), 150);

    // Changeable properties
    {
        afl::data::IntegerValue iv(1);
        setUfoProperty(ufo, game::interface::iupMoveDX, &iv);
        TS_ASSERT_EQUALS(ufo.getMovementVector(), game::map::Point(1, 4));
    }
    {
        afl::data::IntegerValue iv(5);
        setUfoProperty(ufo, game::interface::iupMoveDY, &iv);
        TS_ASSERT_EQUALS(ufo.getMovementVector(), game::map::Point(1, 5));
    }
    {
        afl::data::IntegerValue iv(777);
        setUfoProperty(ufo, game::interface::iupId2, &iv);
        TS_ASSERT_EQUALS(ufo.getRealId(), 777);
    }
    {
        afl::data::BooleanValue bv(true);
        setUfoProperty(ufo, game::interface::iupKeepFlag, &bv);
        TS_ASSERT_EQUALS(ufo.isStoredInHistory(), true);
    }

    // Out of range
    {

        afl::data::IntegerValue iv(10000);
        TS_ASSERT_THROWS(setUfoProperty(ufo, game::interface::iupMoveDX, &iv), interpreter::Error);
    }

    // Type error
    {
        afl::data::StringValue sv("X");
        TS_ASSERT_THROWS(setUfoProperty(ufo, game::interface::iupMoveDX, &sv), interpreter::Error);
    }

    // Not assignable
    {
        afl::data::IntegerValue iv(100);
        TS_ASSERT_THROWS(setUfoProperty(ufo, game::interface::iupId, &iv), interpreter::Error);
    }
}

/** Test properties of empty Ufo. */
void
TestGameInterfaceUfoProperty::testEmpty()
{
    // Environment
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;

    // Make an Ufo
    game::map::Ufo ufo(51);

    // Verify properties
    // This documents some "fields are empty" vs. "fields have default value" choices that are pretty arbitrary and could in principle change.
    verifyNewInteger("iupColorEGA",      getUfoProperty(ufo, game::interface::iupColorEGA,      tx, iface), 0);
    verifyNewInteger("iupColorPCC",      getUfoProperty(ufo, game::interface::iupColorPCC,      tx, iface), 0);
    verifyNewNull   ("iupHeadingInt",    getUfoProperty(ufo, game::interface::iupHeadingInt,    tx, iface));
    verifyNewNull   ("iupHeadingName",   getUfoProperty(ufo, game::interface::iupHeadingName,   tx, iface));
    verifyNewInteger("iupId",            getUfoProperty(ufo, game::interface::iupId,            tx, iface), 51);
    verifyNewInteger("iupId2",           getUfoProperty(ufo, game::interface::iupId2,           tx, iface), 0);
    verifyNewString ("iupInfo1",         getUfoProperty(ufo, game::interface::iupInfo1,         tx, iface), "");
    verifyNewString ("iupInfo2",         getUfoProperty(ufo, game::interface::iupInfo2,         tx, iface), "");
    verifyNewBoolean("iupKeepFlag",      getUfoProperty(ufo, game::interface::iupKeepFlag,      tx, iface), false);
    verifyNewInteger("iupLastScan",      getUfoProperty(ufo, game::interface::iupLastScan,      tx, iface), 0);
    verifyNewNull   ("iupLocX",          getUfoProperty(ufo, game::interface::iupLocX,          tx, iface));
    verifyNewNull   ("iupLocY",          getUfoProperty(ufo, game::interface::iupLocY,          tx, iface));
    verifyNewBoolean("iupMarked",        getUfoProperty(ufo, game::interface::iupMarked,        tx, iface), false);
    verifyNewInteger("iupMoveDX",        getUfoProperty(ufo, game::interface::iupMoveDX,        tx, iface), 0);
    verifyNewInteger("iupMoveDY",        getUfoProperty(ufo, game::interface::iupMoveDY,        tx, iface), 0);
    verifyNewString ("iupName",          getUfoProperty(ufo, game::interface::iupName,          tx, iface), "");
    verifyNewNull   ("iupRadius",        getUfoProperty(ufo, game::interface::iupRadius,        tx, iface));
    verifyNewNull   ("iupSpeedInt",      getUfoProperty(ufo, game::interface::iupSpeedInt,      tx, iface));
    verifyNewNull   ("iupSpeedName",     getUfoProperty(ufo, game::interface::iupSpeedName,     tx, iface));
    verifyNewNull   ("iupType",          getUfoProperty(ufo, game::interface::iupType,          tx, iface));
    verifyNewNull   ("iupVisiblePlanet", getUfoProperty(ufo, game::interface::iupVisiblePlanet, tx, iface));
    verifyNewNull   ("iupVisibleShip",   getUfoProperty(ufo, game::interface::iupVisibleShip,   tx, iface));
}

