/**
  *  \file u/t_game_interface_hullproperty.cpp
  *  \brief Test for game::interface::HullProperty
  */

#include "game/interface/hullproperty.hpp"

#include "t_game_interface.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/spec/shiplist.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewString;

/** Test getHullProperty(). */
void
TestGameInterfaceHullProperty::testGet()
{
    game::spec::ShipList list;
    game::config::HostConfiguration config;
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
    verifyNewInteger("MaxBeams",         getHullProperty(h, game::interface::ihpMaxBeams,         list, config), 7);
    verifyNewInteger("MaxCargo",         getHullProperty(h, game::interface::ihpMaxCargo,         list, config), 200);
    verifyNewInteger("MaxFuel",          getHullProperty(h, game::interface::ihpMaxFuel,          list, config), 150);
    verifyNewInteger("MaxCrew",          getHullProperty(h, game::interface::ihpMaxCrew,          list, config), 20);
    verifyNewInteger("NumEngines",       getHullProperty(h, game::interface::ihpNumEngines,       list, config), 2);
    verifyNewInteger("NumFighterBays",   getHullProperty(h, game::interface::ihpNumFighterBays,   list, config), 6);
    verifyNewString ("Special",          getHullProperty(h, game::interface::ihpSpecial,          list, config), "");
    verifyNewInteger("Image",            getHullProperty(h, game::interface::ihpImage,            list, config), 22);
    verifyNewInteger("Image2",           getHullProperty(h, game::interface::ihpImage2,           list, config), 11);
    verifyNewInteger("MaxTorpLaunchers", getHullProperty(h, game::interface::ihpMaxTorpLaunchers, list, config), 9);
}

/** Test setHullProperty(). */
void
TestGameInterfaceHullProperty::testSet()
{
    game::spec::ShipList list;
    game::spec::Hull h(17);
    h.setInternalPictureNumber(22);

    std::auto_ptr<afl::data::Value> iv(interpreter::makeIntegerValue(77));

    // Successful set operation; verify
    TS_ASSERT_THROWS_NOTHING(setHullProperty(h, game::interface::ihpImage, iv.get(), list));
    TS_ASSERT_EQUALS(h.getInternalPictureNumber(), 77);

    // Failing operation
    TS_ASSERT_THROWS(setHullProperty(h, game::interface::ihpNumFighterBays, iv.get(), list), interpreter::Error);
}

/** Test ihpSpecial. */
void
TestGameInterfaceHullProperty::testSpecial()
{
    game::spec::ShipList list;
    game::config::HostConfiguration config;

    // Normal ability assigned directly
    {
        game::spec::Hull h(1);
        h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS), game::PlayerSet_t(), true);
        verifyNewString("Special", getHullProperty(h, game::interface::ihpSpecial, list, config), "C");
    }

    // Normal ability assigned to single race is not reported
    {
        game::spec::Hull h(1);
        h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t(5), game::PlayerSet_t(), true);
        verifyNewString("Special", getHullProperty(h, game::interface::ihpSpecial, list, config), "");
    }

    // Mixed abilities that add up to full set
    {
        game::spec::Hull h(1);
        h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak), game::PlayerSet_t(5), game::PlayerSet_t(), true);
        h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::AdvancedCloak), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 5, game::PlayerSet_t(), true);
        verifyNewString("Special", getHullProperty(h, game::interface::ihpSpecial, list, config), "C");
    }

    // Ability that adds up with racial ability
    {
        game::spec::Hull h(1);
        list.racialAbilities().change(game::spec::BasicHullFunction::Bioscan, game::PlayerSet_t(5), game::PlayerSet_t());
        h.changeHullFunction(list.modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Bioscan), game::PlayerSet_t::allUpTo(game::MAX_PLAYERS) - 5, game::PlayerSet_t(), true);
        verifyNewString("Special", getHullProperty(h, game::interface::ihpSpecial, list, config), "B");
    }
}

