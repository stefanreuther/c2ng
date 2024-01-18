/**
  *  \file test/game/interface/planetcontexttest.cpp
  *  \brief Test for game::interface::PlanetContext
  */

#include "game/interface/planetcontext.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/basedata.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

namespace {
    const int PLANET_ID = 42;
    const int PLAYER = 14;
    const int TURN_NR = 22;

    void addPlanetXY(game::Session& session, game::Game& g, game::Id_t id, int x, int y)
    {
        game::map::Planet& pl = *g.currentTurn().universe().planets().create(id);
        pl.setPosition(game::map::Point(x, y));
        pl.internalCheck(g.mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, session.translator(), session.log());
    }
}

/** Basic tests with normal planet. */
AFL_TEST("game.interface.PlanetContext:basics", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());
    session.world().planetProperties().create(PLANET_ID)->setNew(interpreter::World::pp_Comment, interpreter::makeStringValue("note"));

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    root->playerList().create(PLAYER)->setName(game::Player::AdjectiveName, "funny");

    afl::base::Ref<game::Game> g(*new game::Game());

    // Planet with exemplary data
    game::map::Planet& pl = *g->currentTurn().universe().planets().create(PLANET_ID);
    game::map::PlanetData pd;
    pd.owner             = PLAYER;
    pd.friendlyCode      = "jkl";
    pd.numMines          = 20;
    pd.numFactories      = 30;
    pd.numDefensePosts   = 15;
    pd.colonistTax       = 7;
    pd.colonistClans     = 1200;

    game::map::BaseData bd;
    bd.numBaseDefensePosts = 10;
    bd.engineStorage.set(3, 10);
    bd.mission = 2;

    pl.setPosition(game::map::Point(1030, 2700));
    pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
    pl.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
    pl.setName("Earth 2");
    pl.setPlayability(game::map::Object::Playable);
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, tx, session.log());

    // Testee
    game::interface::PlanetContext testee(PLANET_ID, session, root, g);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifySerializable(interpreter::TagNode::Tag_Planet, PLANET_ID, afl::base::Nothing);
    verif.verifyTypes();
    a.checkEqual("01. getObject", testee.getObject(), &pl);

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Planet(42)");
    verif.verifyInteger("FACTORIES", 30);
    verif.verifyInteger("PLANET.FACTORIES", 30);
    verif.verifyString ("FCODE", "jkl");
    verif.verifyString ("PLANET.FCODE", "jkl");
    verif.verifyInteger("DEFENSE.BASE", 10);
    verif.verifyInteger("PLANET.DEFENSE.BASE", 10);
    verif.verifyString ("OWNER.ADJ", "funny");
    verif.verifyString ("PLANET.OWNER.ADJ", "funny");
    verif.verifyString ("COMMENT", "note");
    verif.verifyString ("PLANET.COMMENT", "note");

    // Modification
    AFL_CHECK_SUCCEEDS(a("21. set COLONISTS.TAX"), verif.setIntegerValue("COLONISTS.TAX", 9));
    a.checkEqual("22. getColonistTax", pl.getColonistTax().orElse(-1), 9);

    AFL_CHECK_SUCCEEDS(a("31. set MISSION$"), verif.setIntegerValue("MISSION$", 5));
    a.checkEqual("32. getBaseMission", pl.getBaseMission().orElse(-1), 5);

    AFL_CHECK_SUCCEEDS(a("41. set PLANET.COMMENT"), verif.setStringValue("PLANET.COMMENT", "updated"));
    a.checkEqual("42. pp_Comment", interpreter::toString(session.world().planetProperties().get(PLANET_ID, interpreter::World::pp_Comment), false), "updated");

    AFL_CHECK_THROWS(a("51. set OWNER$"),     verif.setIntegerValue("OWNER$", 3), interpreter::Error);
    AFL_CHECK_THROWS(a("52. set SETMISSION"), verif.setIntegerValue("SETMISSION", 3), interpreter::Error);

    // Method
    {
        std::auto_ptr<afl::data::Value> p(verif.getValue("SETMISSION"));
        interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(p.get());
        a.checkNonNull("61. cv", cv);

        interpreter::test::ValueVerifier vv(*cv, a("SetMission"));
        vv.verifyBasics();
        vv.verifyNotSerializable();

        afl::data::Segment seg;
        seg.pushBackInteger(3);

        interpreter::Process proc(session.world(), "tester", 777);
        cv->call(proc, seg, false);

        a.checkEqual("71. getBaseMission", pl.getBaseMission().orElse(-1), 3);
    }
}

/** Test behaviour on empty planet. */
AFL_TEST("game.interface.PlanetContext:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setShipList(new game::spec::ShipList());

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    // Planet with no data
    game::map::Planet& pl = *g->currentTurn().universe().planets().create(PLANET_ID);

    // Testee
    game::interface::PlanetContext testee(PLANET_ID, session, root, g);
    interpreter::test::ContextVerifier verif(testee, a);
    a.checkEqual("01. getObject", testee.getObject(), &pl);

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Planet(42)");
    verif.verifyNull("FACTORIES");
    verif.verifyNull("PLANET.FACTORIES");
    verif.verifyNull("FCODE");
    verif.verifyNull("PLANET.FCODE");
    verif.verifyNull("DEFENSE.BASE");
    verif.verifyNull("PLANET.DEFENSE.BASE");
    verif.verifyNull("OWNER.ADJ");
    verif.verifyNull("PLANET.OWNER.ADJ");
    verif.verifyNull("COMMENT");
    verif.verifyNull("PLANET.COMMENT");

    // Modification
    AFL_CHECK_THROWS(a("21. set COLONISTS.TAX"), verif.setIntegerValue("COLONISTS.TAX", 9), interpreter::Error);
    AFL_CHECK_THROWS(a("22. set MISSION$"),      verif.setIntegerValue("MISSION$", 5), interpreter::Error);
    AFL_CHECK_THROWS(a("23. set OWNER$"),        verif.setIntegerValue("OWNER$", 3), interpreter::Error);
    AFL_CHECK_THROWS(a("24. set SETMISSION"),    verif.setIntegerValue("SETMISSION", 3), interpreter::Error);

    // User-defined property can be assigned as long as Planet object exists
    AFL_CHECK_SUCCEEDS(a("31. set COMMENT"), verif.setStringValue("COMMENT", "mod"));
    a.checkEqual("32. pp_Comment", interpreter::toString(session.world().planetProperties().get(PLANET_ID, interpreter::World::pp_Comment), false), "mod");
}

/** Test behaviour on nonexistant (null) planet. */
AFL_TEST("game.interface.PlanetContext:null", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    // Testee
    game::interface::PlanetContext testee(PLANET_ID, session, root, g);
    interpreter::test::ContextVerifier verif(testee, a);
    a.checkNull("01. getObject", testee.getObject());

    // Specific properties
    a.checkEqual("11. toString", testee.toString(true), "Planet(42)");
    verif.verifyNull("FACTORIES");
    verif.verifyNull("PLANET.FACTORIES");
    verif.verifyNull("FCODE");
    verif.verifyNull("PLANET.FCODE");
    verif.verifyNull("DEFENSE.BASE");
    verif.verifyNull("PLANET.DEFENSE.BASE");
    verif.verifyNull("OWNER.ADJ");
    verif.verifyNull("PLANET.OWNER.ADJ");
    verif.verifyNull("COMMENT");
    verif.verifyNull("PLANET.COMMENT");

    // Modification
    AFL_CHECK_THROWS(a("21. set COLONISTS.TAX"), verif.setIntegerValue("COLONISTS.TAX", 9), interpreter::Error);
    AFL_CHECK_THROWS(a("22. set MISSION$"),      verif.setIntegerValue("MISSION$", 5), interpreter::Error);
    AFL_CHECK_THROWS(a("23. set OWNER$"),        verif.setIntegerValue("OWNER$", 3), interpreter::Error);
    AFL_CHECK_THROWS(a("24. set SETMISSION"),    verif.setIntegerValue("SETMISSION", 3), interpreter::Error);

    // User-defined property can not be assigned if Planet object does not exist
    AFL_CHECK_THROWS(a("31. set COMMENT"), verif.setStringValue("COMMENT", "mod"), interpreter::Error);
}

/** Test iteration behaviour. */
AFL_TEST("game.interface.PlanetContext:iteration", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    afl::base::Ref<game::Game> g(*new game::Game());

    addPlanetXY(session, *g, 100, 1000, 1020);
    addPlanetXY(session, *g, 200, 1200, 1010);
    addPlanetXY(session, *g, 250, 1300, 1000);

    game::interface::PlanetContext testee(100, session, root, g);
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyInteger("ID", 100);
    a.check("01. next", testee.next());
    verif.verifyInteger("ID", 200);
    a.check("02. next", testee.next());
    verif.verifyInteger("ID", 250);
    a.check("03. next", !testee.next());
}

/*
 *  Factory function
 */

// Normal case
AFL_TEST("game.interface.PlanetContext:create:normal", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setGame(new game::Game());
    addPlanetXY(session, *session.getGame(), 100, 1000, 1020);

    std::auto_ptr<game::interface::PlanetContext> ctx(game::interface::PlanetContext::create(100, session));
    a.checkNonNull("ctx", ctx.get());
    interpreter::test::ContextVerifier(*ctx, a).verifyInteger("ID", 100);
}

// Nonexistant planet
AFL_TEST("game.interface.PlanetContext:create:no-planet", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setGame(new game::Game());

    std::auto_ptr<game::interface::PlanetContext> ctx(game::interface::PlanetContext::create(100, session));
    a.checkNull("ctx", ctx.get());
}

// No root
AFL_TEST("game.interface.PlanetContext:create:no-root", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setGame(new game::Game());
    addPlanetXY(session, *session.getGame(), 100, 1000, 1020);

    std::auto_ptr<game::interface::PlanetContext> ctx(game::interface::PlanetContext::create(100, session));
    a.checkNull("ctx", ctx.get());
}

// No game
AFL_TEST("game.interface.PlanetContext:create:no-game", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());

    std::auto_ptr<game::interface::PlanetContext> ctx(game::interface::PlanetContext::create(100, session));
    a.checkNull("ctx", ctx.get());
}
