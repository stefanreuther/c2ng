/**
  *  \file test/server/play/beampackertest.cpp
  *  \brief Test for server::play::BeamPacker
  */

#include "server/play/beampacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include <memory>

/** Simple functionality test.
    A: create ship list; create BeamPacker
    E: correct values for all properties */
AFL_TEST("server.play.BeamPacker:basics", a)
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardBeams(*sl);
    sl->beams().get(1)->setShortName("Las");
    sl->beams().get(10)->setShortName("HPh");

    // Testee
    server::play::BeamPacker testee(*sl, *r, 0);
    a.checkEqual("01. getName", testee.getName(), "beam");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Standard list has 10 beams, so this needs to be 11 elements (including dummy)
    a.checkEqual("11. getArraySize", ap.getArraySize(), 11U);
    a.check("12. result", ap[0].isNull());
    a.check("13. result", !ap[1].isNull());
    a.check("14. result", !ap[10].isNull());

    // Verify all attributes of #1
    a.checkEqual("21", ap[1]("NAME").toString(), "Laser");
    a.checkEqual("21a", ap[1]("NAME.SHORT").toString(), "Las");
    a.checkEqual("22", ap[1]("COST")("MC").toInteger(), 1);
    a.checkEqual("23", ap[1]("COST")("T").toInteger(), 1);
    a.checkEqual("24", ap[1]("COST")("D").toInteger(), 0);
    a.checkEqual("25", ap[1]("COST")("M").toInteger(), 0);
    a.checkEqual("26", ap[1]("DAMAGE").toInteger(), 3);
    a.checkEqual("27", ap[1]("KILL").toInteger(), 10);
    a.checkEqual("28", ap[1]("TECH").toInteger(), 1);
    a.checkEqual("29", ap[1]("MASS").toInteger(), 1);

    // Verify all attributes of #10
    a.checkEqual("31", ap[10]("NAME").toString(), "Heavy Phaser");
    a.checkEqual("31a", ap[10]("NAME.SHORT").toString(), "HPh");
    a.checkEqual("32", ap[10]("COST")("MC").toInteger(), 54);
    a.checkEqual("33", ap[10]("COST")("T").toInteger(), 1);
    a.checkEqual("34", ap[10]("COST")("D").toInteger(), 12);
    a.checkEqual("35", ap[10]("COST")("M").toInteger(), 55);
    a.checkEqual("36", ap[10]("DAMAGE").toInteger(), 45);
    a.checkEqual("37", ap[10]("KILL").toInteger(), 35);
    a.checkEqual("38", ap[10]("TECH").toInteger(), 10);
    a.checkEqual("39", ap[10]("MASS").toInteger(), 6);
}

/** Test offset 1.
    A: create BeamPacker with firstSlot=1
    E: no dummy element returned */
AFL_TEST("server.play.BeamPacker:offset1", a)
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardBeams(*sl);

    // Testee
    server::play::BeamPacker testee(*sl, *r, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("01. getArraySize", ap.getArraySize(), 10U);
    a.checkEqual("02", ap[0]("NAME").toString(), "Laser");
    a.checkEqual("03", ap[9]("NAME").toString(), "Heavy Phaser");
}
