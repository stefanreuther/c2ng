/**
  *  \file u/t_game_config_aliasoption.cpp
  *  \brief Test for game::config::AliasOption
  */

#include "game/config/aliasoption.hpp"

#include "t_game_config.hpp"
#include "game/config/stringoption.hpp"

/** Test AliasOption. */
void
TestGameConfigAliasOption::testIt()
{
    static const game::config::StringOptionDescriptor base = { "Base" };
    static const game::config::AliasOptionDescriptor one = { "One", "base" };
    static const game::config::AliasOptionDescriptor two = { "Two", "Other" };
    
    game::config::Configuration fig;
    fig[base].set("hi");

    // Validate option one
    TS_ASSERT_EQUALS(fig[one].getForwardedOption(), &fig[base]);
    TS_ASSERT_EQUALS(fig[one].toString(), "hi");

    // Validate option two (dead link)
    TS_ASSERT(fig[two].getForwardedOption() == 0);
    TS_ASSERT_EQUALS(fig[two].toString(), "");

    // Modify
    fig[one].set("ho");
    TS_ASSERT_EQUALS(fig[base].toString(), "ho");

    // Modify dead link; this is ignored
    fig[two].set("ha");
}

