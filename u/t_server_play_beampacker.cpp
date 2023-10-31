/**
  *  \file u/t_server_play_beampacker.cpp
  *  \brief Test for server::play::BeamPacker
  */

#include "server/play/beampacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

/** Simple functionality test.
    A: create ship list; create BeamPacker
    E: correct values for all properties */
void
TestServerPlayBeamPacker::testIt()
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardBeams(*sl);

    // Testee
    server::play::BeamPacker testee(*sl, *r, 0);
    TS_ASSERT_EQUALS(testee.getName(), "beam");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Standard list has 10 beams, so this needs to be 11 elements (including dummy)
    TS_ASSERT_EQUALS(a.getArraySize(), 11U);
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[1].isNull());
    TS_ASSERT(!a[10].isNull());

    // Verify all attributes of #1
    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[1]("COST")("MC").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("COST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("COST")("D").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("COST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("DAMAGE").toInteger(), 3);
    TS_ASSERT_EQUALS(a[1]("KILL").toInteger(), 10);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("MASS").toInteger(), 1);

    // Verify all attributes of #10
    TS_ASSERT_EQUALS(a[10]("NAME").toString(), "Heavy Phaser");
    TS_ASSERT_EQUALS(a[10]("COST")("MC").toInteger(), 54);
    TS_ASSERT_EQUALS(a[10]("COST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("COST")("D").toInteger(), 12);
    TS_ASSERT_EQUALS(a[10]("COST")("M").toInteger(), 55);
    TS_ASSERT_EQUALS(a[10]("DAMAGE").toInteger(), 45);
    TS_ASSERT_EQUALS(a[10]("KILL").toInteger(), 35);
    TS_ASSERT_EQUALS(a[10]("TECH").toInteger(), 10);
    TS_ASSERT_EQUALS(a[10]("MASS").toInteger(), 6);
}

/** Test offset 1.
    A: create BeamPacker with firstSlot=1
    E: no dummy element returned */
void
TestServerPlayBeamPacker::testOffset1()
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardBeams(*sl);

    // Testee
    server::play::BeamPacker testee(*sl, *r, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    TS_ASSERT_EQUALS(a.getArraySize(), 10U);
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Laser");
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Heavy Phaser");
}

