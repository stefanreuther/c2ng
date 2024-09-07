/**
  *  \file test/server/play/torpedopackertest.cpp
  *  \brief Test for server::play::TorpedoPacker
  */

#include "server/play/torpedopacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include <memory>

/** Simple functionality test.
    A: create ship list; create TorpedoPacker
    E: correct values for all properties */
AFL_TEST("server.play.TorpedoPacker:basics", a)
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardTorpedoes(*sl);
    sl->launchers().get(1)->setShortName("Mk1");
    sl->launchers().get(10)->setShortName("Mk8");

    // Testee
    server::play::TorpedoPacker testee(*sl, *r, 0);
    a.checkEqual("01. getName", testee.getName(), "torp");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    // Standard list has 10 torpedoes, so this needs to be 11 elements (including dummy)
    a.checkEqual("11. getArraySize", ap.getArraySize(), 11U);
    a.check("12. result", ap[0].isNull());
    a.check("13. result", !ap[1].isNull());
    a.check("14. result", !ap[10].isNull());

    a.checkEqual("21", ap[1]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("21a", ap[1]("NAME.SHORT").toString(), "Mk1");
    a.checkEqual("22", ap[1]("TORPCOST")("MC").toInteger(), 1);
    a.checkEqual("23", ap[1]("TORPCOST")("T").toInteger(), 1);
    a.checkEqual("24", ap[1]("TORPCOST")("D").toInteger(), 1);
    a.checkEqual("25", ap[1]("TORPCOST")("M").toInteger(), 1);
    a.checkEqual("26", ap[1]("TUBECOST")("MC").toInteger(), 1);
    a.checkEqual("27", ap[1]("TUBECOST")("T").toInteger(), 1);
    a.checkEqual("28", ap[1]("TUBECOST")("D").toInteger(), 1);
    a.checkEqual("29", ap[1]("TUBECOST")("M").toInteger(), 0);
    a.checkEqual("30", ap[1]("DAMAGE1").toInteger(), 5);
    a.checkEqual("31", ap[1]("KILL1").toInteger(), 4);
    a.checkEqual("32", ap[1]("DAMAGE").toInteger(), 10);
    a.checkEqual("33", ap[1]("KILL").toInteger(), 8);
    a.checkEqual("34", ap[1]("TECH").toInteger(), 1);
    a.checkEqual("35", ap[1]("MASS").toInteger(), 2);

    a.checkEqual("41", ap[10]("NAME").toString(), "Mark 8 Photon");
    a.checkEqual("41a", ap[10]("NAME.SHORT").toString(), "Mk8");
    a.checkEqual("42", ap[10]("TORPCOST")("MC").toInteger(), 54);
    a.checkEqual("43", ap[10]("TORPCOST")("T").toInteger(), 1);
    a.checkEqual("44", ap[10]("TORPCOST")("D").toInteger(), 1);
    a.checkEqual("45", ap[10]("TORPCOST")("M").toInteger(), 1);
    a.checkEqual("46", ap[10]("TUBECOST")("MC").toInteger(), 190);
    a.checkEqual("47", ap[10]("TUBECOST")("T").toInteger(), 1);
    a.checkEqual("48", ap[10]("TUBECOST")("D").toInteger(), 1);
    a.checkEqual("49", ap[10]("TUBECOST")("M").toInteger(), 9);
    a.checkEqual("50", ap[10]("DAMAGE1").toInteger(), 55);
    a.checkEqual("51", ap[10]("KILL1").toInteger(), 35);
    a.checkEqual("52", ap[10]("DAMAGE").toInteger(), 110);
    a.checkEqual("53", ap[10]("KILL").toInteger(), 70);
    a.checkEqual("54", ap[10]("TECH").toInteger(), 10);
    a.checkEqual("55", ap[10]("MASS").toInteger(), 3);
}

/** Test offset 1.
    A: create TorpedoPacker with firstSlot=1
    E: no dummy element returned */
AFL_TEST("server.play.TorpedoPacker:offset1", a)
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    game::test::initStandardTorpedoes(*sl);

    // Testee
    server::play::TorpedoPacker testee(*sl, *r, 1);
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("01. getArraySize", ap.getArraySize(), 10U);
    a.checkEqual("02", ap[0]("NAME").toString(), "Mark 1 Photon");
    a.checkEqual("03", ap[9]("NAME").toString(), "Mark 8 Photon");
}
