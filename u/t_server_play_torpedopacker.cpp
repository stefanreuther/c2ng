/**
  *  \file u/t_server_play_torpedopacker.cpp
  *  \brief Test for server::play::TorpedoPacker
  */

#include "server/play/torpedopacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

/** Simple functionality test. */
void
TestServerPlayTorpedoPacker::testIt()
{
    // Session
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    // Populate session
    session.setRoot(new game::test::Root(game::HostVersion()));
    session.setShipList(new game::spec::ShipList());
    game::test::initStandardTorpedoes(*session.getShipList());

    // Testee
    server::play::TorpedoPacker testee(session);
    TS_ASSERT_EQUALS(testee.getName(), "torp");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Standard list has 10 torpedoes, so this needs to be 11 elements (including dummy)
    TS_ASSERT_EQUALS(a.getArraySize(), 11U);
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[1].isNull());
    TS_ASSERT(!a[10].isNull());

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a[10]("NAME").toString(), "Mark 8 Photon");
    TS_ASSERT_EQUALS(a[10]("DAMAGE1").toInteger(), 55);
    TS_ASSERT_EQUALS(a[10]("TUBECOST")("MC").toInteger(), 190);
    TS_ASSERT_EQUALS(a[10]("TORPCOST")("MC").toInteger(), 54);
}

