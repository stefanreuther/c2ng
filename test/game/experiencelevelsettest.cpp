/**
  *  \file test/game/experiencelevelsettest.cpp
  *  \brief Test for game::ExperienceLevelSet
  */

#include "game/experiencelevelset.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Test cases without experience, formatting fails immediately. */
AFL_TEST("game.ExperienceLevelSet:formatExperienceLevelSet:errors", a)
{
    afl::string::NullTranslator tx;
    game::ExperienceLevelSet_t t = game::ExperienceLevelSet_t::allUpTo(5);

    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(0);
    a.checkEqual("01", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Unknown, 0), config, tx), "");
    a.checkEqual("02", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Host, MKVERSION(3,20,0)), config, tx), "");
    a.checkEqual("03", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::NuHost, MKVERSION(3,20,0)), config, tx), "");
    a.checkEqual("04", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,3,0)), config, tx), "");

    config[config.NumExperienceLevels].set(3);
    a.checkEqual("11", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Unknown, 0), config, tx), "");
    a.checkEqual("12", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::Host, MKVERSION(3,20,0)), config, tx), "");
    a.checkEqual("13", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::NuHost, MKVERSION(3,20,0)), config, tx), "");
    a.checkEqual("14", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(3,4,0)), config, tx), "");
    a.checkEqual("15", game::formatExperienceLevelSet(t, game::HostVersion(game::HostVersion::PHost, MKVERSION(4,3,0)), config, tx), "");
}

/** Test cases with experience, format operates normally. */
AFL_TEST("game.ExperienceLevelSet:formatExperienceLevelSet", a)
{
    afl::string::NullTranslator tx;
    game::config::HostConfiguration config;
    config[config.NumExperienceLevels].set(5);
    game::HostVersion host(game::HostVersion::PHost, MKVERSION(4,0,0));

    // No level
    a.checkEqual("01", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(), host, config, tx), "no level");

    // Level outside valid range, maps to no level
    a.checkEqual("11", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(10), host, config, tx), "no level");

    // All levels up to
    a.checkEqual("21", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3) + 4 + 5 + 6 + 7, host, config, tx), "level 3+");
    a.checkEqual("22", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3) + 4 + 5, host, config, tx), "level 3+");

    // Only one level
    a.checkEqual("31", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(3), host, config, tx), "level 3");
    a.checkEqual("32", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(5), host, config, tx), "level 5");
    a.checkEqual("33", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(0), host, config, tx), "level 0");

    // Mixed
    a.checkEqual("41", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(0) + 1, host, config, tx), "levels 0, 1");
    a.checkEqual("42", game::formatExperienceLevelSet(game::ExperienceLevelSet_t(1) + 3 + 5, host, config, tx), "levels 1, 3, 5");
}
