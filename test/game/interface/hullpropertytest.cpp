/**
  *  \file test/game/interface/hullpropertytest.cpp
  *  \brief Test for game::interface::HullProperty
  */

#include "game/interface/hullproperty.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using game::config::HostConfiguration;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewString;

/** Test getHullProperty(). */
AFL_TEST("game.interface.HullProperty:get", a)
{
    game::spec::ShipList list;
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::spec::Hull h(17);
    h.setMaxBeams(7);
    h.setMaxCargo(200);
    h.setMaxFuel(150);
    h.setMaxCrew(20);
    h.setNumEngines(2);
    h.setNumBays(6);
    h.setMaxLaunchers(9);
    h.setExternalPictureNumber(11);
    h.setInternalPictureNumber(22);

    // Check
    verifyNewInteger(a("MaxBeams"),         getHullProperty(h, game::interface::ihpMaxBeams,         list, *config), 7);
    verifyNewInteger(a("MaxCargo"),         getHullProperty(h, game::interface::ihpMaxCargo,         list, *config), 200);
    verifyNewInteger(a("MaxFuel"),          getHullProperty(h, game::interface::ihpMaxFuel,          list, *config), 150);
    verifyNewInteger(a("MaxCrew"),          getHullProperty(h, game::interface::ihpMaxCrew,          list, *config), 20);
    verifyNewInteger(a("NumEngines"),       getHullProperty(h, game::interface::ihpNumEngines,       list, *config), 2);
    verifyNewInteger(a("NumFighterBays"),   getHullProperty(h, game::interface::ihpNumFighterBays,   list, *config), 6);
    verifyNewString (a("Special"),          getHullProperty(h, game::interface::ihpSpecial,          list, *config), "");
    verifyNewInteger(a("Image"),            getHullProperty(h, game::interface::ihpImage,            list, *config), 22);
    verifyNewInteger(a("Image2"),           getHullProperty(h, game::interface::ihpImage2,           list, *config), 11);
    verifyNewInteger(a("MaxTorpLaunchers"), getHullProperty(h, game::interface::ihpMaxTorpLaunchers, list, *config), 9);
}

/** Test setHullProperty(). */
AFL_TEST("game.interface.HullProperty:set", a)
{
    game::spec::ShipList list;
    game::spec::Hull h(17);
    h.setInternalPictureNumber(22);

    std::auto_ptr<afl::data::Value> iv(interpreter::makeIntegerValue(77));

    // Successful set operation; verify
    AFL_CHECK_SUCCEEDS(a("01. set ihpImage"), setHullProperty(h, game::interface::ihpImage, iv.get(), list));
    a.checkEqual("02. getInternalPictureNumber", h.getInternalPictureNumber(), 77);

    // Failing operation
    AFL_CHECK_THROWS(a("11. set ihpNumFighterBays"), setHullProperty(h, game::interface::ihpNumFighterBays, iv.get(), list), interpreter::Error);
}

// Normal ability assigned directly
AFL_TEST("game.interface.HullProperty:ihpSpecial:direct-ability", a)
{
    game::spec::ShipList list;
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::spec::Hull h(1);
    h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS), game::PlayerSet_t(), true);
    verifyNewString(a, getHullProperty(h, game::interface::ihpSpecial, list, *config), "C");
}

// Normal ability assigned to single race is not reported
AFL_TEST("game.interface.HullProperty:ihpSpecial:race-limited", a)
{
    game::spec::ShipList list;
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::spec::Hull h(1);
    h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t(5), game::PlayerSet_t(), true);
    verifyNewString(a, getHullProperty(h, game::interface::ihpSpecial, list, *config), "");
}

// Mixed abilities that add up to full set
AFL_TEST("game.interface.HullProperty:ihpSpecial:mixed-abilities", a)
{
    game::spec::ShipList list;
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::spec::Hull h(1);
    h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t(5), game::PlayerSet_t(), true);
    h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::AdvancedCloak), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 5, game::PlayerSet_t(), true);
    verifyNewString(a, getHullProperty(h, game::interface::ihpSpecial, list, *config), "C");
}

// Ability that adds up with racial ability
AFL_TEST("game.interface.HullProperty:ihpSpecial:ship+racial", a)
{
    game::spec::ShipList list;
    Ref<HostConfiguration> config = HostConfiguration::create();
    game::spec::Hull h(1);
    list.racialAbilities().change(game::spec::BasicHullFunction::Bioscan, game::PlayerSet_t(5), game::PlayerSet_t());
    h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Bioscan), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 5, game::PlayerSet_t(), true);
    verifyNewString(a, getHullProperty(h, game::interface::ihpSpecial, list, *config), "B");
}
