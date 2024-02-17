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
    game::map::IonStorm storm(42);
    storm.setName("Kyrill");
    storm.setPosition(game::map::Point(4503, 1701));
    storm.setRadius(20);
    storm.setVoltage(40);
    storm.setWarpFactor(4);
    storm.setHeading(70);
    storm.setIsGrowing(true);

    verifyNewInteger(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx), 1);
    verifyNewInteger(a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx), 70);
    verifyNewString (a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx), "ENE");
    verifyNewInteger(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx), 42);
    verifyNewInteger(a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx), 4503);
    verifyNewInteger(a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx), 1701);
    verifyNewBoolean(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx), false);
    verifyNewString (a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx), "Kyrill");
    verifyNewInteger(a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx), 20);
    verifyNewInteger(a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx), 4);
    verifyNewString (a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx), "Warp 4");
    verifyNewBoolean(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx), true);
    verifyNewString (a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx), "Growing");
    verifyNewInteger(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx), 40);
}

/** Test property retrieval, empty storm.
    An empty (inactive, invisible) storm reports all properties as empty. */
AFL_TEST("game.interface.IonStormProperty:get:empty", a)
{
    afl::string::NullTranslator tx;
    game::map::IonStorm storm(17);
    a.check("isActive", !storm.isActive());

    verifyNewNull(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx));
    verifyNewNull(a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx));
    verifyNewNull(a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx));
    verifyNewNull(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx));
    verifyNewNull(a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx));
    verifyNewNull(a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx));
    verifyNewNull(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx));
    verifyNewNull(a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx));
    verifyNewNull(a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx));
    verifyNewNull(a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx));
    verifyNewNull(a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx));
    verifyNewNull(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx));
    verifyNewNull(a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx));
    verifyNewNull(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx));
}

/** Test property retrieval, mostly empty storm.
    Most properties are nullable and return empty if never set. */
AFL_TEST("game.interface.IonStormProperty:get:mostly-empty", a)
{
    afl::string::NullTranslator tx;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active
    storm.setIsMarked(true);
    a.check("isActive", storm.isActive());

    verifyNewInteger(a("iipClass"),       getIonStormProperty(storm, game::interface::iipClass,       tx), 3);
    verifyNewNull   (a("iipHeadingInt"),  getIonStormProperty(storm, game::interface::iipHeadingInt,  tx));
    verifyNewNull   (a("iipHeadingName"), getIonStormProperty(storm, game::interface::iipHeadingName, tx));
    verifyNewInteger(a("iipId"),          getIonStormProperty(storm, game::interface::iipId,          tx), 23);
    verifyNewNull   (a("iipLocX"),        getIonStormProperty(storm, game::interface::iipLocX,        tx));
    verifyNewNull   (a("iipLocY"),        getIonStormProperty(storm, game::interface::iipLocY,        tx));
    verifyNewBoolean(a("iipMarked"),      getIonStormProperty(storm, game::interface::iipMarked,      tx), true);
    verifyNewString (a("iipName"),        getIonStormProperty(storm, game::interface::iipName,        tx), "Ion storm #23");
    verifyNewNull   (a("iipRadius"),      getIonStormProperty(storm, game::interface::iipRadius,      tx));
    verifyNewNull   (a("iipSpeedInt"),    getIonStormProperty(storm, game::interface::iipSpeedInt,    tx));
    verifyNewNull   (a("iipSpeedName"),   getIonStormProperty(storm, game::interface::iipSpeedName,   tx));
    verifyNewBoolean(a("iipStatusFlag"),  getIonStormProperty(storm, game::interface::iipStatusFlag,  tx), false);
    verifyNewString (a("iipStatusName"),  getIonStormProperty(storm, game::interface::iipStatusName,  tx), "Weakening");
    verifyNewInteger(a("iipVoltage"),     getIonStormProperty(storm, game::interface::iipVoltage,     tx), 120);
}

/** Test setIonStormProperty().
    For now, no properties are settable. */
AFL_TEST("game.interface.IonStormProperty:set", a)
{
    afl::string::NullTranslator tx;
    game::map::IonStorm storm(23);
    storm.setVoltage(120);              // This makes the storm active

    afl::data::StringValue sv("Katrina");
    AFL_CHECK_THROWS(a("set iipName"), setIonStormProperty(storm, game::interface::iipName, &sv), interpreter::Error);
}
