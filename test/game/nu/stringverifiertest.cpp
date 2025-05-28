/**
  *  \file test/game/nu/stringverifiertest.cpp
  *  \brief Test for game::nu::StringVerifier
  */

#include "game/nu/stringverifier.hpp"

#include "afl/test/testrunner.hpp"

using game::nu::StringVerifier;

AFL_TEST("game.nu.StringVerifier:basics", a)
{
    StringVerifier testee;

    a.check("01", testee.isValidString(StringVerifier::FriendlyCode, ""));
    a.check("02", testee.isValidString(StringVerifier::FriendlyCode, "foo"));
    a.check("03", !testee.isValidString(StringVerifier::FriendlyCode, "foo1"));

    a.check("11", testee.isValidString(StringVerifier::ShipName, ""));
    a.check("12", testee.isValidString(StringVerifier::ShipName, "xxxxxxxxxx"));
    a.check("13", testee.isValidString(StringVerifier::ShipName, "xxxxxxxxxxyyyyyyyyyyxxxxxxxxxxyyyyyyyyyyxxxxxxxxxx"));
    a.check("14", !testee.isValidString(StringVerifier::ShipName, "xxxxxxxxxxyyyyyyyyyyxxxxxxxxxxyyyyyyyyyyxxxxxxxxxxz"));

    a.check("21", testee.isValidCharacter(StringVerifier::ShipName, ' '));
    a.check("22", testee.isValidCharacter(StringVerifier::ShipName, 0xFF));
    a.check("23", testee.isValidCharacter(StringVerifier::ShipName, 0x100));
    a.check("24", testee.isValidCharacter(StringVerifier::ShipName, 0x1000));
    a.check("25", testee.isValidCharacter(StringVerifier::ShipName, 0x10000));
    a.check("26", testee.isValidCharacter(StringVerifier::ShipName, 0x103C));
}

// Specific tests for the generic blacklist
AFL_TEST("game.nu.StringVerifier:ship-name", a)
{
    StringVerifier testee;

    a.check("01", !testee.isValidString(StringVerifier::ShipName, "USS <blink>"));
    a.check("02", !testee.isValidString(StringVerifier::ShipName, "USS &nbsp;"));
    a.check("03", !testee.isValidString(StringVerifier::ShipName, "USS a|||b"));
    a.check("04", !testee.isValidString(StringVerifier::ShipName, "USS a:::b"));
    a.check("05", !testee.isValidString(StringVerifier::ShipName, "USS a=b"));
}

// Specific tests for the message blacklist
AFL_TEST("game.nu.StringVerifier:message", a)
{
    StringVerifier testee;

    a.check("01", !testee.isValidString(StringVerifier::Message, "USS <blink>"));
    a.check("02", !testee.isValidString(StringVerifier::Message, "USS &nbsp;"));
    a.check("03",  testee.isValidString(StringVerifier::Message, "USS a|||b"));
    a.check("04",  testee.isValidString(StringVerifier::Message, "USS a:::b"));
    a.check("05",  testee.isValidString(StringVerifier::Message, "USS a=b"));
}

// Coverage test for clone, getMaxStringLength
AFL_TEST("game.nu.StringVerifier:getMaxStringLength", a)
{
    StringVerifier testee;
    std::auto_ptr<StringVerifier> dup(testee.clone());

    a.checkNonNull("01. clone", dup.get());

    a.checkEqual("11", testee.getMaxStringLength(StringVerifier::Unknown),             dup->getMaxStringLength(StringVerifier::Unknown));
    a.checkEqual("12", testee.getMaxStringLength(StringVerifier::ShipName),            dup->getMaxStringLength(StringVerifier::ShipName));
    a.checkEqual("13", testee.getMaxStringLength(StringVerifier::PlanetName),          dup->getMaxStringLength(StringVerifier::PlanetName));
    a.checkEqual("14", testee.getMaxStringLength(StringVerifier::PlayerLongName),      dup->getMaxStringLength(StringVerifier::PlayerLongName));
    a.checkEqual("15", testee.getMaxStringLength(StringVerifier::PlayerShortName),     dup->getMaxStringLength(StringVerifier::PlayerShortName));
    a.checkEqual("16", testee.getMaxStringLength(StringVerifier::PlayerAdjectiveName), dup->getMaxStringLength(StringVerifier::PlayerAdjectiveName));
    a.checkEqual("17", testee.getMaxStringLength(StringVerifier::FriendlyCode),        dup->getMaxStringLength(StringVerifier::FriendlyCode));
    a.checkEqual("18", testee.getMaxStringLength(StringVerifier::Message),             dup->getMaxStringLength(StringVerifier::Message));
}
