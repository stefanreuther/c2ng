/**
  *  \file u/t_game_interface_ionstormproperty.cpp
  *  \brief Test for game::interface::IonStormProperty
  */

#include "game/interface/ionstormproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/test/interpreterinterface.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** Test property retrieval, fully populated storm. */
void
TestGameInterfaceIonStormProperty::testGet()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(42);
    storm.setName("Kyrill");
    storm.setPosition(game::map::Point(4503, 1701));
    storm.setRadius(20);
    storm.setVoltage(40);
    storm.setWarpFactor(4);
    storm.setHeading(70);
    storm.setIsGrowing(true);

    verifyNewInteger("iipClass",       getIonStormProperty(storm, game::interface::iipClass,       tx, iface), 1);
    verifyNewInteger("iipHeadingInt",  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface), 70);
    verifyNewString ("iipHeadingName", getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface), "ENE");
    verifyNewInteger("iipId",          getIonStormProperty(storm, game::interface::iipId,          tx, iface), 42);
    verifyNewInteger("iipLocX",        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface), 4503);
    verifyNewInteger("iipLocY",        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface), 1701);
    verifyNewBoolean("iipMarked",      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface), false);
    verifyNewString ("iipName",        getIonStormProperty(storm, game::interface::iipName,        tx, iface), "Kyrill");
    verifyNewInteger("iipRadius",      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface), 20);
    verifyNewInteger("iipSpeedInt",    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface), 4);
    verifyNewString ("iipSpeedName",   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface), "Warp 4");
    verifyNewBoolean("iipStatusFlag",  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface), true);
    verifyNewString ("iipStatusName",  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface), "Growing");
    verifyNewInteger("iipVoltage",     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface), 40);
}

/** Test property retrieval, empty storm.
    An empty (inactive, invisible) storm reports all properties as empty. */
void
TestGameInterfaceIonStormProperty::testGetEmpty()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(17);
    TS_ASSERT(!storm.isActive());

    verifyNewNull("iipClass",       getIonStormProperty(storm, game::interface::iipClass,       tx, iface));
    verifyNewNull("iipHeadingInt",  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface));
    verifyNewNull("iipHeadingName", getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface));
    verifyNewNull("iipId",          getIonStormProperty(storm, game::interface::iipId,          tx, iface));
    verifyNewNull("iipLocX",        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface));
    verifyNewNull("iipLocY",        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface));
    verifyNewNull("iipMarked",      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface));
    verifyNewNull("iipName",        getIonStormProperty(storm, game::interface::iipName,        tx, iface));
    verifyNewNull("iipRadius",      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface));
    verifyNewNull("iipSpeedInt",    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface));
    verifyNewNull("iipSpeedName",   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface));
    verifyNewNull("iipStatusFlag",  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface));
    verifyNewNull("iipStatusName",  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface));
    verifyNewNull("iipVoltage",     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface));
}

/** Test property retrieval, mostly empty storm.
    Most properties are nullable and return empty if never set. */
void
TestGameInterfaceIonStormProperty::testGetMostlyEmpty()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active
    storm.setIsMarked(true);
    TS_ASSERT(storm.isActive());

    verifyNewInteger("iipClass",       getIonStormProperty(storm, game::interface::iipClass,       tx, iface), 3);
    verifyNewNull   ("iipHeadingInt",  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface));
    verifyNewNull   ("iipHeadingName", getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface));
    verifyNewInteger("iipId",          getIonStormProperty(storm, game::interface::iipId,          tx, iface), 23);
    verifyNewNull   ("iipLocX",        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface));
    verifyNewNull   ("iipLocY",        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface));
    verifyNewBoolean("iipMarked",      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface), true);
    verifyNewString ("iipName",        getIonStormProperty(storm, game::interface::iipName,        tx, iface), "Ion storm #23");
    verifyNewNull   ("iipRadius",      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface));
    verifyNewNull   ("iipSpeedInt",    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface));
    verifyNewNull   ("iipSpeedName",   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface));
    verifyNewBoolean("iipStatusFlag",  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface), false);
    verifyNewString ("iipStatusName",  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface), "Weakening");
    verifyNewInteger("iipVoltage",     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface), 120);
}

/** Test setIonStormProperty().
    For now, no properties are settable. */
void
TestGameInterfaceIonStormProperty::testSet()
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active

    afl::data::StringValue sv("Katrina");
    TS_ASSERT_THROWS(setIonStormProperty(storm, game::interface::iipName, &sv), interpreter::Error);
}

