/**
  *  \file u/t_server_play_enginepacker.cpp
  *  \brief Test for server::play::EnginePacker
  */

#include "server/play/enginepacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/test/shiplist.hpp"

/** Simple functionality test.
    A: create ship list; create EnginePacker
    E: correct values for all properties */
void
TestServerPlayEnginePacker::testIt()
{
    // Input data
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::addTranswarp(*sl);
    game::test::addNovaDrive(*sl);

    // Testee
    server::play::EnginePacker testee(*sl, 0);
    TS_ASSERT_EQUALS(testee.getName(), "engine");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    // Transwarp is #9, so this needs to be 10 elements (including dummy)
    TS_ASSERT_EQUALS(a.getArraySize(), 10U);
    TS_ASSERT(a[0].isNull());
    TS_ASSERT(!a[9].isNull());

    // Verify all attributes of #5
    TS_ASSERT_EQUALS(a[5]("NAME").toString(), "Nova Drive 5");

    // Verify all attributes of #9
    TS_ASSERT_EQUALS(a[9]("NAME").toString(), "Transwarp Drive");
    TS_ASSERT_EQUALS(a[9]("COST")("MC").toInteger(), 300);
    TS_ASSERT_EQUALS(a[9]("COST")("T").toInteger(), 3);
    TS_ASSERT_EQUALS(a[9]("COST")("D").toInteger(), 16);
    TS_ASSERT_EQUALS(a[9]("COST")("M").toInteger(), 35);
    TS_ASSERT_EQUALS(a[9]("TECH").toInteger(), 10);
    TS_ASSERT_EQUALS(a[9]("SPEED").toInteger(), 9);
    TS_ASSERT_EQUALS(a[9]("FUELFACTOR")[0].toInteger(), 0);
    TS_ASSERT_EQUALS(a[9]("FUELFACTOR")[1].toInteger(), 100);
    TS_ASSERT_EQUALS(a[9]("FUELFACTOR")[2].toInteger(), 400);
    TS_ASSERT_EQUALS(a[9]("FUELFACTOR")[9].toInteger(), 8100);
}

/** Test offset 1.
    A: create BeamPacker with firstSlot=1
    E: no dummy element returned */
void
TestServerPlayEnginePacker::testOffset1()
{
    // Input data
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::addTranswarp(*sl);
    game::test::addNovaDrive(*sl);

    // Testee
    server::play::EnginePacker testee(*sl, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access a(value.get());

    TS_ASSERT_EQUALS(a[4]("NAME").toString(), "Nova Drive 5");
    TS_ASSERT_EQUALS(a[8]("NAME").toString(), "Transwarp Drive");
}
