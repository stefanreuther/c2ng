/**
  *  \file u/t_server_play_flakconfigurationpacker.cpp
  *  \brief Test for server::play::FlakConfigurationPacker
  */

#include <memory>
#include "server/play/flakconfigurationpacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/test/root.hpp"

void
TestServerPlayFlakConfigurationPacker::testIt()
{
    // Create configuration with some recognizable valuess
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    root->flakConfiguration().StartingDistanceShip = 23456;
    root->flakConfiguration().RatingPEBonus = 42;

    // Verify constructor
    server::play::FlakConfigurationPacker testee(*root);
    TS_ASSERT_EQUALS(testee.getName(), "flakconfig");

    // Verify buildValue
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access a(result.get());
    TS_ASSERT_EQUALS(a("StartingDistanceShip").toInteger(), 23456);
    TS_ASSERT_EQUALS(a("RatingPEBonus").toInteger(), 42);
}

