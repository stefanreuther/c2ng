/**
  *  \file u/t_game_interface_shipmethod.cpp
  *  \brief Test for game::interface::ShipMethod
  */

#include "game/interface/shipmethod.hpp"

#include "t_game_interface.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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

/** Test ismMark, ismUnmark. */
void
TestGameInterfaceShipMethod::testMarkUnmark()
{
    Environment env;
    Ship sh(77);
    TS_ASSERT(!sh.isMarked());

    // Mark
    {
        afl::data::Segment seg;
        call(env, sh, game::interface::ismMark, seg);
        TS_ASSERT(sh.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        call(env, sh, game::interface::ismUnmark, seg);
        TS_ASSERT(!sh.isMarked());
    }

    // Mark True
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        call(env, sh, game::interface::ismMark, seg);
        TS_ASSERT(sh.isMarked());
    }

    // Mark False
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        call(env, sh, game::interface::ismMark, seg);
        TS_ASSERT(!sh.isMarked());
    }
}

/** Test ismSetComment. */
void
TestGameInterfaceShipMethod::testSetComment()
{
    Environment env;
    Ship sh(77);

    // Set comment
    {
        afl::data::Segment seg;
        seg.pushBackString("hi there");
        call(env, sh, game::interface::ismSetComment, seg);
        TS_ASSERT_EQUALS(interpreter::toString(env.session.world().shipProperties().get(77, interpreter::World::sp_Comment), false), "hi there");
    }

    // Null does not change the value
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetComment, seg);
        TS_ASSERT_EQUALS(interpreter::toString(env.session.world().shipProperties().get(77, interpreter::World::sp_Comment), false), "hi there");
    }

    // Arity error
    {
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetComment, seg), interpreter::Error);
    }
}

/** Tes ismSetFCode. */
void
TestGameInterfaceShipMethod::testSetFCode()
{
    // Set friendly code
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("abc");
        call(env, sh, game::interface::ismSetFCode, seg);
        TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "abc");
    }

    // Null does not change the value
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetFCode, seg);
        TS_ASSERT_EQUALS(sh.getFriendlyCode().orElse(""), "jkl");
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetFCode, seg), interpreter::Error);
    }

    // Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
    {
        Environment env;
        Ship sh(77);

        afl::data::Segment seg;
        seg.pushBackString("abc");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetFCode, seg), interpreter::Error);
    }
}

/** Test ismSetEnemy. */
void
TestGameInterfaceShipMethod::testSetEnemy()
{
    // Success case
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);
        env.root->playerList().create(3);       // Defines valid value

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        call(env, sh, game::interface::ismSetEnemy, seg);
        TS_ASSERT_EQUALS(sh.getPrimaryEnemy().orElse(-1), 3);
    }

    // Null
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetEnemy, seg);
        TS_ASSERT_EQUALS(sh.getPrimaryEnemy().orElse(-1), 1);   // unchanged
    }

    // Range error, specified value is not a valid race
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);
        env.root->playerList().create(3);       // Defines valid value

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);
        env.root->playerList().create(3);       // Defines valid value

        afl::data::Segment seg;
        seg.pushBackString("3");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);
        env.root->playerList().create(3);       // Defines valid value

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetEnemy, seg), interpreter::Error);
    }
}

/** Test ismSetSpeed. */
void
TestGameInterfaceShipMethod::testSetSpeed()
{
    // Success case
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        call(env, sh, game::interface::ismSetSpeed, seg);
        TS_ASSERT_EQUALS(sh.getWarpFactor().orElse(-1), 3);
    }

    // Null
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetSpeed, seg);
        TS_ASSERT_EQUALS(sh.getWarpFactor().orElse(-1), 9);
    }

    // Range error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(14);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("3");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetSpeed, seg), interpreter::Error);
    }
}

/** Test ismSetName. */
void
TestGameInterfaceShipMethod::testSetName()
{
    // Success case
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("Yacht");
        call(env, sh, game::interface::ismSetName, seg);
        TS_ASSERT_EQUALS(sh.getName(), "Yacht");
    }

    // Null
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetName, seg);
        TS_ASSERT_EQUALS(sh.getName(), "Boat");
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetName, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetName, seg), interpreter::Error);
    }

    // Target only - name can be changed!
    {
        Environment env;
        Ship sh(77);
        sh.addShipXYData(game::map::Point(1000, 1000), PLAYER+1, 100, game::PlayerSet_t(PLAYER));
        sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

        afl::data::Segment seg;
        seg.pushBackString("Spy");
        call(env, sh, game::interface::ismSetName, seg);
        TS_ASSERT_EQUALS(sh.getName(), "Spy");
    }
}

/** Test ismSetMission. */
void
TestGameInterfaceShipMethod::testSetMission()
{
    // Success case
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(40);
        seg.pushBackInteger(10);
        seg.pushBackInteger(900);
        call(env, sh, game::interface::ismSetMission, seg);
        TS_ASSERT_EQUALS(sh.getMission().orElse(-1), 40);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::InterceptParameter).orElse(-1), 10);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::TowParameter).orElse(-1), 900);
    }

    // Null mission - command will be ignored
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
        TS_ASSERT_EQUALS(sh.getMission().orElse(-1), 1);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::InterceptParameter).orElse(-1), 2);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::TowParameter).orElse(-1), 3);
    }

    // Null parameter - will be set to 0
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
        TS_ASSERT_EQUALS(sh.getMission().orElse(-1), 40);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::InterceptParameter).orElse(-1), 0);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::TowParameter).orElse(-1), 900);
    }

    // Only mission number given - parameters will be set to 0
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);
        sh.setMission(1, 2, 3);

        afl::data::Segment seg;
        seg.pushBackInteger(40);
        call(env, sh, game::interface::ismSetMission, seg);
        TS_ASSERT_EQUALS(sh.getMission().orElse(-1), 40);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::InterceptParameter).orElse(-1), 0);
        TS_ASSERT_EQUALS(sh.getMissionParameter(game::TowParameter).orElse(-1), 0);
    }

    // Rejected because ship is fleet member
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
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetMission, seg), game::Exception);
    }

    // Range error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(-40);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);

        afl::data::Segment seg;
        seg.pushBackInteger(40);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetMission, seg), interpreter::Error);
    }
}

/** Test ismFixShip. */
void
TestGameInterfaceShipMethod::testFixShip()
{
    // Normal case
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        call(env, sh, game::interface::ismFixShip, seg);
        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 77);
    }

    // Not at planet
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismFixShip, seg), game::Exception);
    }

    // Planet has no base
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismFixShip, seg), game::Exception);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismFixShip, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismFixShip, seg), interpreter::Error);
    }
}

/** Test ismRecycleShip. */
void
TestGameInterfaceShipMethod::testRecycleShip()
{
    // Normal case
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        call(env, sh, game::interface::ismRecycleShip, seg);
        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::RecycleShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 77);
    }

    // Not at planet
    {
        Environment env;
        Ship sh(77);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
    }

    // Planet has no base
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
    }

    // Not played
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismRecycleShip, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Ship sh(77);
        Planet& pl = *env.turn->universe().planets().create(99);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismRecycleShip, seg), interpreter::Error);
    }
}

/** Test ismSetWaypoint. */
void
TestGameInterfaceShipMethod::testSetWaypoint()
{
    // Normal case
    {
        Environment env;
        Ship sh(66);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(X - 20);
        seg.pushBackInteger(Y + 30);
        call(env, sh, game::interface::ismSetWaypoint, seg);

        TS_ASSERT_EQUALS(sh.getWaypointDX().orElse(-1), -20);
        TS_ASSERT_EQUALS(sh.getWaypointDY().orElse(-1), +30);
    }

    // Null X - waypoint remains unchanged
    {
        Environment env;
        Ship sh(66);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(Y + 30);
        call(env, sh, game::interface::ismSetWaypoint, seg);

        TS_ASSERT_EQUALS(sh.getWaypointDX().orElse(-1), 0);
        TS_ASSERT_EQUALS(sh.getWaypointDY().orElse(-1), 0);
    }

    // Null Y - waypoint remains unchanged
    {
        Environment env;
        Ship sh(66);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(X - 20);
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetWaypoint, seg);

        TS_ASSERT_EQUALS(sh.getWaypointDX().orElse(-1), 0);
        TS_ASSERT_EQUALS(sh.getWaypointDY().orElse(-1), 0);
    }

    // Rejected because ship is fleet member
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
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetWaypoint, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Ship sh(66);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(X - 20);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Ship sh(66);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(X - 20);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship sh(66);

        afl::data::Segment seg;
        seg.pushBackInteger(X - 20);
        seg.pushBackInteger(Y + 30);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetWaypoint, seg), interpreter::Error);
    }
}

/** Test ismCargoTransfer. */
void
TestGameInterfaceShipMethod::testCargoTransfer()
{
    // Normal case
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

        TS_ASSERT_EQUALS(from.getCargo(game::Element::Tritanium).orElse(-1), 5);
        TS_ASSERT_EQUALS(to.getCargo(game::Element::Tritanium).orElse(-1), 15);
    }

    // Null amount - command is ignored
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

        TS_ASSERT_EQUALS(from.getCargo(game::Element::Tritanium).orElse(-1), 10);
        TS_ASSERT_EQUALS(to.getCargo(game::Element::Tritanium).orElse(-1), 10);
    }

    // Overflow
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
        TS_ASSERT_THROWS(call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
    }

    // Partial transfer
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
        TS_ASSERT_THROWS_NOTHING(call(env, from, game::interface::ismCargoTransfer, seg));

        TS_ASSERT_EQUALS(from.getCargo(game::Element::Tritanium).orElse(-1), 0);
        TS_ASSERT_EQUALS(to.getCargo(game::Element::Tritanium).orElse(-1), 20);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "45T");
    }

    // Missing target
    {
        Environment env;
        Ship& from = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, from);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        seg.pushBackString("5t");
        seg.pushBackInteger(22);
        TS_ASSERT_THROWS(call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
    }

    // Syntax error
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
        TS_ASSERT_THROWS(call(env, from, game::interface::ismCargoTransfer, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Ship& from = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, from);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        seg.pushBackString("5t");
        TS_ASSERT_THROWS(call(env, from, game::interface::ismCargoTransfer, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Ship& from = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, from);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        seg.pushBackString("5t");
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, from, game::interface::ismCargoTransfer, seg), interpreter::Error);
    }
}

/** Test ismCargoUnload. */
void
TestGameInterfaceShipMethod::testCargoUnload()
{
    // Normal case
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

        TS_ASSERT_EQUALS(sh.getCargo(game::Element::Tritanium).orElse(-1), 5);
        TS_ASSERT_EQUALS(pl.getCargo(game::Element::Tritanium).orElse(-1), 55);
    }

    // Null amount - command is ignored
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

        TS_ASSERT_EQUALS(sh.getCargo(game::Element::Tritanium).orElse(-1), 10);
        TS_ASSERT_EQUALS(pl.getCargo(game::Element::Tritanium).orElse(-1), 50);
    }

    // Overflow
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
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
    }

    // Partial transfer
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
        TS_ASSERT_THROWS_NOTHING(call(env, sh, game::interface::ismCargoUnload, seg));

        TS_ASSERT_EQUALS(sh.getCargo(game::Element::Tritanium).orElse(-1), 0);
        TS_ASSERT_EQUALS(pl.getCargo(game::Element::Tritanium).orElse(-1), 60);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "45T");
    }

    // Missing target
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, sh);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        seg.pushBackString("5t");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
    }

    // Syntax error
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
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismCargoUnload, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        Planet& pl = *env.turn->universe().planets().create(77);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismCargoUnload, seg), interpreter::Error);
    }
}

/** Test ismCargoUpload. */
void
TestGameInterfaceShipMethod::testCargoUpload()
{
    // Normal case
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

        TS_ASSERT_EQUALS(sh.getCargo(game::Element::Tritanium).orElse(-1), 15);
        TS_ASSERT_EQUALS(pl.getCargo(game::Element::Tritanium).orElse(-1), 45);
    }

    // Partial transfer
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
        TS_ASSERT_THROWS_NOTHING(call(env, sh, game::interface::ismCargoUpload, seg));

        TS_ASSERT_EQUALS(sh.getCargo(game::Element::Tritanium).orElse(-1), 60);
        TS_ASSERT_EQUALS(pl.getCargo(game::Element::Tritanium).orElse(-1), 0);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "5T");
    }

    // Arity error
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        Planet& pl = *env.turn->universe().planets().create(77);
        configurePlayableShip(env, sh);
        configurePlayablePlanet(env, pl);
        env.session.setGame(env.g.asPtr());               // Cargo transfer requires game/shiplist connected!
        env.session.setShipList(env.shipList.asPtr());

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismCargoUpload, seg), interpreter::Error);
    }
}

/** Test ismSetFleet. */
void
TestGameInterfaceShipMethod::testSetFleet()
{
    // Make it a fleet leader
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackInteger(55);
        call(env, sh, game::interface::ismSetFleet, seg);

        TS_ASSERT_EQUALS(sh.getFleetNumber(), 55);
    }

    // Null
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, sh);
        sh.setFleetNumber(12);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, sh, game::interface::ismSetFleet, seg);

        TS_ASSERT_EQUALS(sh.getFleetNumber(), 12);  // unchanged
    }

    // Type error
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Ship& sh = *env.turn->universe().ships().create(55);

        afl::data::Segment seg;
        seg.pushBackInteger(55);
        TS_ASSERT_THROWS(call(env, sh, game::interface::ismSetFleet, seg), interpreter::Error);
    }
}

