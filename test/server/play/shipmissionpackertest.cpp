/**
  *  \file test/server/play/shipmissionpackertest.cpp
  *  \brief Test for server::play::ShipMissionPacker
  */

#include <stdexcept>
#include "server/play/shipmissionpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using afl::data::Access;
using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::Game;
using game::HostVersion;
using game::Root;
using game::Session;
using game::map::Ship;
using game::spec::Mission;
using game::spec::MissionList;
using game::spec::ShipList;
using server::play::ShipMissionPacker;

/** General test. */
AFL_TEST("server.play.ShipMissionPacker:general", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());

    // Missions
    MissionList& ml = session.getShipList()->missions();
    Mission m1(10, "p#,first");
    m1.setHotkey('f');
    m1.setParameterName(game::TowParameter, "t1");
    ml.addMission(m1);

    Mission m2(20, "s*,second");
    m2.setGroup("sg");
    m2.setParameterName(game::InterceptParameter, "i2");
    ml.addMission(m2);

    Mission m3(30, "-5,third");
    m3.setHotkey('t');
    ml.addMission(m3);

    // Ship
    Ship& sh = *session.getGame()->currentTurn().universe().ships().create(20);
    sh.setOwner(5);
    sh.setPlayability(game::map::Object::Playable);

    // Test
    ShipMissionPacker testee(session, 20);
    a.checkEqual("01. getName", testee.getName(), "shipmsn20");

    std::auto_ptr<server::Value_t> value(testee.buildValue());
    Access ap(value.get());
    a.checkEqual("11. length", ap.getArraySize(), 2U);

    a.checkEqual("21. name",  ap[0]("name").toString(), "first");
    a.checkEqual("22. group", ap[0]("group").toString(), "");
    a.checkEqual("23. key",   ap[0]("key").toString(), "f");
    a.checkEqual("24. iarg",  ap[0]("iarg").toInteger(), 0);
    a.checkEqual("25. targ",  ap[0]("targ").toInteger(), 2);
    a.checkEqual("26. iname", ap[0]("iname").toString(), "");
    a.checkEqual("27. tname", ap[0]("tname").toString(), "t1");

    a.checkEqual("31. name",  ap[1]("name").toString(), "second");
    a.checkEqual("32. group", ap[1]("group").toString(), "sg");
    a.checkEqual("33. key",   ap[1]("key").toString(), "a");       // Automatically assigned!
    a.checkEqual("34. iarg",  ap[1]("iarg").toInteger(), 3);
    a.checkEqual("35. targ",  ap[1]("targ").toInteger(), 0);
    a.checkEqual("36. iname", ap[1]("iname").toString(), "i2");
    a.checkEqual("37. tname", ap[1]("tname").toString(), "");
}

/** Test all the parameter types. */
AFL_TEST("server.play.ShipMissionPacker:parameter-types", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());

    // Missions
    MissionList& ml = session.getShipList()->missions();
    ml.addMission(Mission(1, ",none"));
    ml.addMission(Mission(2, "i#,none"));
    ml.addMission(Mission(3, "p#,planet"));
    ml.addMission(Mission(4, "s#,ship"));
    ml.addMission(Mission(5, "h#,here"));
    ml.addMission(Mission(6, "b#,base"));
    ml.addMission(Mission(7, "y#,player"));
    ml.addMission(Mission(8, "os#,own ship"));
    ml.addMission(Mission(9, "!y#,other player"));

    // Ship
    Ship& sh = *session.getGame()->currentTurn().universe().ships().create(20);
    sh.setOwner(5);
    sh.setPlayability(game::map::Object::Playable);

    // Test
    std::auto_ptr<server::Value_t> value(ShipMissionPacker(session, 20).buildValue());
    Access ap(value.get());
    a.checkEqual("11", ap.getArraySize(), 9U);

    a.checkEqual("21", ap[0]("targ").toInteger(), 0);
    a.checkEqual("22", ap[1]("targ").toInteger(), 1);
    a.checkEqual("23", ap[2]("targ").toInteger(), 2);
    a.checkEqual("24", ap[3]("targ").toInteger(), 3);
    a.checkEqual("25", ap[4]("targ").toInteger(), 4);
    a.checkEqual("26", ap[5]("targ").toInteger(), 5);
    a.checkEqual("27", ap[6]("targ").toInteger(), 6);
    a.checkEqual("28", ap[7]("targ").toInteger(), 19);
    a.checkEqual("29", ap[8]("targ").toInteger(), 38);
}

/** Error: session not populated. */
AFL_TEST("server.play.ShipMissionPacker:error:empty-session", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    AFL_CHECK_THROWS(a, ShipMissionPacker(session, 20).buildValue(), std::exception);
}


/** Error: ship not present. */
AFL_TEST("server.play.ShipMissionPacker:error:no-ship", a)
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);

    session.setGame(new Game());
    session.setRoot(game::test::makeRoot(HostVersion()).asPtr());
    session.setShipList(new ShipList());
    AFL_CHECK_THROWS(a, ShipMissionPacker(session, 20).buildValue(), std::exception);
}

