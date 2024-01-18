/**
  *  \file test/game/interface/ionstormpropertytest.cpp
  *  \brief Test for game::interface::IonStormProperty
  */

#include "game/interface/ionstormproperty.hpp"

#include "afl/data/stringvalue.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/interpreterinterface.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

/** Test property retrieval, fully populated storm. */
AFL_TEST("game.interface.IonStormProperty:get:full", a)
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

    verifyNewInteger(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx, iface), 1);
    verifyNewInteger(a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface), 70);
    verifyNewString (a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface), "ENE");
    verifyNewInteger(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx, iface), 42);
    verifyNewInteger(a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface), 4503);
    verifyNewInteger(a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface), 1701);
    verifyNewBoolean(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface), false);
    verifyNewString (a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx, iface), "Kyrill");
    verifyNewInteger(a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface), 20);
    verifyNewInteger(a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface), 4);
    verifyNewString (a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface), "Warp 4");
    verifyNewBoolean(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface), true);
    verifyNewString (a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface), "Growing");
    verifyNewInteger(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface), 40);
}

/** Test property retrieval, empty storm.
    An empty (inactive, invisible) storm reports all properties as empty. */
AFL_TEST("game.interface.IonStormProperty:get:empty", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(17);
    a.check("isActive", !storm.isActive());

    verifyNewNull(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx, iface));
    verifyNewNull(a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface));
    verifyNewNull(a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface));
    verifyNewNull(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx, iface));
    verifyNewNull(a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface));
    verifyNewNull(a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface));
    verifyNewNull(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface));
    verifyNewNull(a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx, iface));
    verifyNewNull(a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface));
    verifyNewNull(a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface));
    verifyNewNull(a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface));
    verifyNewNull(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface));
    verifyNewNull(a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface));
    verifyNewNull(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface));
}

/** Test property retrieval, mostly empty storm.
    Most properties are nullable and return empty if never set. */
AFL_TEST("game.interface.IonStormProperty:get:mostly-empty", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active
    storm.setIsMarked(true);
    a.check("isActive", storm.isActive());

    verifyNewInteger(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx, iface), 3);
    verifyNewNull   (a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx, iface));
    verifyNewNull   (a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx, iface));
    verifyNewInteger(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx, iface), 23);
    verifyNewNull   (a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx, iface));
    verifyNewNull   (a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx, iface));
    verifyNewBoolean(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx, iface), true);
    verifyNewString (a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx, iface), "Ion storm #23");
    verifyNewNull   (a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx, iface));
    verifyNewNull   (a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx, iface));
    verifyNewNull   (a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx, iface));
    verifyNewBoolean(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx, iface), false);
    verifyNewString (a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx, iface), "Weakening");
    verifyNewInteger(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx, iface), 120);
}

/** Test setIonStormProperty().
    For now, no properties are settable. */
AFL_TEST("game.interface.IonStormProperty:set", a)
{
    afl::string::NullTranslator tx;
    game::test::InterpreterInterface iface;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active

    afl::data::StringValue sv("Katrina");
    AFL_CHECK_THROWS(a("set iipName"), setIonStormProperty(storm, game::interface::iipName, &sv), interpreter::Error);
}
