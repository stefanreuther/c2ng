/**
  *  \file test/game/interface/shipmethodtest.cpp
  *  \brief Test for game::interface::ShipMethod
  */

#include "game/interface/shipmethod.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using afl::base::Ref;
using game::map::Planet;
using game::map::Ship;

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

                // Session: connect ship list (no need to connect root, game; they're not supposed to be taken from session!)
                // session.setShipList(shipList.asPtr());
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

    // Add playable starbase with some default data to planet
    void configurePlayableBase(Environment& env, Planet& pl)
    {
        game::map::BaseData bd;
        bd.numBaseDefensePosts           = 10;
        bd.damage                        = 0;
        bd.techLevels[game::HullTech]    = 1;
        bd.techLevels[game::EngineTech]  = 1;
        bd.techLevels[game::BeamTech]    = 1;
        bd.techLevels[game::TorpedoTech] = 1;
        bd.numFighters                   = 10;
        bd.shipyardId                    = 0;
        bd.shipyardAction                = 0;
        bd.mission                       = 0;

        pl.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
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

    void call(Environment& env, Ship& sh, game::interface::ShipMethod m, afl::data::Segment& seg)
    {
        interpreter::Arguments args(seg, 0, seg.size());
        callShipMethod(sh, m, args, env.proc, env.session, *env.root, env.mapConfig, *env.shipList, *env.turn);
    }
}

/*
 *  ismMark, ismUnmark
 */
AFL_TEST("game.interface.ShipMethod:ismMark", a)
{
    Environment env;
    Ship sh(77);
    a.check("01. isMarked", !sh.isMarked());

    // Mark
    {
        afl::data::Segment seg;
        call(env, sh, game::interface::ismMark, seg);
        a.check("11. isMarked", sh.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        call(env, sh, game::interface::ismUnmark, seg);
        a.check("21. isMarked", !sh.isMarked());
    }

    // Mark True
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        call(env, sh, game::interface::ismMark, seg);
        a.check("31. isMarked", sh.isMarked());
    }

    // Mark False
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        call(env, sh, game::interface::ismMark, seg);
        a.check("41. isMarked", !sh.isMarked());
    }
}

/*
 *  ismSetComment
 */

AFL_TEST("game.interface.ShipMethod:ismSetComment", a)
{
    Environment env;
    Ship sh(77);

    // Set comment
    {
        afl::data::Segment seg;
        seg.pushBackString("hi there");
        call(env, sh, game::interface::ismSetComment, seg);
        a.checkEqual("01. sp_Comment", interpreter::toString(env.session.world().shipProperties().get(77, interpreter::World::sp_Comment), false), "hi there");
    }

    // Null does not change the value
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetComment, seg);
        a.checkEqual("11. sp_Comment", interpreter::toString(env.session.world().shipProperties().get(77, interpreter::World::sp_Comment), false), "hi there");
    }

    // Arity error
    {
        afl::data::Segment seg;
        AFL_CHECK_THROWS(a("21. arity error"), call(env, sh, game::interface::ismSetComment, seg), interpreter::Error);
    }
}


/*
 *  ismSetFCode
 */

// Set friendly code
AFL_TEST("game.interface.ShipMethod:ismSetFCode:normal", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackString("abc");
    call(env, sh, game::interface::ismSetFCode, seg);
    a.checkEqual("getFriendlyCode", sh.getFriendlyCode().orElse(""), "abc");
}

// Null does not change the value
AFL_TEST("game.interface.ShipMethod:ismSetFCode:null", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetFCode, seg);
    a.checkEqual("getFriendlyCode", sh.getFriendlyCode().orElse(""), "jkl");
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetFCode:error:arity", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetFCode, seg), interpreter::Error);
}

// Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
AFL_TEST("game.interface.ShipMethod:ismSetFCode:error:not-played", a)
{
    Environment env;
    Ship sh(77);

    afl::data::Segment seg;
    seg.pushBackString("abc");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetFCode, seg), interpreter::Error);
}

/*
 *  ismSetEnemy
 */

// Success case
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:normal", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    env.root->playerList().create(3);       // Defines valid value

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    call(env, sh, game::interface::ismSetEnemy, seg);
    a.checkEqual("getPrimaryEnemy", sh.getPrimaryEnemy().orElse(-1), 3);
}

// Null
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:null", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetEnemy, seg);
    a.checkEqual("getPrimaryEnemy", sh.getPrimaryEnemy().orElse(-1), 1);   // unchanged
}

// Range error, specified value is not a valid race
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:error:range", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    env.root->playerList().create(3);       // Defines valid value

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:error:arity", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:error:type", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    env.root->playerList().create(3);       // Defines valid value

    afl::data::Segment seg;
    seg.pushBackString("3");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetEnemy:error:not-played", a)
{
    Environment env;
    Ship sh(77);
    env.root->playerList().create(3);       // Defines valid value

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
}

/*
 *  ismSetSpeed
 */

// Success case
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:normal", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    call(env, sh, game::interface::ismSetSpeed, seg);
    a.checkEqual("getWarpFactor", sh.getWarpFactor().orElse(-1), 3);
}

// Null
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:null", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetSpeed, seg);
    a.checkEqual("getWarpFactor", sh.getWarpFactor().orElse(-1), 9);
}

// Range error
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:error:range", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(14);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:error:arity", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:error:type", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackString("3");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetSpeed:error:not-played", a)
{
    Environment env;
    Ship sh(77);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
}

/*
 *  ismSetName
 */


// Success case
AFL_TEST("game.interface.ShipMethod:ismSetName:normal", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackString("Yacht");
    call(env, sh, game::interface::ismSetName, seg);
    a.checkEqual("getName", sh.getName(), "Yacht");
}

// Null
AFL_TEST("game.interface.ShipMethod:ismSetName:null", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetName, seg);
    a.checkEqual("getName", sh.getName(), "Boat");
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetName:error:arity", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetName, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetName:error:not-played", a)
{
    Environment env;
    Ship sh(77);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetName, seg), interpreter::Error);
}

// Target only - name can be changed!
AFL_TEST("game.interface.ShipMethod:ismSetName:target", a)
{
    Environment env;
    Ship sh(77);
    sh.addShipXYData(game::map::Point(1000, 1000), PLAYER+1, 100, game::PlayerSet_t(PLAYER));
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    afl::data::Segment seg;
    seg.pushBackString("Spy");
    call(env, sh, game::interface::ismSetName, seg);
    a.checkEqual("getName", sh.getName(), "Spy");
}

/*
 *  ismSetMission
 */

// Success case
AFL_TEST("game.interface.ShipMethod:ismSetMission:normal", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(40);
    seg.pushBackInteger(10);
    seg.pushBackInteger(900);
    call(env, sh, game::interface::ismSetMission, seg);
    a.checkEqual("getMission",         sh.getMission().orElse(-1), 40);
    a.checkEqual("InterceptParameter", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 10);
    a.checkEqual("TowParameter",       sh.getMissionParameter(game::TowParameter).orElse(-1), 900);
}

// Null mission - command will be ignored
AFL_TEST("game.interface.ShipMethod:ismSetMission:null-mission", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    sh.setMission(1, 2, 3);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(10);
    seg.pushBackInteger(900);
    call(env, sh, game::interface::ismSetMission, seg);
    a.checkEqual("getMission",         sh.getMission().orElse(-1), 1);
    a.checkEqual("InterceptParameter", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 2);
    a.checkEqual("TowParameter",       sh.getMissionParameter(game::TowParameter).orElse(-1), 3);
}

// Null parameter - will be set to 0
AFL_TEST("game.interface.ShipMethod:ismSetMission:null-parameter", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    sh.setMission(1, 2, 3);

    afl::data::Segment seg;
    seg.pushBackInteger(40);
    seg.pushBackNew(0);
    seg.pushBackInteger(900);
    call(env, sh, game::interface::ismSetMission, seg);
    a.checkEqual("getMission",         sh.getMission().orElse(-1), 40);
    a.checkEqual("InterceptParameter", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 0);
    a.checkEqual("TowParameter",       sh.getMissionParameter(game::TowParameter).orElse(-1), 900);
}

// Only mission number given - parameters will be set to 0
AFL_TEST("game.interface.ShipMethod:ismSetMission:mission-only", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);
    sh.setMission(1, 2, 3);

    afl::data::Segment seg;
    seg.pushBackInteger(40);
    call(env, sh, game::interface::ismSetMission, seg);
    a.checkEqual("getMission",         sh.getMission().orElse(-1), 40);
    a.checkEqual("InterceptParameter", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 0);
    a.checkEqual("TowParameter",       sh.getMissionParameter(game::TowParameter).orElse(-1), 0);
}

// Rejected because ship is fleet member
AFL_TEST("game.interface.ShipMethod:ismSetMission:error:fleet", a)
{
    Environment env;
    Ship& sh     = *env.turn->universe().ships().create(66);
    Ship& leader = *env.turn->universe().ships().create(77);
    Ship& target = *env.turn->universe().ships().create(88);
    configurePlayableShip(env, sh);
    configurePlayableShip(env, leader);
    configurePlayableShip(env, target);
    leader.setFleetNumber(77);
    leader.setMission(8, 88, 0);
    sh.setFleetNumber(77);
    sh.setMission(8, 88, 0);
    env.shipList->missions().addMission(game::spec::Mission(8, "!is*,Intercept"));

    afl::data::Segment seg;
    seg.pushBackInteger(40);
    seg.pushBackInteger(10);
    seg.pushBackInteger(900);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetMission, seg), game::Exception);
}

// Range error
AFL_TEST("game.interface.ShipMethod:ismSetMission:error:range", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(-40);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismSetMission:error:type", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetMission:error:arity", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetMission:error:not-played", a)
{
    Environment env;
    Ship sh(77);

    afl::data::Segment seg;
    seg.pushBackInteger(40);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
}

/*
 *  ismFixShip
 */

// Normal case
AFL_TEST("game.interface.ShipMethod:ismFixShip:normal", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    call(env, sh, game::interface::ismFixShip, seg);
    a.checkEqual("getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    a.checkEqual("getBaseShipyardId",     pl.getBaseShipyardId().orElse(-1), 77);
}

// Not at planet
AFL_TEST("game.interface.ShipMethod:ismFixShip:error:not-at-planet", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismFixShip, seg), game::Exception);
}

// Planet has no base
AFL_TEST("game.interface.ShipMethod:ismFixShip:error:no-base", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismFixShip, seg), game::Exception);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismFixShip:error:not-played", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismFixShip, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismFixShip:error:arity", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismFixShip, seg), interpreter::Error);
}

/*
 *  ismRecycleShip
 */

// Normal case
AFL_TEST("game.interface.ShipMethod:ismRecycleShip:normal", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    call(env, sh, game::interface::ismRecycleShip, seg);
    a.checkEqual("getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::RecycleShipyardAction);
    a.checkEqual("getBaseShipyardId",     pl.getBaseShipyardId().orElse(-1), 77);
}

// Not at planet
AFL_TEST("game.interface.ShipMethod:ismRecycleShip:not-at-planet", a)
{
    Environment env;
    Ship sh(77);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
}

// Planet has no base
AFL_TEST("game.interface.ShipMethod:ismRecycleShip:no-base", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismRecycleShip:not-played", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismRecycleShip:error:arity", a)
{
    Environment env;
    Ship sh(77);
    Planet& pl = *env.turn->universe().planets().create(99);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismRecycleShip, seg), interpreter::Error);
}

/*
 *  ismSetWaypoint
 */

// Normal case
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:normal", a)
{
    Environment env;
    Ship sh(66);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    seg.pushBackInteger(Y + 30);
    call(env, sh, game::interface::ismSetWaypoint, seg);

    a.checkEqual("getWaypointDX", sh.getWaypointDX().orElse(-1), -20);
    a.checkEqual("getWaypointDY", sh.getWaypointDY().orElse(-1), +30);
}

// Null X - waypoint remains unchanged
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:null-x", a)
{
    Environment env;
    Ship sh(66);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(Y + 30);
    call(env, sh, game::interface::ismSetWaypoint, seg);

    a.checkEqual("getWaypointDX", sh.getWaypointDX().orElse(-1), 0);
    a.checkEqual("getWaypointDY", sh.getWaypointDY().orElse(-1), 0);
}

// Null Y - waypoint remains unchanged
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:null-y", a)
{
    Environment env;
    Ship sh(66);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetWaypoint, seg);

    a.checkEqual("getWaypointDX", sh.getWaypointDX().orElse(-1), 0);
    a.checkEqual("getWaypointDY", sh.getWaypointDY().orElse(-1), 0);
}

// Rejected because ship is fleet member
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:error:fleet", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(66);
    Ship& leader = *env.turn->universe().ships().create(77);
    configurePlayableShip(env, sh);
    configurePlayableShip(env, leader);
    leader.setFleetNumber(77);
    sh.setFleetNumber(77);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    seg.pushBackInteger(Y + 30);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetWaypoint, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:error:arity", a)
{
    Environment env;
    Ship sh(66);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:error:type", a)
{
    Environment env;
    Ship sh(66);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetWaypoint:error:not-played", a)
{
    Environment env;
    Ship sh(66);

    afl::data::Segment seg;
    seg.pushBackInteger(X - 20);
    seg.pushBackInteger(Y + 30);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
}

/*
 *  ismCargoTransfer
 */


// Normal case
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:normal", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    Ship& to   = *env.turn->universe().ships().create(22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    seg.pushBackInteger(22);
    call(env, from, game::interface::ismCargoTransfer, seg);

    a.checkEqual("from Tritanium", from.getCargo(game::Element::Tritanium).orElse(-1), 5);
    a.checkEqual("to Tritanium",   to.getCargo(game::Element::Tritanium).orElse(-1), 15);
}

// Null amount - command is ignored
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:null", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    Ship& to   = *env.turn->universe().ships().create(22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(22);
    call(env, from, game::interface::ismCargoTransfer, seg);

    a.checkEqual("from Tritanium", from.getCargo(game::Element::Tritanium).orElse(-1), 10);
    a.checkEqual("to Tritanium",   to.getCargo(game::Element::Tritanium).orElse(-1), 10);
}

// Overflow
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:error:overflow", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    Ship& to   = *env.turn->universe().ships().create(22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("55t");
    seg.pushBackInteger(22);
    AFL_CHECK_THROWS(a, call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
}

// Partial transfer
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:partial", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    Ship& to   = *env.turn->universe().ships().create(22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("55t");
    seg.pushBackInteger(22);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a, call(env, from, game::interface::ismCargoTransfer, seg));

    a.checkEqual("from Tritanium", from.getCargo(game::Element::Tritanium).orElse(-1), 0);
    a.checkEqual("to Tritanium", to.getCargo(game::Element::Tritanium).orElse(-1), 20);
    interpreter::test::verifyNewString(a("remainder"), env.proc.getVariable("CARGO.REMAINDER").release(), "45T");
}

// Missing target
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:error:no-target", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, from);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    seg.pushBackInteger(22);
    AFL_CHECK_THROWS(a, call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
}

// Syntax error
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:error:syntax", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    Ship& to   = *env.turn->universe().ships().create(22);
    configurePlayableShip(env, from);
    configurePlayableShip(env, to);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("xyzzy");
    seg.pushBackInteger(22);
    AFL_CHECK_THROWS(a, call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:error:arity", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, from);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    AFL_CHECK_THROWS(a, call(env, from, game::interface::ismCargoTransfer, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismCargoTransfer:error:type", a)
{
    Environment env;
    Ship& from = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, from);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, from, game::interface::ismCargoTransfer, seg), interpreter::Error);
}

/*
 *  ismCargoUnload
 */

// Normal case
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:normal", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    call(env, sh, game::interface::ismCargoUnload, seg);

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 5);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 55);
}

// Null amount - command is ignored
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:null", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismCargoUnload, seg);

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 10);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 50);
}

// Overflow
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:error:overflow", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("55t");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
}

// Partial transfer
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:partial", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("55t");
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a, call(env, sh, game::interface::ismCargoUnload, seg));

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 0);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 60);
    interpreter::test::verifyNewString(a("remainder"), env.proc.getVariable("CARGO.REMAINDER").release(), "45T");
}

// Missing target
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:error:no-target", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, sh);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
}

// Syntax error
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:error:syntax", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("xyzzy");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismCargoUnload:error:arity", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismCargoUnload, seg), interpreter::Error);
}

/*
 *  ismCargoUpload
 */


// Normal case
AFL_TEST("game.interface.ShipMethod:ismCargoUpload:normal", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("5t");
    call(env, sh, game::interface::ismCargoUpload, seg);

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 15);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 45);
}

// Partial transfer
AFL_TEST("game.interface.ShipMethod:ismCargoUpload:partial", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    seg.pushBackString("55t");
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a, call(env, sh, game::interface::ismCargoUpload, seg));

    a.checkEqual("ship Tritanium",   sh.getCargo(game::Element::Tritanium).orElse(-1), 60);
    a.checkEqual("planet Tritanium", pl.getCargo(game::Element::Tritanium).orElse(-1), 0);
    interpreter::test::verifyNewString(a("remainder"), env.proc.getVariable("CARGO.REMAINDER").release(), "5T");
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismCargoUpload:error:arity", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    Planet& pl = *env.turn->universe().planets().create(77);
    configurePlayableShip(env, sh);
    configurePlayablePlanet(env, pl);
    env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
    env.session.setShipList(env.shipList.asPtr());

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismCargoUpload, seg), interpreter::Error);
}

/*
 *  ismSetFleet
 */

// Make it a fleet leader
AFL_TEST("game.interface.ShipMethod:ismSetFleet:leader", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackInteger(55);
    call(env, sh, game::interface::ismSetFleet, seg);

    a.checkEqual("getFleetNumber", sh.getFleetNumber(), 55);
}

// Null
AFL_TEST("game.interface.ShipMethod:ismSetFleet:null", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, sh);
    sh.setFleetNumber(12);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, sh, game::interface::ismSetFleet, seg);

    a.checkEqual("getFleetNumber", sh.getFleetNumber(), 12);  // unchanged
}

// Type error
AFL_TEST("game.interface.ShipMethod:ismSetFleet:error:type", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.ShipMethod:ismSetFleet:error:arity", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);
    configurePlayableShip(env, sh);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.ShipMethod:ismSetFleet:error:not-played", a)
{
    Environment env;
    Ship& sh = *env.turn->universe().ships().create(55);

    afl::data::Segment seg;
    seg.pushBackInteger(55);
    AFL_CHECK_THROWS(a, call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
}
