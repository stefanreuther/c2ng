/**
  *  \file u/t_game_v3_hconfig.cpp
  *  \brief Test for game::v3::HConfig
  */

#include "game/v3/hconfig.hpp"

#include "t_game_v3.hpp"
#include "game/v3/structures.hpp"

/** Test that packHConfig() initializes everything. */
void
TestGameV3HConfig::testPack()
{
    // Prepare
    game::v3::structures::HConfig fig;
    afl::base::Bytes_t bytes(afl::base::fromObject(fig));
    bytes.fill(0xE1);

    // Pack a default host configuration
    game::config::HostConfiguration config;
    game::v3::packHConfig(fig, config);

    // Check. There shouldn't be a 0xE1 byte in there
    TS_ASSERT_EQUALS(bytes.find(0xE1), bytes.size());
}

