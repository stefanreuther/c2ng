/**
  *  \file test/game/config/aliasoptiontest.cpp
  *  \brief Test for game::config::AliasOption
  */

#include "game/config/aliasoption.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/configuration.hpp"
#include "game/config/stringoption.hpp"

/** Test AliasOption. */
AFL_TEST("game.config.AliasOption", a)
{
    static const game::config::StringOptionDescriptor base = { "Base" };
    static const game::config::AliasOptionDescriptor one = { "One", "base" };
    static const game::config::AliasOptionDescriptor two = { "Two", "Other" };

    afl::base::Ref<game::config::Configuration> rfig = game::config::Configuration::create();
    game::config::Configuration& fig = *rfig;
    fig[base].set("hi");

    // Validate option one
    a.checkEqual("01. getForwardedOption", fig[one].getForwardedOption(), &fig[base]);
    a.checkEqual("02. toString", fig[one].toString(), "hi");

    // Validate option two (dead link)
    a.checkNull("11. getForwardedOption", fig[two].getForwardedOption());
    a.checkEqual("12. toString", fig[two].toString(), "");

    // Modify
    fig[one].set("ho");
    a.checkEqual("21.  toString", fig[base].toString(), "ho");

    // Modify dead link; this is ignored
    fig[two].set("ha");
}
