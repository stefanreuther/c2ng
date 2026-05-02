/**
  *  \file game/sim/result.cpp
  *  \brief Class game::sim::Result
  */

#include "game/sim/result.hpp"

// Initialize.
game::sim::Result::Result(const Configuration& config, int this_battle_index)
    : this_battle_weight(1),
      total_battle_weight(1),
      series_length(config.getMode() == Configuration::VcrNuHost ? 118 : 110),
      this_battle_index(this_battle_index),
      battles()
{ }

// Add a series of a given length.
int
game::sim::Result::addSeries(int length)
{
    int result = this_battle_index / series_length;
    series_length *= length;
    return result % length;
}

// Adjust weight.
void
game::sim::Result::changeWeightTo(int32_t new_weight)
{
    this_battle_weight = this_battle_weight * new_weight / total_battle_weight;
    total_battle_weight = new_weight;
}
