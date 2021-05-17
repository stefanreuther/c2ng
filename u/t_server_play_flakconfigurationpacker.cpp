/**
  *  \file u/t_server_play_flakconfigurationpacker.cpp
  *  \brief Test for server::play::FlakConfigurationPacker
  */

#include <memory>
#include "server/play/flakconfigurationpacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"

void
TestServerPlayFlakConfigurationPacker::testIt()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(new game::test::Root(game::HostVersion()));

    // Set some recognizable valuess
    session.getRoot()->flakConfiguration().StartingDistanceShip = 23456;
    session.getRoot()->flakConfiguration().RatingPEBonus = 42;

    // Verify constructor
    server::play::FlakConfigurationPacker testee(session);
    TS_ASSERT_EQUALS(testee.getName(), "flakconfig");

    // Verify buildValue
    std::auto_ptr<server::Value_t> result(testee.buildValue());
    afl::data::Access a(result.get());
    TS_ASSERT_EQUALS(a("StartingDistanceShip").toInteger(), 23456);
    TS_ASSERT_EQUALS(a("RatingPEBonus").toInteger(), 42);
}

