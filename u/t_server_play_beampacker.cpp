/**
  *  \file u/t_server_play_beampacker.cpp
  *  \brief Test for server::play::BeamPacker
  */

#include "server/play/beampacker.hpp"

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
TestServerPlayBeamPacker::testIt()
{
    // Session
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    // Populate session
    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    game::test::initStandardBeams(*session.getShipList());

    // Testee
    server::play::BeamPacker testee(session);
    TS_ASSERT_EQUALS(testee.getName(), "beam");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Standard list has 10 beams, so this needs to be 11 elements (including dummy)
    TS_ASSERT_EQUALS(a.getArraySize(), 11U);
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[1].isNull());
    TS_ASSERT(!a[10].isNull());

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[10]("NAME").toString(), "Heavy Phaser");
    TS_ASSERT_EQUALS(a[10]("DAMAGE").toInteger(), 45);
    TS_ASSERT_EQUALS(a[10]("COST")("MC").toInteger(), 54);
}

