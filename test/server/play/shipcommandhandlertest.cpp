/**
  *  \file test/server/play/shipcommandhandlertest.cpp
  *  \brief Test for server::play::ShipCommandHandler
  */

#include "server/play/shipcommandhandler.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "server/play/packerlist.hpp"

using afl::base::Ref;
using afl::data::Segment;
using game::map::Planet;
using game::map::Ship;
using interpreter::Arguments;

namespace {
    /*
     *  Environment
     */
    const int TURN_NR = 10;
    const int PLAYER = 4;
    const int HULL_ID = 5;
    const int X = 1030;
    const int Y = 2700;

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        interpreter::Process proc;
        Ref<game::Root> root;
        Ref<game::Game> g;
        Ref<game::Turn> turn;
        game::map::Configuration mapConfig;
        Ref<game::spec::ShipList> shipList;

        Environment()
            : tx(), fs(), session(tx, fs),
              proc(session.world(), "tester", 777),
              root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0)))),
              g(*new game::Game()),
              turn(g->currentTurn()),
              shipList(*new game::spec::ShipList())
            {
                // Process: push a frame to be able to set CARGO.REMAINDER/BUILD.REMAINDER variables
                interpreter::Process::Frame& f = proc.pushFrame(interpreter::BytecodeObject::create(true), false);
                f.localNames.add("CARGO.REMAINDER");

                // Ship list: create a hull for a ship that can hold 300 cargo, 100 fuel.
                game::spec::Hull& h = *shipList->hulls().create(HULL_ID);
                h.setMaxCargo(300);
                h.setMaxFuel(100);
                h.setMaxCrew(10);
                h.setNumEngines(2);

                // More properties
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
                game::test::addTranswarp(*shipList);

                // Connect everything
                session.setShipList(shipList.asPtr());
                session.setRoot(root.asPtr());
                session.setGame(g.asPtr());
            }
    };

    // Make planet playable with some default data
    void configurePlayablePlanet(Environment& env, Planet& pl)
    {
        // Planet
        game::map::PlanetData pd;
        pd.owner             = PLAYER;
        pd.minedNeutronium   = 50;
        pd.minedTritanium    = 50;
        pd.minedDuranium     = 50;
        pd.minedMolybdenum   = 50;
        pd.colonistClans     = 1200;
        pd.supplies          = 10;
        pd.money             = 15000;
        pd.baseFlag          = 0;

        pl.setPosition(game::map::Point(X, Y));
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        pl.setName("Earth 2");
        pl.setPlayability(game::map::Object::Playable);
        pl.internalCheck(env.mapConfig, game::PlayerSet_t(PLAYER), TURN_NR, env.tx, env.session.log());
    }

    // Make ship playable with default data
    void configurePlayableShip(Environment& /*env*/, Ship& sh)
    {
        game::map::ShipData sd;
        sd.x                 = X;
        sd.y                 = Y;
        sd.waypointDX        = 0;
        sd.waypointDY        = 0;
        sd.owner             = PLAYER;
        sd.hullType          = HULL_ID;
        sd.beamType          = 0;
        sd.numBeams          = 0;
        sd.numBays           = 0;
        sd.torpedoType       = 0;
        sd.ammo              = 0;
        sd.numLaunchers      = 0;
        sd.colonists         = 0;
        sd.neutronium        = 10;
        sd.tritanium         = 10;
        sd.duranium          = 10;
        sd.molybdenum        = 10;
        sd.supplies          = 10;
        sd.money             = 100;
        sd.unload.targetId   = 0;
        sd.transfer.targetId = 0;
        sd.friendlyCode      = "jkl";
        sd.warpFactor        = 9;
        sd.primaryEnemy      = 1;
        sd.name              = "Boat";
        sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);
        sh.setPlayability(game::map::Object::Playable);
    }

    Ship& makeShip(Environment& env, int id)
    {
        return *env.session.getGame()->currentTurn().universe().ships().create(id);
    }

    Planet& makePlanet(Environment& env, int id)
    {
        return *env.session.getGame()->currentTurn().universe().planets().create(id);
    }

    void call(server::play::CommandHandler& testee, String_t cmd, Segment& seg)
    {
        server::play::PackerList list;
        Arguments args(seg, 0, seg.size());
        testee.processCommand(cmd, args, list);
    }
}

/*
 *  Happy path for all commands - test cases partially derived from ShipMethodTest
 */

// Test 'setcomment'
AFL_TEST("server.play.ShipCommandHandler:setcomment", a)
{
    Environment env;
    makeShip(env, 77);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackString("hi there");
    call(testee, "setcomment", args);

    a.checkEqual("comment", interpreter::toString(env.session.world().shipProperties().get(77, interpreter::World::sp_Comment), false), "hi there");
}

// Test 'setfcode'
AFL_TEST("server.play.ShipCommandHandler:setfcode", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackString("abc");
    call(testee, "setfcode", args);

    a.checkEqual("getFriendlyCode", sh.getFriendlyCode().orElse(""), "abc");
}

// Test 'setname'
AFL_TEST("server.play.ShipCommandHandler:setname", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackString("USS Honk");
    call(testee, "setname", args);

    a.checkEqual("getName", sh.getName(), "USS Honk");
}

// Test 'setwaypoint'
AFL_TEST("server.play.ShipCommandHandler:setwaypoint", a)
{
    Environment env;
    Ship& sh = makeShip(env, 66);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 66);
    Segment args;
    args.pushBackInteger(X - 20);
    args.pushBackInteger(Y + 30);
    call(testee, "setwaypoint", args);

    a.checkEqual("getWaypointDX", sh.getWaypointDX().orElse(-1), -20);
    a.checkEqual("getWaypointDY", sh.getWaypointDY().orElse(-1), +30);
}

// Test 'setenemy'
AFL_TEST("server.play.ShipCommandHandler:setenemy", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);
    env.root->playerList().create(3);       // Defines valid value

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackInteger(3);
    call(testee, "setenemy", args);

    a.checkEqual("getPrimaryEnemy", sh.getPrimaryEnemy().orElse(-1), 3);
}

// Test 'setspeed'
AFL_TEST("server.play.ShipCommandHandler:setspeed", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackInteger(3);
    call(testee, "setspeed", args);

    a.checkEqual("getWarpFactor", sh.getWarpFactor().orElse(-1), 3);
}

// Test 'setmission'
AFL_TEST("server.play.ShipCommandHandler:setmission", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackInteger(40);
    args.pushBackInteger(10);
    args.pushBackInteger(900);
    call(testee, "setmission", args);

    a.checkEqual("getMission",         sh.getMission().orElse(-1), 40);
    a.checkEqual("InterceptParameter", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 10);
    a.checkEqual("TowParameter",       sh.getMissionParameter(game::TowParameter).orElse(-1), 900);
}

// Test 'cargotransfer'
AFL_TEST("server.play.ShipCommandHandler:cargotransfer", a)
{
    Environment env;
    Ship& from = makeShip(env, 55);
    Ship& to   = makeShip(env, 22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);

    server::play::ShipCommandHandler testee(env.session, 55);
    Segment args;
    args.pushBackString("5t");
    args.pushBackInteger(22);
    call(testee, "cargotransfer", args);

    a.checkEqual("from Tritanium", from.getCargo(game::Element::Tritanium).orElse(-1), 5);
    a.checkEqual("to Tritanium",   to.getCargo(game::Element::Tritanium).orElse(-1), 15);
}

// Test 'cargounload'
AFL_TEST("server.play.ShipCommandHandler:cargounload", a)
{
    Environment env;
    Ship& sh = makeShip(env, 55);
    Planet& pl = makePlanet(env, 77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);

    server::play::ShipCommandHandler testee(env.session, 55);
    Segment args;
    args.pushBackString("5t");
    call(testee, "cargounload", args);

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 5);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 55);
}

// Test 'cargoupload'
AFL_TEST("server.play.ShipCommandHandler:cargoupload", a)
{
    Environment env;
    Ship& sh = makeShip(env, 55);
    Planet& pl = makePlanet(env, 77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);

    server::play::ShipCommandHandler testee(env.session, 55);
    Segment args;
    args.pushBackString("5t");
    call(testee, "cargoupload", args);

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 15);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 45);
}

/*
 *  Error cases
 */

// Error: bad verb
AFL_TEST("server.play.ShipCommandHandler:error:verb", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "doabarrelroll", args), std::exception);
}

// Error: bad verb (case sensitive!)
AFL_TEST("server.play.ShipCommandHandler:error:verb:2", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(testee, "SetSpeed", args), std::exception);
}

// Error: bad type
AFL_TEST("server.play.ShipCommandHandler:error:type", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackString("3");
    AFL_CHECK_THROWS(a, call(testee, "setspeed", args), std::exception);
}

// Error: arity
AFL_TEST("server.play.ShipCommandHandler:error:arity", a)
{
    Environment env;
    Ship& sh = makeShip(env, 77);
    configurePlayableShip(env, sh);

    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "setspeed", args), std::exception);
}

// Error: no ship
AFL_TEST("server.play.ShipCommandHandler:error:no-ship", a)
{
    Environment env;
    server::play::ShipCommandHandler testee(env.session, 77);
    Segment args;
    args.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(testee, "setspeed", args), std::exception);
}
