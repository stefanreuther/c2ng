/**
  *  \file test/server/play/enginepackertest.cpp
  *  \brief Test for server::play::EnginePacker
  */

#include "server/play/enginepacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/shiplist.hpp"

/** Simple functionality test.
    A: create ship list; create EnginePacker
    E: correct values for all properties */
AFL_TEST("server.play.EnginePacker:basics", a)
{
    // Input data
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::addTranswarp(*sl);
    game::test::addNovaDrive(*sl);
    sl->engines().get(5)->setShortName("Nova");
    sl->engines().get(9)->setShortName("TWD");

    // Testee
    server::play::EnginePacker testee(*sl, 0);
    a.checkEqual("01. getName", testee.getName(), "engine");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Transwarp is #9, so this needs to be 10 elements (including dummy)
    a.checkEqual("11. getArraySize", ap.getArraySize(), 10U);
    a.check("12. result", ap[0].isNull());
    a.check("13. result", !ap[9].isNull());

    // Verify all attributes of #5
    a.checkEqual("21", ap[5]("NAME").toString(), "Nova Drive 5");
    a.checkEqual("21a", ap[5]("NAME.SHORT").toString(), "Nova");

    // Verify all attributes of #9
    a.checkEqual("31", ap[9]("NAME").toString(), "Transwarp Drive");
    a.checkEqual("31a", ap[9]("NAME.SHORT").toString(), "TWD");
    a.checkEqual("32", ap[9]("COST")("MC").toInteger(), 300);
    a.checkEqual("33", ap[9]("COST")("T").toInteger(), 3);
    a.checkEqual("34", ap[9]("COST")("D").toInteger(), 16);
    a.checkEqual("35", ap[9]("COST")("M").toInteger(), 35);
    a.checkEqual("36", ap[9]("TECH").toInteger(), 10);
    a.checkEqual("37", ap[9]("SPEED").toInteger(), 9);
    a.checkEqual("38", ap[9]("FUELFACTOR")[0].toInteger(), 0);
    a.checkEqual("39", ap[9]("FUELFACTOR")[1].toInteger(), 100);
    a.checkEqual("40", ap[9]("FUELFACTOR")[2].toInteger(), 400);
    a.checkEqual("41", ap[9]("FUELFACTOR")[9].toInteger(), 8100);
}

/** Test offset 1.
    A: create BeamPacker with firstSlot=1
    E: no dummy element returned */
AFL_TEST("server.play.EnginePacker:offset1", a)
{
    // Input data
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::addTranswarp(*sl);
    game::test::addNovaDrive(*sl);

    // Testee
    server::play::EnginePacker testee(*sl, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("01", ap[4]("NAME").toString(), "Nova Drive 5");
    a.checkEqual("02", ap[8]("NAME").toString(), "Transwarp Drive");
}
