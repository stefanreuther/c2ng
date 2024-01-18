/**
  *  \file test/server/play/flakconfigurationpackertest.cpp
  *  \brief Test for server::play::FlakConfigurationPacker
  */

#include "server/play/flakconfigurationpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include <memory>

AFL_TEST("server.play.FlakConfigurationPacker", a)
{
    // Create configuration with some recognizable valuess
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion()));
    root->flakConfiguration().StartingDistanceShip = 23456;
    root->flakConfiguration().RatingPEBonus = 42;

    // Verify constructor
    server::play::FlakConfigurationPacker testee(*root);
    a.checkEqual("01. getName", testee.getName(), "flakconfig");

    // Verify buildValue
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access ap(result.get());
    a.checkEqual("11", ap("StartingDistanceShip").toInteger(), 23456);
    a.checkEqual("12", ap("RatingPEBonus").toInteger(), 42);
}
