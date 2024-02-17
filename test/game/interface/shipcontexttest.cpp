/**
  *  \file test/game/interface/shipcontexttest.cpp
  *  \brief Test for game::interface::ShipContext
  */

#include "game/interface/shipcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/values.hpp"

namespace {
    void addShipXY(game::map::Ship& sh)
    {
        sh.addShipXYData(game::map::Point(1000, 1000), 1, 100, game::PlayerSet_t(2));
        sh.internalCheck(game::PlayerSet_t(2), 10);
    }
}

/** General tests. */
AFL_TEST("game.interface.ShipContext:basics", a)
{
    const int SHIP_ID = 83;
    const int PLAYER = 5;
    const int ENEMY = 8;
    const int TURN_NR = 10;

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.world().shipProperties().create(SHIP_ID)->setNew(interpreter::World::sp_Comment, interpreter::makeStringValue("note"));

    // Ship list
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    game::test::addGorbie(*shipList);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    root->playerList().create(PLAYER)->setName(game::Player::AdjectiveName, "funny");
    root->playerList().create(ENEMY)->setName(game::Player::AdjectiveName, "evil");

    // Game
    afl::base::Ref<game::Game> g(*new game::Game());

    // Ship with full data
    game::map::Ship& sh = *g->currentTurn().universe().ships().create(SHIP_ID);
    game::map::ShipData sd;
    sd.owner                     = PLAYER;
    sd.friendlyCode              = "xxy";
    sd.warpFactor                = 7;
    sd.waypointDX                = 0;
    sd.waypointDY                = 0;
    sd.x                         = 1000;
    sd.y                         = 2000;
    sd.engineType                = 9;
    sd.hullType                  = game::test::GORBIE_HULL_ID;
    sd.beamType                  = 0;
    sd.numBeams                  = 0;
    sd.numBays                   = 0;
    sd.torpedoType               = 0;
    sd.ammo                      = 0;
    sd.numLaunchers              = 0;
    sd.mission                   = 5;
    sd.primaryEnemy              = ENEMY;
    sd.missionTowParameter       = 0;
    sd.damage                    = 0;
    sd.crew                      = 10;
    sd.colonists                 = 0;
    sd.name                      = "Michail";
    sd.neutronium                = 10;
    sd.tritanium                 = 20;
    sd.duranium                  = 30;
    sd.molybdenum                = 40;
    sd.supplies                  = 0;
    sd.missionInterceptParameter = 0;
    sd.money                     = 0;
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.setPlayability(game::map::Object::Playable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    // Test object
    game::interface::ShipContext testee(SHIP_ID, session, root, g, g->currentTurn(), shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifySerializable(interpreter::TagNode::Tag_Ship, SHIP_ID, afl::base::Nothing);
    a.checkEqual("01. getObject", testee.getObject(), &sh);

    // Verify properties
    // - ship properties
    verif.verifyInteger("CARGO.T", 20);
    verif.verifyInteger("SHIP.CARGO.T", 20);
    verif.verifyString("NAME", "Michail");
    verif.verifyString("SHIP.NAME", "Michail");

    // - hull properties
    verif.verifyInteger("CARGO.MAX", 250);
    verif.verifyInteger("SHIP.CARGO.MAX", 250);

    // - component properties (common hull properties)
    verif.verifyString("HULL", "GORBIE CLASS BATTLECARRIER");
    verif.verifyString("SHIP.HULL", "GORBIE CLASS BATTLECARRIER");

    // - owner
    verif.verifyString("OWNER.ADJ", "funny");
    verif.verifyString("SHIP.OWNER.ADJ", "funny");

    // - enemy
    verif.verifyString("ENEMY.ADJ", "evil");
    verif.verifyString("SHIP.ENEMY.ADJ", "evil");

    // - user-defined
    verif.verifyString("COMMENT", "note");
    verif.verifyString("SHIP.COMMENT", "note");

    // Modify
    verif.setStringValue("NAME", "Eric");
    a.checkEqual("11. getName", sh.getName(), "Eric");
    verif.setIntegerValue("SHIP.MISSION$", 3);
    a.checkEqual("12. getMission", sh.getMission().orElse(-1), 3);

    AFL_CHECK_THROWS(a("21. set CARGO.MAX"), verif.setIntegerValue("CARGO.MAX", 100), interpreter::Error);
    AFL_CHECK_THROWS(a("22. set HULL"),      verif.setStringValue("HULL", "x"), interpreter::Error);
    AFL_CHECK_THROWS(a("23. set OWNER.ADJ"), verif.setStringValue("OWNER.ADJ", "y"), interpreter::Error);
    AFL_CHECK_THROWS(a("24. set ENEMY.ADJ"), verif.setStringValue("ENEMY.ADJ", "z"), interpreter::Error);
    AFL_CHECK_THROWS(a("25. set MARK"),      verif.setIntegerValue("MARK", 1), interpreter::Error);

    verif.setStringValue("COMMENT", "hi");
    a.checkEqual("31. sp_Comment", interpreter::toString(session.world().shipProperties().get(SHIP_ID, interpreter::World::sp_Comment), false), "hi");

    // Call method
    {
        std::auto_ptr<afl::data::Value> p(verif.getValue("SHIP.MARK"));
        interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(p.get());
        a.checkNonNull("41. CallableValue", cv);

        interpreter::test::ValueVerifier vv(*cv, a("Ship.Mark"));
        vv.verifyBasics();
        vv.verifyNotSerializable();

        afl::data::Segment seg;
        interpreter::Process proc(session.world(), "tester", 777);
        cv->call(proc, seg, false);

        a.check("51. isMarked", sh.isMarked());
    }
}

/** Test on empty object. */
AFL_TEST("game.interface.ShipContext:empty", a)
{
    const int SHIP_ID = 84;

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.world().shipProperties().create(SHIP_ID)->setNew(interpreter::World::sp_Comment, interpreter::makeStringValue("note2"));

    // Environment
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    // Ship with no data
    game::map::Ship& sh = *g->currentTurn().universe().ships().create(SHIP_ID);

    // Test object
    game::interface::ShipContext testee(SHIP_ID, session, root, g, g->currentTurn(), shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    a.checkEqual("01. getObject", testee.getObject(), &sh);

    // Verify properties
    verif.verifyNull("CARGO.T");
    verif.verifyNull("SHIP.CARGO.T");
    verif.verifyNull("NAME");
    verif.verifyNull("SHIP.NAME");
    verif.verifyNull("CARGO.MAX");
    verif.verifyNull("SHIP.CARGO.MAX");
    verif.verifyNull("HULL");
    verif.verifyNull("SHIP.HULL");
    verif.verifyNull("OWNER.ADJ");
    verif.verifyNull("SHIP.OWNER.ADJ");
    verif.verifyNull("ENEMY.ADJ");
    verif.verifyNull("SHIP.ENEMY.ADJ");

    // - user-defined
    verif.verifyString("COMMENT", "note2");
    verif.verifyString("SHIP.COMMENT", "note2");

    // Modify
    AFL_CHECK_THROWS(a("11. set NAME"),          verif.setStringValue("NAME", "Eric"), interpreter::Error);
    AFL_CHECK_THROWS(a("12. set SHIP.MISSION$"), verif.setIntegerValue("SHIP.MISSION$", 3), interpreter::Error);

    // - user-defined
    verif.setStringValue("COMMENT", "hi");
    a.checkEqual("21. sp_Comment", interpreter::toString(session.world().shipProperties().get(SHIP_ID, interpreter::World::sp_Comment), false), "hi");
}

/** Test on null object. */
AFL_TEST("game.interface.ShipContext:null", a)
{
    const int SHIP_ID = 85;

    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.world().shipProperties().create(SHIP_ID)->setNew(interpreter::World::sp_Comment, interpreter::makeStringValue("note2"));

    // Environment (with no ship!)
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    // Test object
    game::interface::ShipContext testee(SHIP_ID, session, root, g, g->currentTurn(), shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    a.checkNull("01. getObject", testee.getObject());

    // Verify properties
    verif.verifyNull("CARGO.T");
    verif.verifyNull("SHIP.CARGO.T");
    verif.verifyNull("NAME");
    verif.verifyNull("SHIP.NAME");
    verif.verifyNull("CARGO.MAX");
    verif.verifyNull("SHIP.CARGO.MAX");
    verif.verifyNull("HULL");
    verif.verifyNull("SHIP.HULL");
    verif.verifyNull("OWNER.ADJ");
    verif.verifyNull("SHIP.OWNER.ADJ");
    verif.verifyNull("ENEMY.ADJ");
    verif.verifyNull("SHIP.ENEMY.ADJ");

    // - user-defined also reports as null
    verif.verifyNull("COMMENT");
    verif.verifyNull("SHIP.COMMENT");

    // Modify
    AFL_CHECK_THROWS(a("11. set NAME"),          verif.setStringValue("NAME", "Eric"), interpreter::Error);
    AFL_CHECK_THROWS(a("12. set SHIP.MISSION$"), verif.setIntegerValue("SHIP.MISSION$", 3), interpreter::Error);
    AFL_CHECK_THROWS(a("13. set COMMENT"),       verif.setStringValue("COMMENT", "new"), interpreter::Error);
}

/** Test iteration. */
AFL_TEST("game.interface.ShipContext:iteration", a)
{
    // Session
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Environment
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    // Some ships
    game::map::Universe& univ = g->currentTurn().universe();
    for (int i = 1; i < 50; ++i) {
        univ.ships().create(i);
    }
    addShipXY(*univ.ships().get(10));
    addShipXY(*univ.ships().get(20));
    addShipXY(*univ.ships().get(21));

    // Create
    game::interface::ShipContext testee(10, session, root, g, g->currentTurn(), shipList);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("ID", 10);
    a.check("01. next", testee.next());
    verif.verifyInteger("ID", 20);
    a.check("02. next", testee.next());
    verif.verifyInteger("ID", 21);
    a.check("03. next", !testee.next());
}

/*
 *  Test creation
 */

// Normal case
AFL_TEST("game.interface.ShipContext:create:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());
    addShipXY(*session.getGame()->currentTurn().universe().ships().create(100));

    std::auto_ptr<game::interface::ShipContext> ctx(game::interface::ShipContext::create(100, session, *session.getGame(), session.getGame()->viewpointTurn()));
    a.checkNonNull("ctx", ctx.get());
    interpreter::test::ContextVerifier(*ctx, a).verifyInteger("ID", 100);
}

// Nonexistant ship
AFL_TEST("game.interface.ShipContext:create:no-ship", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());

    std::auto_ptr<game::interface::ShipContext> ctx(game::interface::ShipContext::create(100, session, *session.getGame(), session.getGame()->viewpointTurn()));
    a.checkNull("ctx", ctx.get());
}

// No root
AFL_TEST("game.interface.ShipContext:create:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    session.setShipList(new game::spec::ShipList());
    addShipXY(*session.getGame()->currentTurn().universe().ships().create(100));

    std::auto_ptr<game::interface::ShipContext> ctx(game::interface::ShipContext::create(100, session, *session.getGame(), session.getGame()->viewpointTurn()));
    a.checkNull("ctx", ctx.get());
}

// No ship list
AFL_TEST("game.interface.ShipContext:create:no-shiplist", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setGame(new game::Game());
    addShipXY(*session.getGame()->currentTurn().universe().ships().create(100));

    std::auto_ptr<game::interface::ShipContext> ctx(game::interface::ShipContext::create(100, session, *session.getGame(), session.getGame()->viewpointTurn()));
    a.checkNull("ctx", ctx.get());
}
