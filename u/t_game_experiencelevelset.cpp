/**
  *  \file u/t_game_experiencelevelset.cpp
  *  \brief Test for game::ExperienceLevelSet
  */

#include "game/experiencelevelset.hpp"

#include "t_game.hpp"
#include "afl/string/nulltranslator.hpp"

/** Test cases without experience, formatting fails immediately. */
void
TestGameExperienceLevelSet::testPreconditions()
{
    afl::string::NullTranslator tx;
    game::ExperienceLevelSet_t t = game::ExperienceLevelSet_t::allUpTo(5);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(0);
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Unknown, 0), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Host, MKVERSION(3,20,0)), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::NuHost, MKVERSION(3,20,0)), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,3,0)), config, tx), "");

    config[config.NumExperienceLevels].set(3);
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Unknown, 0), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Host, MKVERSION(3,20,0)), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::NuHost, MKVERSION(3,20,0)), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(3,4,0)), config, tx), "");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,3,0)), config, tx), "");
}

/** Test cases with experience, format operates normally. */
void
TestGameExperienceLevelSet::testFormat()
{
    afl::string::NullTranslator tx;
    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(5);
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4,0,0));

    // No level
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(), host, config, tx), "no level");

    // Level outside valid range, maps to no level
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(10), host, config, tx), "no level");

    // All levels up to
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3) + 4 + 5 + 6 + 7, host, config, tx), "level 3+");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3) + 4 + 5, host, config, tx), "level 3+");

    // Only one level
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3), host, config, tx), "level 3");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(5), host, config, tx), "level 5");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(0), host, config, tx), "level 0");

    // Mixed
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(0) + 1, host, config, tx), "levels 0, 1");
    TS_ASSERT_EQUALS(game::formatExperienceLevelSet(game::ExperienceLevelSet_t(1) + 3 + 5, host, config, tx), "levels 1, 3, 5");
}

