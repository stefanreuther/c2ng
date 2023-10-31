/**
  *  \file u/t_server_play_torpedopacker.cpp
  *  \brief Test for server::play::TorpedoPacker
  */

#include "server/play/torpedopacker.hpp"

#include <memory>
#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"

/** Simple functionality test.
    A: create ship list; create TorpedoPacker
    E: correct values for all properties */
void
TestServerPlayTorpedoPacker::testIt()
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardTorpedoes(*sl);

    // Testee
    server::play::TorpedoPacker testee(*sl, *r, 0);
    TS_ASSERT_EQUALS(testee.getName(), "torp");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Standard list has 10 torpedoes, so this needs to be 11 elements (including dummy)
    TS_ASSERT_EQUALS(a.getArraySize(), 11U);
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[1].isNull());
    TS_ASSERT(!a[10].isNull());

    TS_ASSERT_EQUALS(a[1]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("MC").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TORPCOST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("MC").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("TUBECOST")("M").toInteger(), 0);
    TS_ASSERT_EQUALS(a[1]("DAMAGE1").toInteger(), 5);
    TS_ASSERT_EQUALS(a[1]("KILL1").toInteger(), 4);
    TS_ASSERT_EQUALS(a[1]("DAMAGE").toInteger(), 10);
    TS_ASSERT_EQUALS(a[1]("KILL").toInteger(), 8);
    TS_ASSERT_EQUALS(a[1]("TECH").toInteger(), 1);
    TS_ASSERT_EQUALS(a[1]("MASS").toInteger(), 1);

    TS_ASSERT_EQUALS(a[10]("NAME").toString(), "Mark 8 Photon");
    TS_ASSERT_EQUALS(a[10]("TORPCOST")("MC").toInteger(), 54);
    TS_ASSERT_EQUALS(a[10]("TORPCOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("TORPCOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("TORPCOST")("M").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("TUBECOST")("MC").toInteger(), 190);
    TS_ASSERT_EQUALS(a[10]("TUBECOST")("T").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("TUBECOST")("D").toInteger(), 1);
    TS_ASSERT_EQUALS(a[10]("TUBECOST")("M").toInteger(), 9);
    TS_ASSERT_EQUALS(a[10]("DAMAGE1").toInteger(), 55);
    TS_ASSERT_EQUALS(a[10]("KILL1").toInteger(), 35);
    TS_ASSERT_EQUALS(a[10]("DAMAGE").toInteger(), 110);
    TS_ASSERT_EQUALS(a[10]("KILL").toInteger(), 70);
    TS_ASSERT_EQUALS(a[10]("TECH").toInteger(), 10);
    TS_ASSERT_EQUALS(a[10]("MASS").toInteger(), 1);
}

/** Test offset 1.
    A: create TorpedoPacker with firstSlot=1
    E: no dummy element returned */
void
TestServerPlayTorpedoPacker::testOffset1()
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardTorpedoes(*sl);

    // Testee
    server::play::TorpedoPacker testee(*sl, *r, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    TS_ASSERT_EQUALS(a.getArraySize(), 10U);
    TS_ASSERT_EQUALS(a[0]("NAME").toString(), "Mark 1 Photon");
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Mark 8 Photon");
}

