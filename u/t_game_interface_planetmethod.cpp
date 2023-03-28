/**
  *  \file u/t_game_interface_planetmethod.cpp
  *  \brief Test for game::interface::PlanetMethod
  */

#include "game/interface/planetmethod.hpp"

#include "t_game_interface.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/reverter.hpp"
#include "game/map/ship.hpp"
#include "game/root.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using afl::base::Optional;
using afl::base::Ref;
using game::Element;
using game::ShipBuildOrder;
using game::map::Planet;
using game::map::Ship;

namespace {
    /*
     *  Reverter for testing - allows downgrading everything to 0 / selling 100 of everything
     */

    class Reverter : public game::map::Reverter {
     public:
        virtual Optional<int> getMinBuildings(int /*planetId*/, game::PlanetaryBuilding /*building*/) const
            { return 0; }
        virtual int getSuppliesAllowedToBuy(int /*planetId*/) const
            { return 100; }
        virtual Optional<int> getMinTechLevel(int /*planetId*/, game::TechLevel /*techLevel*/) const
            { return 1; }
        virtual Optional<int> getMinBaseStorage(int /*planetId*/, game::TechLevel /*area*/, int /*slot*/) const
            { return 0; }
        virtual int getNumTorpedoesAllowedToSell(int /*planetId*/, int /*slot*/) const
            { return 100; }
        virtual int getNumFightersAllowedToSell(int /*planetId*/) const
            { return 100; }
        virtual Optional<String_t> getPreviousShipFriendlyCode(game::Id_t /*shipId*/) const
            { return afl::base::Nothing; }
        virtual Optional<String_t> getPreviousPlanetFriendlyCode(game::Id_t /*planetId*/) const
            { return afl::base::Nothing; }
        virtual bool getPreviousShipMission(int /*shipId*/, int& /*m*/, int& /*i*/, int& /*t*/) const
            { return false; }
        virtual bool getPreviousShipBuildOrder(int /*planetId*/, game::ShipBuildOrder& /*result*/) const
            { return false; }
        virtual game::map::LocationReverter* createLocationReverter(game::map::Point /*pt*/) const
            { return 0; }
    };

    /*
     *  Test environment
     */

    const int TURN_NR = 10;
    const int PLAYER = 4;
    const int HULL_ID = 5;
    const int HULL_SLOT = 7;

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;
        interpreter::Process proc;
        Ref<game::Root> root;
        Ref<game::Turn> turn;
        game::map::Configuration mapConfig;
        Ref<game::spec::ShipList> shipList;

        Environment()
            : tx(), fs(), session(tx, fs),
              proc(session.world(), "tester", 777),
              root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0)))),
              turn(*new game::Turn()),
              shipList(*new game::spec::ShipList())
            {
                // Process: push a frame to be able to set CARGO.REMAINDER/BUILD.REMAINDER variables
                interpreter::Process::Frame& f = proc.pushFrame(interpreter::BytecodeObject::create(true), false);
                f.localNames.add("CARGO.REMAINDER");
                f.localNames.add("BUILD.REMAINDER");

                // Ship list: create a hull for a ship that can hold 200 cargo, 100 fuel.
                // Define additional parameters to make it buildable.
                game::spec::Hull& h = *shipList->hulls().create(HULL_ID);
                h.setMaxCargo(200);
                h.setMaxFuel(100);
                h.setMaxCrew(10);
                h.setNumEngines(2);
                h.setNumBays(0);
                h.setMaxLaunchers(10);
                h.setMaxBeams(8);
                h.setMass(20);
                h.setTechLevel(5);
                h.cost().set(game::spec::Cost::Tritanium, 5);
                h.cost().set(game::spec::Cost::Duranium, 7);
                h.cost().set(game::spec::Cost::Molybdenum, 9);
                h.cost().set(game::spec::Cost::Money, 100);

                // More properties
                shipList->hullAssignments().add(PLAYER, HULL_SLOT, HULL_ID);
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
                game::test::addNovaDrive(*shipList);
                game::test::addTranswarp(*shipList);

                // Session: connect ship list (no need to connect root, game; they're not supposed to be taken from session!)
                session.setShipList(shipList.asPtr());
            }
    };

    // Make planet playable with some default data
    void configurePlayablePlanet(Environment& env, Planet& pl)
    {
        // Planet
        game::map::PlanetData pd;
        pd.owner             = PLAYER;
        pd.friendlyCode      = "jkl";
        pd.numMines          = 20;
        pd.numFactories      = 30;
        pd.numDefensePosts   = 15;
        pd.minedNeutronium   = 120;
        pd.minedTritanium    = 84;
        pd.minedDuranium     = 76;
        pd.minedMolybdenum   = 230;
        pd.colonistClans     = 1200;
        pd.supplies          = 31;
        pd.money             = 15000;
        pd.groundNeutronium  = 1092;
        pd.groundTritanium   = 9102;
        pd.groundDuranium    = 349;
        pd.groundMolybdenum  = 781;
        pd.densityNeutronium = 14;
        pd.densityTritanium  = 87;
        pd.densityDuranium   = 29;
        pd.densityMolybdenum = 7;
        pd.colonistTax       = 3;
        pd.nativeTax         = 12;
        pd.colonistHappiness = 97;
        pd.nativeHappiness   = 76;
        pd.nativeGovernment  = 4;
        pd.nativeClans       = 7821;
        pd.nativeRace        = 3;
        pd.temperature       = 53;
        pd.baseFlag          = 1;

        pl.setPosition(game::map::Point(1030, 2700));
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
        for (int i = 1; i <= 10; ++i) {
            bd.engineStorage.set(i, 0);
            bd.hullStorage.set(i, 0);
            bd.beamStorage.set(i, 0);
            bd.launcherStorage.set(i, 0);
            bd.torpedoStorage.set(i, 0);
        }
        bd.numFighters    = 5;
        bd.shipyardId     = 0;
        bd.shipyardAction = 0;
        bd.mission        = 0;
        pl.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
        pl.internalCheck(env.mapConfig, game::PlayerSet_t(PLAYER), TURN_NR, env.tx, env.session.log());
    }

    // Make ship playable with default data
    void configurePlayableShip(Environment& /*env*/, Ship& sh)
    {
        game::map::ShipData sd;
        sd.x                 = 1030;
        sd.y                 = 2700;
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
        sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
        sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);
        sh.setPlayability(game::map::Object::Playable);
    }

    void call(Environment& env, Planet& pl, game::interface::PlanetMethod m, afl::data::Segment& seg)
    {
        interpreter::Arguments args(seg, 0, seg.size());
        callPlanetMethod(pl, m, args, env.proc, env.session, env.mapConfig, *env.turn, *env.root);
    }
}

/** Test parseBuildShipCommand(). */
void
TestGameInterfacePlanetMethod::testParseBuildShipCommand()
{
    game::spec::ShipList sl;
    game::test::addAnnihilation(sl);
    game::test::addGorbie(sl);
    game::test::initStandardBeams(sl);
    game::test::initStandardTorpedoes(sl);
    game::test::addTranswarp(sl);
    game::test::addNovaDrive(sl);

    // Null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(!result.isValid());
    }

    // Canceling a build
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), 0);
    }

    // Build a Gorbie, but do not specify anything (will fail because it has no engine)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::GORBIE_HULL_ID);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build a Gorbie, but do not specify weapons (will build without)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::GORBIE_HULL_ID);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 2);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::GORBIE_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 0);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 0);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 0);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 0);
    }

    // Wrong hull type
    {
        afl::data::Segment seg;
        seg.pushBackInteger(999);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Wrong engine type
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::GORBIE_HULL_ID);
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build a Gorbie, specifying weapon types, but no counts
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::GORBIE_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackNew(0);
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 6);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::GORBIE_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 7);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 10);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 0);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 0);
    }

    // Build an Annihilation, specifying weapon types, but no counts
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackNew(0);
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 6);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 7);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 10);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 4);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 10);
    }

    // Build an Annihilation, specifying weapon types and counts
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(3);
        seg.pushBackInteger(4);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 7);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 3);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 4);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 9);
    }

    // Build an Annihilation, beam type out of range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(77);
        seg.pushBackInteger(3);
        seg.pushBackInteger(4);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build an Annihilation, beam count out of range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(33);
        seg.pushBackInteger(4);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build an Annihilation, torpedo type out of range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(3);
        seg.pushBackInteger(44);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build an Annihilation, launcher type out of range
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(3);
        seg.pushBackInteger(4);
        seg.pushBackInteger(99);
        interpreter::Arguments args(seg, 0, 6);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Build an Annihilation, beam count given as 0 (will implicitly set torp count to 0)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(0);
        seg.pushBackInteger(4);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 0);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 0);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 4);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 9);
    }

    // Build an Annihilation, beam count given as -1 (will pick default, same as null)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
        seg.pushBackInteger(9);
        seg.pushBackInteger(7);
        seg.pushBackInteger(-1);
        seg.pushBackInteger(4);
        seg.pushBackInteger(9);
        interpreter::Arguments args(seg, 0, 6);
        Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
        TS_ASSERT(result.isValid());
        TS_ASSERT_EQUALS(result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
        TS_ASSERT_EQUALS(result.get()->getEngineType(), 9);
        TS_ASSERT_EQUALS(result.get()->getBeamType(), 7);
        TS_ASSERT_EQUALS(result.get()->getNumBeams(), 10);
        TS_ASSERT_EQUALS(result.get()->getTorpedoType(), 4);
        TS_ASSERT_EQUALS(result.get()->getNumLaunchers(), 9);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
    }
}

/** Test ipmMark, ipmUnmark. */
void
TestGameInterfacePlanetMethod::testMarkUnmark()
{
    Environment env;
    Planet pl(77);
    TS_ASSERT(!pl.isMarked());

    // Mark
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmMark, seg);
        TS_ASSERT(pl.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmUnmark, seg);
        TS_ASSERT(!pl.isMarked());
    }

    // Mark True
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        call(env, pl, game::interface::ipmMark, seg);
        TS_ASSERT(pl.isMarked());
    }

    // Mark False
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        call(env, pl, game::interface::ipmMark, seg);
        TS_ASSERT(!pl.isMarked());
    }
}

/** Test ipmSetComment. */
void
TestGameInterfacePlanetMethod::testSetComment()
{
    Environment env;
    Planet pl(77);

    // Set comment
    {
        afl::data::Segment seg;
        seg.pushBackString("hi there");
        call(env, pl, game::interface::ipmSetComment, seg);
        TS_ASSERT_EQUALS(interpreter::toString(env.session.world().planetProperties().get(77, interpreter::World::pp_Comment), false), "hi there");
    }

    // Null does not change the value
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetComment, seg);
        TS_ASSERT_EQUALS(interpreter::toString(env.session.world().planetProperties().get(77, interpreter::World::pp_Comment), false), "hi there");
    }

    // Arity error
    {
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetComment, seg), interpreter::Error);
    }
}

/** Test ipmFixShip. */
void
TestGameInterfacePlanetMethod::testFixShip()
{
    // Normal case
    {
        Environment env;
        Planet pl(99);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(66);
        call(env, pl, game::interface::ipmFixShip, seg);

        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 66);

        // Cancel
        afl::data::Segment seg2;
        seg2.pushBackInteger(0);
        call(env, pl, game::interface::ipmFixShip, seg2);

        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 0);
    }
    // More related testcases below for ipmRecycleShip.
}

/** Test ipmRecycleShip. */
void
TestGameInterfacePlanetMethod::testRecycleShip()
{
    // Normal case
    {
        Environment env;
        Planet pl(99);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(66);
        call(env, pl, game::interface::ipmRecycleShip, seg);

        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::RecycleShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 66);

        // Cancel
        afl::data::Segment seg2;
        seg2.pushBackInteger(0);
        call(env, pl, game::interface::ipmRecycleShip, seg2);

        TS_ASSERT_EQUALS(pl.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
        TS_ASSERT_EQUALS(pl.getBaseShipyardId().orElse(-1), 0);
    }

    // Bad ship Id
    {
        Environment env;
        Planet pl(99);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
    }

    // Bad ship position
    {
        Environment env;
        Planet pl(99);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);
        sh.setPosition(game::map::Point(3333, 3333));

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
    }

    // Type error
    {
        Environment env;
        Planet pl(99);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(99);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        // Recycle
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), interpreter::Error);
    }

    // No base
    {
        Environment env;
        Planet pl(99);
        configurePlayablePlanet(env, pl);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
    }

    // Not played
    {
        Environment env;
        Planet pl(99);

        // Recycle
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
    }
}

/** Test ipmBuildBase. */
void
TestGameInterfacePlanetMethod::testBuildBase()
{
    // Normal case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setBuildBaseFlag(false);
        pl.setCargo(Element::Tritanium, 500);
        pl.setCargo(Element::Duranium, 500);
        pl.setCargo(Element::Molybdenum, 500);

        // Build it
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmBuildBase, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14100);
        TS_ASSERT_EQUALS(pl.isBuildingBase(), true);

        // Cancel it
        afl::data::Segment seg1;
        seg1.pushBackInteger(0);
        call(env, pl, game::interface::ipmBuildBase, seg1);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 15000);
        TS_ASSERT_EQUALS(pl.isBuildingBase(), false);

        // Build again
        afl::data::Segment seg2;
        seg2.pushBackInteger(1);
        call(env, pl, game::interface::ipmBuildBase, seg2);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14100);
        TS_ASSERT_EQUALS(pl.isBuildingBase(), true);
    }

    // Failure: no resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setBuildBaseFlag(false);
        pl.setCargo(Element::Tritanium, 50);
        pl.setCargo(Element::Duranium, 50);
        pl.setCargo(Element::Molybdenum, 50);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
    }

    // Failure: already building
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setBuildBaseFlag(true);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
    }

    // Failure: base already present
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBuildBaseFlag(false);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setBuildBaseFlag(false);
        pl.setCargo(Element::Tritanium, 500);
        pl.setCargo(Element::Duranium, 500);
        pl.setCargo(Element::Molybdenum, 500);

        // Build it
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(2);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBase, seg), interpreter::Error);
    }
}

/** Test ipmAutoBuild. */
void
TestGameInterfacePlanetMethod::testAutoBuild()
{
    // Normal case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        call(env, pl, game::interface::ipmAutoBuild, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::MineBuilding).orElse(-1), 28);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 18);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 0);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 0);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14878);
    }

    // With starbase
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        call(env, pl, game::interface::ipmAutoBuild, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::MineBuilding).orElse(-1), 28);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 18);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 20);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 0);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14778);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmAutoBuild, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmAutoBuild, seg), interpreter::Error);
    }
}

/** Test ipmBuildDefense. */
void
TestGameInterfacePlanetMethod::testBuildDefense()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmBuildDefense, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 35);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14800);
    }

    // Limit exceeded
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        seg.pushBackString("n");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildDefense, seg));

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 56);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14590);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 159);
    }

    // Try to scrap with no reverter
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(-20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
    }

    // Try to scrap with reverter, exceeding limit
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        env.turn->universe().setNewReverter(new Reverter());

        afl::data::Segment seg;
        seg.pushBackInteger(-20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
    }

    // Try to scrap with reverter, exceeding limit, partial scrap allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        env.turn->universe().setNewReverter(new Reverter());

        afl::data::Segment seg;
        seg.pushBackInteger(-20);
        seg.pushBackString("N");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildDefense, seg));
        TS_ASSERT_EQUALS(pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 0);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), -5);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
    }
}

/** Test ipmBuildFactories. */
void
TestGameInterfacePlanetMethod::testBuildFactories()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmBuildFactories, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14940);
    }

    // Limit exceeded
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFactories, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        seg.pushBackString("n");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildFactories, seg));

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 90);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14820);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 140);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFactories, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFactories, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFactories, seg), game::Exception);
    }
}

/** Test ipmBuildMines. */
void
TestGameInterfacePlanetMethod::testBuildMines()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmBuildMines, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::MineBuilding).orElse(-1), 40);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14920);
    }

    // Limit exceeded
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildMines, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Colonists, 90);
        pl.setCargo(Element::Supplies, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        seg.pushBackString("n");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildMines, seg));

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::MineBuilding).orElse(-1), 90);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14720);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 130);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildMines, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildMines, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildMines, seg), game::Exception);
    }
}

/** Test ipmSetColonistTax. */
void
TestGameInterfacePlanetMethod::testSetColonistTax()
{
    // Normal case
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmSetColonistTax, seg);
        TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 20);
    }

    // Null does not change the value
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetColonistTax, seg);
        TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 3);
    }

    // Arity error
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
    }

    // Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
    {
        Environment env;
        Planet pl(77);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(77);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
    }

    // Range error
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(101);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
    }
}

/** Test ipmSetNativeTax. */
void
TestGameInterfacePlanetMethod::testSetNativeTax()
{
    // Normal case
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmSetNativeTax, seg);
        TS_ASSERT_EQUALS(pl.getNativeTax().orElse(-1), 20);
    }

    // Null does not change the value
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetNativeTax, seg);
        TS_ASSERT_EQUALS(pl.getNativeTax().orElse(-1), 12);
    }

    // Arity error
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
    }

    // Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
    {
        Environment env;
        Planet pl(77);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(77);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
    }

    // Range error
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(101);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
    }

    // No natives
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);
        pl.setNativeRace(0);
        pl.setNatives(0);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
    }
}

/** Test ipmSetFCode. */
void
TestGameInterfacePlanetMethod::testSetFCode()
{
    // Set friendly code
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("abc");
        call(env, pl, game::interface::ipmSetFCode, seg);
        TS_ASSERT_EQUALS(pl.getFriendlyCode().orElse(""), "abc");
    }

    // Null does not change the value
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetFCode, seg);
        TS_ASSERT_EQUALS(pl.getFriendlyCode().orElse(""), "jkl");
    }

    // Arity error
    {
        Environment env;
        Planet pl(77);
        configurePlayablePlanet(env, pl);
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetFCode, seg), interpreter::Error);
    }

    // Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
    {
        Environment env;
        Planet pl(77);

        afl::data::Segment seg;
        seg.pushBackString("abc");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetFCode, seg), interpreter::Error);
    }
}

/** Test ipmSetMission. */
void
TestGameInterfacePlanetMethod::testSetMission()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseMission(1);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmSetMission, seg);

        TS_ASSERT_EQUALS(pl.getBaseMission().orElse(-1), 5);
    }

    // Null
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseMission(1);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetMission, seg);

        TS_ASSERT_EQUALS(pl.getBaseMission().orElse(-1), 1);
    }

    // Range error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseMission(1);

        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
        TS_ASSERT_EQUALS(pl.getBaseMission().orElse(-1), 1);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
    }
}

/** Test ipmBuildBaseDefense. */
void
TestGameInterfacePlanetMethod::testBuildBaseDefense()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        call(env, pl, game::interface::ipmBuildBaseDefense, seg);

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 30);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14800);
    }

    // Limit exceeded
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 10000);

        afl::data::Segment seg;
        seg.pushBackInteger(200);
        seg.pushBackString("n");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildBaseDefense, seg));

        TS_ASSERT_EQUALS(pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 200);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 13100);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 10);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBaseDefense, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBaseDefense, seg), interpreter::Error);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
    }

    // No base, but accepting partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        seg.pushBackString("n");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildBaseDefense, seg));
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 20);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(20);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
    }
}

/** Test ipmSetTech. */
void
TestGameInterfacePlanetMethod::testSetTech()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmSetTech, seg);

        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 5);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 14000);
    }

    // Null index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmSetTech, seg);

        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 15000);
    }

    // Null level
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetTech, seg);

        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 15000);
    }

    // Index range error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
    }

    // Level range error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(15);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
    }

    // Level not permitted by key
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(9);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
    }
}

/** Test ipmBuildFighters. */
void
TestGameInterfacePlanetMethod::testBuildFighters()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildFighters, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Fighters).orElse(-1), 10);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Money, 350);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildFighters, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Fighters).orElse(-1), 8);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 2);
    }

    // Ship target
    {
        Environment env;
        Planet pl(111);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);
        sh.setNumBays(1);
        sh.setAmmo(0);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        call(env, pl, game::interface::ipmBuildFighters, seg);

        TS_ASSERT_EQUALS(sh.getCargo(Element::Fighters).orElse(-1), 5);
    }

    // Failure, bad ship target
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
    }

    // Failure, ship target has no fighters
    {
        Environment env;
        Planet pl(111);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);
        sh.setNumBays(0);
        sh.setTorpedoType(0);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
    }
}

/** Test ipmBuildEngines. */
void
TestGameInterfacePlanetMethod::testBuildEngines()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);      // Nova drive
        seg.pushBackInteger(3);
        call(env, pl, game::interface::ipmBuildEngines, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
    }

    // Null amount
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmBuildEngines, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::EngineTech, 4).orElse(-1), 0);
    }

    // Null type
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildEngines, seg);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 10);

        afl::data::Segment seg;
        seg.pushBackInteger(5);     // Nova drive costs 3 duranium
        seg.pushBackInteger(7);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 10);

        afl::data::Segment seg;
        seg.pushBackInteger(5);     // Nova drive costs 3 duranium
        seg.pushBackInteger(7);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildEngines, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Duranium).orElse(-1), 1);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 4);
    }

    // Try to scrap with no reverter
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseStorage(game::EngineTech, 5, 10);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(-7);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }

    // Try to scrap with reverter, not exceeding limit
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseStorage(game::EngineTech, 5, 10);
        env.turn->universe().setNewReverter(new Reverter());

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(-7);
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildEngines, seg));
        TS_ASSERT_EQUALS(pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
    }

    // Try to scrap with reverter, exceeding limit
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseStorage(game::EngineTech, 5, 10);
        env.turn->universe().setNewReverter(new Reverter());

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(-15);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }

    // Try to scrap with reverter, exceeding limit, partial scrap allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setBaseStorage(game::EngineTech, 5, 10);
        env.turn->universe().setNewReverter(new Reverter());

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(-15);
        seg.pushBackString("N");
        TS_ASSERT_THROWS_NOTHING(call(env, pl, game::interface::ipmBuildEngines, seg));
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), -5);
    }

    // Failure, tech not allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(9);
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }

    // Failure, bad index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(11);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
    }
}

/** Test ipmBuildHulls. */
void
TestGameInterfacePlanetMethod::testBuildHulls()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildHulls, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 5);
    }

    // Null amount
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmBuildHulls, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
    }

    // Null type
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildHulls, seg);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 20);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);     // costs 7 Duranium
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 20);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);     // costs 7 Duranium
        seg.pushBackInteger(5);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildHulls, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 2);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Duranium).orElse(-1), 6);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 3);
    }

    // Failure, tech not allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        env.shipList->hulls().get(HULL_ID)->setTechLevel(10);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
    }

    // Failure, bad index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(111);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
    }

    // Failure, valid index but not buildable
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        env.shipList->hulls().create(HULL_ID+1);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID+1);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
    }
}

/** Test ipmBuildLaunchers. */
void
TestGameInterfacePlanetMethod::testBuildLaunchers()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildLaunchers, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 5);
    }

    // Null amount
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmBuildLaunchers, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 0);
    }

    // Null type
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildLaunchers, seg);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 15);

        afl::data::Segment seg;
        seg.pushBackInteger(3);     // Mark 2 Photon costs 4 Duranium
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 15);

        afl::data::Segment seg;
        seg.pushBackInteger(3);     // Mark 2 Photon costs 4 Duranium
        seg.pushBackInteger(5);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildLaunchers, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::TorpedoTech, 3).orElse(-1), 3);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Duranium).orElse(-1), 3);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 2);
    }

    // Failure, tech not allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(10);
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
    }

    // Failure, bad index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(11);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
    }
}

/** Test ipmBuildBeams. */
void
TestGameInterfacePlanetMethod::testBuildBeams()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildBeams, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::BeamTech, 4).orElse(-1), 5);
    }

    // Null amount
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmBuildBeams, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::BeamTech, 4).orElse(-1), 0);
    }

    // Null type
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildBeams, seg);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 5);

        afl::data::Segment seg;
        seg.pushBackInteger(3);     // Plasma Bolt costs 2 Duranium
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 5);

        afl::data::Segment seg;
        seg.pushBackInteger(3);     // Plasma Bolt costs 2 Duranium
        seg.pushBackInteger(5);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildBeams, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::BeamTech, 3).orElse(-1), 2);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Duranium).orElse(-1), 1);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 3);
    }

    // Failure, tech not allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(10);
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
    }

    // Failure, bad index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(11);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
    }
}

/** Test ipmBuildTorps. */
void
TestGameInterfacePlanetMethod::testBuildTorps()
{
    // Success case
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildTorps, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 5);
    }

    // Null amount
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmBuildTorps, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 0);
    }

    // Null type
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(5);
        call(env, pl, game::interface::ipmBuildTorps, seg);
    }

    // Failure, not enough resources
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 2);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }

    // Partial build
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        pl.setCargo(Element::Duranium, 2);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmBuildTorps, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 2);
        interpreter::test::verifyNewInteger("remainder", env.proc.getVariable("BUILD.REMAINDER").release(), 3);
    }

    // Ship target
    {
        Environment env;
        Planet pl(111);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);
        sh.setNumLaunchers(1);
        sh.setTorpedoType(4);
        sh.setAmmo(0);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        call(env, pl, game::interface::ipmBuildTorps, seg);

        TS_ASSERT_EQUALS(sh.getCargo(Element::fromTorpedoType(4)).orElse(-1), 5);
    }

    // Failure, tech not allowed
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(10);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }

    // Failure, bad index
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(11);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
    }

    // Failure, bad ship target
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }

    // Failure, ship target has no torps
    {
        Environment env;
        Planet pl(111);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);
        configurePlayableShip(env, sh);
        sh.setNumLaunchers(0);
        sh.setTorpedoType(0);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        seg.pushBackInteger(66);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }

    // Arity error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
    }

    // Not played
    {
        Environment env;
        Planet pl(111);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }

    // No base
    {
        Environment env;
        Planet pl(111);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
    }
}

/** Test ipmSellSupplies. */
void
TestGameInterfacePlanetMethod::testSellSupplies()
{
    // Success case
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        seg.pushBackInteger(30);
        call(env, pl, game::interface::ipmSellSupplies, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 70);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 80);
    }

    // Overflow case
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        seg.pushBackInteger(130);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSellSupplies, seg), game::Exception);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 100);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 50);
    }

    // Partial
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        seg.pushBackInteger(130);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmSellSupplies, seg);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Supplies).orElse(-1), 0);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 150);
    }

    // Arity error
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSellSupplies, seg), interpreter::Error);
    }

    // Type error
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        pl.setCargo(Element::Supplies, 100);
        pl.setCargo(Element::Money, 50);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmSellSupplies, seg), interpreter::Error);
    }
}

/** Test ipmBuildShip. */
void
TestGameInterfacePlanetMethod::testBuildShip()
{
    // Success case
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(5);   // Nova drive
        seg.pushBackInteger(2);
        seg.pushBackInteger(3);   // 3 beams
        seg.pushBackInteger(4);
        seg.pushBackInteger(7);   // 7 launchers
        call(env, pl, game::interface::ipmBuildShip, seg);

        TS_ASSERT_EQUALS(pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
        TS_ASSERT_EQUALS(pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 2);
        TS_ASSERT_EQUALS(pl.getBaseStorage(game::BeamTech, 2).orElse(-1), 3);
        TS_ASSERT_EQUALS(pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 7);

        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::HullTech).orElse(-1), 5);
        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::EngineTech).orElse(-1), 5);
        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
        TS_ASSERT_EQUALS(pl.getBaseTechLevel(game::TorpedoTech).orElse(-1), 3);

        TS_ASSERT_EQUALS(pl.getBaseBuildOrder().getHullIndex(), HULL_SLOT);

        TS_ASSERT_EQUALS(pl.getCargo(Element::Money).orElse(-1), 12502);

        // We can also cancel
        afl::data::Segment seg2;
        seg2.pushBackInteger(0);
        call(env, pl, game::interface::ipmBuildShip, seg2);

        TS_ASSERT_EQUALS(pl.getBaseBuildOrder().getHullIndex(), 0);
    }

    // Failure case: no base
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(5);   // Nova drive
        seg.pushBackInteger(2);
        seg.pushBackInteger(3);   // 3 beams
        seg.pushBackInteger(4);
        seg.pushBackInteger(7);   // 7 launchers
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildShip, seg), game::Exception);
    }

    // Failure case: no tech
    {
        Environment env;
        Planet pl(55);
        configurePlayablePlanet(env, pl);
        configurePlayableBase(env, pl);

        afl::data::Segment seg;
        seg.pushBackInteger(HULL_ID);
        seg.pushBackInteger(9);   // Transwarp - not possible, test key only allows tech 6
        seg.pushBackInteger(2);
        seg.pushBackInteger(3);   // 3 beams
        seg.pushBackInteger(4);
        seg.pushBackInteger(7);   // 7 launchers
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmBuildShip, seg), game::Exception);
    }
}

/** Test ipmCargoTransfer. */
void
TestGameInterfacePlanetMethod::testCargoTransfer()
{
    // Normal case
    {
        Environment env;
        Planet& pl = *env.turn->universe().planets().create(44);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("n20");
        seg.pushBackInteger(66);
        call(env, pl, game::interface::ipmCargoTransfer, seg);

        TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(-1), 30);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Neutronium).orElse(-1), 100);
    }

    // Partial case
    {
        Environment env;
        Planet& pl = *env.turn->universe().planets().create(44);
        Ship& sh = *env.turn->universe().ships().create(66);
        configurePlayablePlanet(env, pl);
        configurePlayableShip(env, sh);

        afl::data::Segment seg;
        seg.pushBackString("n200");
        seg.pushBackInteger(66);
        seg.pushBackString("n");
        call(env, pl, game::interface::ipmCargoTransfer, seg);

        TS_ASSERT_EQUALS(sh.getCargo(Element::Neutronium).orElse(-1), 100);
        TS_ASSERT_EQUALS(pl.getCargo(Element::Neutronium).orElse(-1), 30);
        interpreter::test::verifyNewString("remainder", env.proc.getVariable("CARGO.REMAINDER").release(), "110N");
    }

    // Error case, bad Id
    {
        Environment env;
        Planet& pl = *env.turn->universe().planets().create(44);
        configurePlayablePlanet(env, pl);

        afl::data::Segment seg;
        seg.pushBackString("n200");
        seg.pushBackInteger(77);
        seg.pushBackString("n");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmCargoTransfer, seg), game::Exception);
    }
}

/** Test ipmAutoTaxColonists. */
void
TestGameInterfacePlanetMethod::testAutoTaxColonists()
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    // Normal case
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmAutoTaxColonists, seg);
        TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 10);
    }

    // Arity error
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmAutoTaxColonists, seg), interpreter::Error);
    }
}

/** Test ipmAutoTaxNatives. */
void
TestGameInterfacePlanetMethod::testAutoTaxNatives()
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    // Normal case
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmAutoTaxNatives, seg);
        TS_ASSERT_EQUALS(pl.getNativeTax().orElse(-1), 2);
    }

    // Arity error
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmAutoTaxNatives, seg), interpreter::Error);
    }
}

/** Test ipmApplyBuildGoals. */
void
TestGameInterfacePlanetMethod::testApplyBuildGoals()
{
    Environment env;
    Planet pl(77);

    // Standard case: modify everything
    {
        Planet::AutobuildSettings abs;
        abs.goal[game::MineBuilding] = 100;
        abs.goal[game::FactoryBuilding] = 200;
        abs.goal[game::DefenseBuilding] = 300;
        abs.goal[game::BaseDefenseBuilding] = 400;
        abs.speed[game::MineBuilding] = 11;
        abs.speed[game::FactoryBuilding] = 22;
        abs.speed[game::DefenseBuilding] = 33;
        abs.speed[game::BaseDefenseBuilding] = 44;

        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::AutobuildSettingsValue_t(abs));
        call(env, pl, game::interface::ipmApplyBuildGoals, seg);

        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::MineBuilding), 100);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::FactoryBuilding), 200);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::DefenseBuilding), 300);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::BaseDefenseBuilding), 400);

        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::MineBuilding), 11);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::FactoryBuilding), 22);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::DefenseBuilding), 33);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::BaseDefenseBuilding), 44);
    }

    // Modify parts
    {
        Planet::AutobuildSettings abs;
        abs.goal[game::MineBuilding] = 88;
        abs.speed[game::DefenseBuilding] = 55;

        afl::data::Segment seg;
        seg.pushBackNew(new game::interface::AutobuildSettingsValue_t(abs));
        call(env, pl, game::interface::ipmApplyBuildGoals, seg);

        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::MineBuilding), 88);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::FactoryBuilding), 200);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::DefenseBuilding), 300);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::BaseDefenseBuilding), 400);

        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::MineBuilding), 11);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::FactoryBuilding), 22);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::DefenseBuilding), 55);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::BaseDefenseBuilding), 44);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmApplyBuildGoals, seg), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        TS_ASSERT_THROWS(call(env, pl, game::interface::ipmApplyBuildGoals, seg), interpreter::Error);
    }
}

