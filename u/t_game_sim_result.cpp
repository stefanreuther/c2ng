/**
  *  \file u/t_game_sim_result.cpp
  *  \brief Test for game::sim::Result
  */

#include "game/sim/result.hpp"

#include "t_game_sim.hpp"

/** Verify a simple sequence. */
void
TestGameSimResult::testIt()
{
    game::sim::Result result;
    TS_ASSERT(result.this_battle_weight > 0);
    TS_ASSERT_EQUALS(result.this_battle_index, 0);

    // Initialize
    game::sim::Configuration config;
    game::TeamSettings team;
    game::config::HostConfiguration hostConfiguration; 
    config.setMode(config.VcrHost, team, hostConfiguration);
    result.init(config, 120);

    TS_ASSERT_EQUALS(result.series_length, 110);
    TS_ASSERT_EQUALS(result.this_battle_weight, 1);
    TS_ASSERT_EQUALS(result.total_battle_weight, 1);

    int n = result.addSeries(2);
    TS_ASSERT_EQUALS(n, 1); // 120 is in second series, thus result is 1
    TS_ASSERT_EQUALS(result.series_length, 220);

    result.changeWeightTo(7);
    TS_ASSERT_EQUALS(result.this_battle_weight, 7);
    TS_ASSERT_EQUALS(result.total_battle_weight, 7);
}

