/**
  *  \file test/game/sim/resulttest.cpp
  *  \brief Test for game::sim::Result
  */

#include "game/sim/result.hpp"
#include "afl/test/testrunner.hpp"

/** Verify a simple sequence. */
AFL_TEST("game.sim.Result", a)
{
    game::sim::Result result;
    a.check("01. this_battle_weight", result.this_battle_weight > 0);
    a.checkEqual("02. this_battle_index", result.this_battle_index, 0);

    // Initialize
    game::sim::Configuration config;
    game::config::HostConfiguration hostConfiguration;
    config.setMode(config.VcrHost, 0, hostConfiguration);
    result.init(config, 120);

    a.checkEqual("11. series_length", result.series_length, 110);
    a.checkEqual("12. this_battle_weight", result.this_battle_weight, 1);
    a.checkEqual("13. total_battle_weight", result.total_battle_weight, 1);

    int n = result.addSeries(2);
    a.checkEqual("21. addSeries", n, 1); // 120 is in second series, thus result is 1
    a.checkEqual("22. series_length", result.series_length, 220);

    result.changeWeightTo(7);
    a.checkEqual("31. this_battle_weight", result.this_battle_weight, 7);
    a.checkEqual("32. total_battle_weight", result.total_battle_weight, 7);
}
