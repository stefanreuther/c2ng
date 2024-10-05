/**
  *  \file test/game/interface/planetmethodtest.cpp
  *  \brief Test for game::interface::PlanetMethod
  */

#include "game/interface/planetmethod.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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

/*
 *  parseBuildShipCommand
 */

namespace {
    void prepareBuildShipCommand(game::spec::ShipList& sl)
    {
        game::test::addAnnihilation(sl);
        game::test::addGorbie(sl);
        game::test::initStandardBeams(sl);
        game::test::initStandardTorpedoes(sl);
        game::test::addTranswarp(sl);
        game::test::addNovaDrive(sl);
    }
}

// Null
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:null", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", !result.isValid());
}

// Canceling a build
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:cancel", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 1);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex", result.get()->getHullIndex(), 0);
}

// Build a Gorbie, but do not specify anything (will fail because it has no engine)
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:ship-without-engine", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::GORBIE_HULL_ID);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build a Gorbie, but do not specify weapons (will build without)
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:no-weapons", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::GORBIE_HULL_ID);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 2);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::GORBIE_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 0);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 0);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 0);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 0);
}

// Wrong hull type
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-hull-type", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(999);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Wrong engine type
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-engine-type", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::GORBIE_HULL_ID);
    seg.pushBackInteger(99);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build a Gorbie, specifying weapon types, but no counts
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:carrier-with-default-weapons", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::GORBIE_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackNew(0);
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 6);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::GORBIE_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 7);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 10);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 0);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 0);
}

// Build an Annihilation, specifying weapon types, but no counts
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:torper-with-default-weapons", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackNew(0);
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 6);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 7);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 10);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 4);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 10);
}

// Build an Annihilation, specifying weapon types and counts
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:torper-with-specified-weapons", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(3);
    seg.pushBackInteger(4);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 7);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 3);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 4);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 9);
}

// Build an Annihilation, beam type out of range
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-beam-type", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(77);
    seg.pushBackInteger(3);
    seg.pushBackInteger(4);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build an Annihilation, beam count out of range
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-beam-count", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(33);
    seg.pushBackInteger(4);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build an Annihilation, torpedo type out of range
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-torp-type", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(3);
    seg.pushBackInteger(44);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build an Annihilation, launcher count out of range
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:bad-launcher-count", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(3);
    seg.pushBackInteger(4);
    seg.pushBackInteger(99);
    interpreter::Arguments args(seg, 0, 6);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Build an Annihilation, beam count given as 0 (will implicitly set beam count to 0)
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:zero-beams", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(0);
    seg.pushBackInteger(4);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 0);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 0);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 4);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 9);
}

// Build an Annihilation, beam count given as -1 (will pick default, same as null)
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:success:default-beam-count", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackInteger(game::test::ANNIHILATION_HULL_ID);
    seg.pushBackInteger(9);
    seg.pushBackInteger(7);
    seg.pushBackInteger(-1);
    seg.pushBackInteger(4);
    seg.pushBackInteger(9);
    interpreter::Arguments args(seg, 0, 6);
    Optional<ShipBuildOrder> result = game::interface::parseBuildShipCommand(args, sl);
    a.check("isValid", result.isValid());
    a.checkEqual("getHullIndex",    result.get()->getHullIndex(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("getEngineType",   result.get()->getEngineType(), 9);
    a.checkEqual("getBeamType",     result.get()->getBeamType(), 7);
    a.checkEqual("getNumBeams",     result.get()->getNumBeams(), 10);
    a.checkEqual("getTorpedoType",  result.get()->getTorpedoType(), 4);
    a.checkEqual("getNumLaunchers", result.get()->getNumLaunchers(), 9);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:arity", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:parseBuildShipCommand:error:type", a)
{
    game::spec::ShipList sl;
    prepareBuildShipCommand(sl);
    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::parseBuildShipCommand(args, sl), interpreter::Error);
}

/*
 *  ipmMark, ipmUnmark
 */

AFL_TEST("game.interface.PlanetMethod:ipmMark", a)
{
    Environment env;
    Planet pl(77);
    a.check("01", !pl.isMarked());

    // Mark
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmMark, seg);
        a.check("11", pl.isMarked());
    }

    // Unmark
    {
        afl::data::Segment seg;
        call(env, pl, game::interface::ipmUnmark, seg);
        a.check("21", !pl.isMarked());
    }

    // Mark True
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        call(env, pl, game::interface::ipmMark, seg);
        a.check("31", pl.isMarked());
    }

    // Mark False
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        call(env, pl, game::interface::ipmMark, seg);
        a.check("41", !pl.isMarked());
    }
}

/*
 *  ipmSetComment
 */

AFL_TEST("game.interface.PlanetMethod:ipmSetComment", a)
{
    Environment env;
    Planet pl(77);

    // Set comment
    {
        afl::data::Segment seg;
        seg.pushBackString("hi there");
        call(env, pl, game::interface::ipmSetComment, seg);
        a.checkEqual("01", interpreter::toString(env.session.world().planetProperties().get(77, interpreter::World::pp_Comment), false), "hi there");
    }

    // Null does not change the value
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        call(env, pl, game::interface::ipmSetComment, seg);
        a.checkEqual("11", interpreter::toString(env.session.world().planetProperties().get(77, interpreter::World::pp_Comment), false), "hi there");
    }

    // Arity error
    {
        afl::data::Segment seg;
        AFL_CHECK_THROWS(a("21. arity error"), call(env, pl, game::interface::ipmSetComment, seg), interpreter::Error);
    }
}

/*
 *  ipmFixShip, ipmRecycleShip
 */

// More related testcases below for ipmRecycleShip.
AFL_TEST("game.interface.PlanetMethod:ipmFixShip", a)
{
    // Normal case
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

    a.checkEqual("01. getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    a.checkEqual("02. getBaseShipyardId", pl.getBaseShipyardId().orElse(-1), 66);

    // Cancel
    afl::data::Segment seg2;
    seg2.pushBackInteger(0);
    call(env, pl, game::interface::ipmFixShip, seg2);

    a.checkEqual("11. getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    a.checkEqual("12. getBaseShipyardId", pl.getBaseShipyardId().orElse(-1), 0);
}


// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:normal", a)
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

    a.checkEqual("01. getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::RecycleShipyardAction);
    a.checkEqual("02. getBaseShipyardId", pl.getBaseShipyardId().orElse(-1), 66);

    // Cancel
    afl::data::Segment seg2;
    seg2.pushBackInteger(0);
    call(env, pl, game::interface::ipmRecycleShip, seg2);

    a.checkEqual("11. getBaseShipyardAction", pl.getBaseShipyardAction().orElse(-1), game::NoShipyardAction);
    a.checkEqual("12. getBaseShipyardId", pl.getBaseShipyardId().orElse(-1), 0);
}

// Bad ship Id
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:bad-id", a)
{
    Environment env;
    Planet pl(99);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    // Recycle
    afl::data::Segment seg;
    seg.pushBackInteger(66);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
}

// Bad ship position
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:position-mismatch", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:type", a)
{
    Environment env;
    Planet pl(99);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    // Recycle
    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:arity", a)
{
    Environment env;
    Planet pl(99);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    // Recycle
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), interpreter::Error);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:no-base", a)
{
    Environment env;
    Planet pl(99);
    configurePlayablePlanet(env, pl);

    // Recycle
    afl::data::Segment seg;
    seg.pushBackInteger(0);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmRecycleShip:error:not-played", a)
{
    Environment env;
    Planet pl(99);

    // Recycle
    afl::data::Segment seg;
    seg.pushBackInteger(0);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmRecycleShip, seg), game::Exception);
}

/*
 *  ipmBuildBase
 */


// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmBuildBase:normal", a)
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

    a.checkEqual("01. money", pl.getCargo(Element::Money).orElse(-1), 14100);
    a.checkEqual("02. isBuildingBase", pl.isBuildingBase(), true);

    // Cancel it
    afl::data::Segment seg1;
    seg1.pushBackInteger(0);
    call(env, pl, game::interface::ipmBuildBase, seg1);

    a.checkEqual("11. money", pl.getCargo(Element::Money).orElse(-1), 15000);
    a.checkEqual("12. isBuildingBase", pl.isBuildingBase(), false);

    // Build again
    afl::data::Segment seg2;
    seg2.pushBackInteger(1);
    call(env, pl, game::interface::ipmBuildBase, seg2);

    a.checkEqual("21. money", pl.getCargo(Element::Money).orElse(-1), 14100);
    a.checkEqual("22. isBuildingBase", pl.isBuildingBase(), true);
}

// Failure: no resources
AFL_TEST("game.interface.PlanetMethod:ipmBuildBase:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setBuildBaseFlag(false);
    pl.setCargo(Element::Tritanium, 50);
    pl.setCargo(Element::Duranium, 50);
    pl.setCargo(Element::Molybdenum, 50);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
}

// Failure: already building
AFL_TEST("game.interface.PlanetMethod:ipmBuildBase:error:already-building", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setBuildBaseFlag(true);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
}

// Failure: base already present
AFL_TEST("game.interface.PlanetMethod:ipmBuildBase:error:base-present", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setBuildBaseFlag(false);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBase, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildBase:error:arity", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBase, seg), interpreter::Error);
}

/*
 *  ipmAutoBuild
 */

// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmAutoBuild:normal", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    call(env, pl, game::interface::ipmAutoBuild, seg);

    a.checkEqual("01. MineBuilding",        pl.getNumBuildings(game::MineBuilding).orElse(-1), 28);
    a.checkEqual("02. DefenseBuilding",     pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 18);
    a.checkEqual("03. FactoryBuilding",     pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
    a.checkEqual("04. BaseDefenseBuilding", pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 0);
    a.checkEqual("05. Supplies",            pl.getCargo(Element::Supplies).orElse(-1), 0);
    a.checkEqual("06. Money",               pl.getCargo(Element::Money).orElse(-1), 14878);
}

// With starbase
AFL_TEST("game.interface.PlanetMethod:ipmAutoBuild:base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    call(env, pl, game::interface::ipmAutoBuild, seg);

    a.checkEqual("11. MineBuilding",        pl.getNumBuildings(game::MineBuilding).orElse(-1), 28);
    a.checkEqual("12. DefenseBuilding",     pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 18);
    a.checkEqual("13. FactoryBuilding",     pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
    a.checkEqual("14. BaseDefenseBuilding", pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 20);
    a.checkEqual("15. Supplies",            pl.getCargo(Element::Supplies).orElse(-1), 0);
    a.checkEqual("16. Money",               pl.getCargo(Element::Money).orElse(-1), 14778);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmAutoBuild:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoBuild, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmAutoBuild:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoBuild, seg), interpreter::Error);
}

/*
 *  ipmBuildDefense
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:normal", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmBuildDefense, seg);

    a.checkEqual("01. DefenseBuilding", pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 35);
    a.checkEqual("02. Money",           pl.getCargo(Element::Money).orElse(-1), 14800);
}

// Limit exceeded
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:partial", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildDefense, seg));

    a.checkEqual("11. DefenseBuilding", pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 56);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 14590);
    interpreter::test::verifyNewInteger(a("13. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 159);
}

// Try to scrap with no reverter
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:no-reverter", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(-20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
}

// Try to scrap with reverter, exceeding limit
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:revert-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    env.turn->universe().setNewReverter(new Reverter());

    afl::data::Segment seg;
    seg.pushBackInteger(-20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
}

// Try to scrap with reverter, exceeding limit, partial scrap allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:revert-partial", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    env.turn->universe().setNewReverter(new Reverter());

    afl::data::Segment seg;
    seg.pushBackInteger(-20);
    seg.pushBackString("N");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildDefense, seg));
    a.checkEqual("02. DefenseBuilding", pl.getNumBuildings(game::DefenseBuilding).orElse(-1), 0);
    interpreter::test::verifyNewInteger(a("03. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), -5);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildDefense:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildDefense, seg), game::Exception);
}

/*
 *  ipmBuildFactories
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmBuildFactories, seg);

    a.checkEqual("01. FactoryBuilding", pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 50);
    a.checkEqual("02. Money",           pl.getCargo(Element::Money).orElse(-1), 14940);
}

// Limit exceeded
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:error:limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFactories, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:partial", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildFactories, seg));

    a.checkEqual("11. FactoryBuilding", pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 90);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 14820);
    interpreter::test::verifyNewInteger(a("13. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 140);
}

// Null
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:null", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildFactories, seg);

    a.checkEqual("01. FactoryBuilding", pl.getNumBuildings(game::FactoryBuilding).orElse(-1), 30);
    a.checkEqual("02. Money",           pl.getCargo(Element::Money).orElse(-1), 15000);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFactories, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFactories, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildFactories:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFactories, seg), game::Exception);
}


/*
 *  ipmBuildMines
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmBuildMines, seg);

    a.checkEqual("01. MineBuilding", pl.getNumBuildings(game::MineBuilding).orElse(-1), 40);
    a.checkEqual("02. Money",        pl.getCargo(Element::Money).orElse(-1), 14920);
}

// Limit exceeded
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:error:limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildMines, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:partial", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Colonists, 90);
    pl.setCargo(Element::Supplies, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildMines, seg));

    a.checkEqual("11. MineBuilding", pl.getNumBuildings(game::MineBuilding).orElse(-1), 90);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 14720);
    interpreter::test::verifyNewInteger(a("13. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 130);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildMines, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildMines, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildMines:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildMines, seg), game::Exception);
}

/*
 *  ipmSetColonistTax
 */


// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:normal", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmSetColonistTax, seg);
    a.checkEqual("getColonistTax", pl.getColonistTax().orElse(-1), 20);
}

// Null does not change the value
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:null", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmSetColonistTax, seg);
    a.checkEqual("getColonistTax", pl.getColonistTax().orElse(-1), 3);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:error:arity", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
}

// Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:error:not-played", a)
{
    Environment env;
    Planet pl(77);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:error:type", a)
{
    Environment env;
    Planet pl(77);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
}

// Range error
AFL_TEST("game.interface.PlanetMethod:ipmSetColonistTax:error:range", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(101);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetColonistTax, seg), interpreter::Error);
}

/*
 *  ipmSetNativeTax
 */


// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:normal", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmSetNativeTax, seg);
    a.checkEqual("getNativeTax", pl.getNativeTax().orElse(-1), 20);
}

// Null does not change the value
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:null", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmSetNativeTax, seg);
    a.checkEqual("getNativeTax", pl.getNativeTax().orElse(-1), 12);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:error:arity", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
}

// Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:error:not-played", a)
{
    Environment env;
    Planet pl(77);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:error:type", a)
{
    Environment env;
    Planet pl(77);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
}

// Range error
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:error:range", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(101);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
}

// No natives
AFL_TEST("game.interface.PlanetMethod:ipmSetNativeTax:error:no-natives", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    pl.setNativeRace(0);
    pl.setNatives(0);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetNativeTax, seg), interpreter::Error);
}

/*
 *  ipmSetFCode
 */

// Set friendly code
AFL_TEST("game.interface.PlanetMethod:ipmSetFCode:success", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("abc");
    call(env, pl, game::interface::ipmSetFCode, seg);
    a.checkEqual("getFriendlyCode", pl.getFriendlyCode().orElse(""), "abc");
}

// Null does not change the value
AFL_TEST("game.interface.PlanetMethod:ipmSetFCode:null", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmSetFCode, seg);
    a.checkEqual("getFriendlyCode", pl.getFriendlyCode().orElse(""), "jkl");
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSetFCode:error:arity", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetFCode, seg), interpreter::Error);
}

// Not played (generates Error::notAssignable, not Exception::eNotPlaying!)
AFL_TEST("game.interface.PlanetMethod:ipmSetFCode:error:not-played", a)
{
    Environment env;
    Planet pl(77);

    afl::data::Segment seg;
    seg.pushBackString("abc");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetFCode, seg), interpreter::Error);
}

/*
 *  ipmSetMission
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setBaseMission(1);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmSetMission, seg);

    a.checkEqual("getBaseMission", pl.getBaseMission().orElse(-1), 5);
}

// Null
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:null", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setBaseMission(1);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmSetMission, seg);

    a.checkEqual("getBaseMission", pl.getBaseMission().orElse(-1), 1);
}

// Range error
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:error:range", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setBaseMission(1);

    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    AFL_CHECK_THROWS(a("01. call"), call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
    a.checkEqual("02. getBaseMission", pl.getBaseMission().orElse(-1), 1);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmSetMission:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetMission, seg), interpreter::Error);
}

/*
 *  ipmBuildBaseDefense
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    call(env, pl, game::interface::ipmBuildBaseDefense, seg);

    a.checkEqual("01. BaseDefenseBuilding", pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 30);
    a.checkEqual("02. Money",               pl.getCargo(Element::Money).orElse(-1), 14800);
}

// Limit exceeded
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:error:limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:partial", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 10000);

    afl::data::Segment seg;
    seg.pushBackInteger(200);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildBaseDefense, seg));

    a.checkEqual("11. BaseDefenseBuilding", pl.getNumBuildings(game::BaseDefenseBuilding).orElse(-1), 200);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 13100);
    interpreter::test::verifyNewInteger(a("13. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 10);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBaseDefense, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBaseDefense, seg), interpreter::Error);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
}

// No base, but accepting partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:partial-no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    seg.pushBackString("n");
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildBaseDefense, seg));
    interpreter::test::verifyNewInteger(a("02. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 20);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildBaseDefense:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(20);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBaseDefense, seg), game::Exception);
}

/*
 *  ipmSetTech
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmSetTech, seg);

    a.checkEqual("01. BeamTech", pl.getBaseTechLevel(game::BeamTech).orElse(-1), 5);
    a.checkEqual("02. Money", pl.getCargo(Element::Money).orElse(-1), 14000);
}

// Null index
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:null-index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmSetTech, seg);

    a.checkEqual("11. BeamTech", pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 15000);
}

// Null level
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:null-level", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmSetTech, seg);

    a.checkEqual("21. BeamTech", pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
    a.checkEqual("22. Money", pl.getCargo(Element::Money).orElse(-1), 15000);
}

// Index range error
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
}

// Level range error
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:level-range", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(15);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
}

// Level not permitted by key
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:level-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(9);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmSetTech:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSetTech, seg), game::Exception);
}

/*
 *  ipmBuildFighters
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmBuildFighters, seg);

    a.checkEqual("01. Fighters", pl.getCargo(Element::Fighters).orElse(-1), 10);
}

// Failure, not enough resources
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Money, 50);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:partial", a)
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

    a.checkEqual("01. Fighters", pl.getCargo(Element::Fighters).orElse(-1), 8);
    interpreter::test::verifyNewInteger(a("02. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 2);
}

// Ship target
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:ship-target", a)
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

    a.checkEqual("Fighters", sh.getCargo(Element::Fighters).orElse(-1), 5);
}

// Failure, bad ship target
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:bad-ship", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(66);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
}

// Failure, ship target has no fighters
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:not-a-carrier", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildFighters:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildFighters, seg), game::Exception);
}

/*
 *  ipmBuildEngines
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);      // Nova drive
    seg.pushBackInteger(3);
    call(env, pl, game::interface::ipmBuildEngines, seg);

    a.checkEqual("engine storage", pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
}

// Null amount
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:null-amount", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildEngines, seg);

    a.checkEqual("engine storage", pl.getBaseStorage(game::EngineTech, 4).orElse(-1), 0);
}

// Null type
AFL_TEST_NOARG("game.interface.PlanetMethod:ipmBuildEngines:null-type")
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
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 10);

    afl::data::Segment seg;
    seg.pushBackInteger(5);     // Nova drive costs 3 duranium
    seg.pushBackInteger(7);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:partial", a)
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

    a.checkEqual("01. engine storage", pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
    a.checkEqual("02. Duranium", pl.getCargo(Element::Duranium).orElse(-1), 1);
    interpreter::test::verifyNewInteger(a("03. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 4);
}

// Try to scrap with no reverter
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:no-reverter", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setBaseStorage(game::EngineTech, 5, 10);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(-7);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

// Try to scrap with reverter, not exceeding limit
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:revert", a)
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
    AFL_CHECK_SUCCEEDS(a("51. call"), call(env, pl, game::interface::ipmBuildEngines, seg));
    a.checkEqual("52. engine storage", pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
}

// Try to scrap with reverter, exceeding limit
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:revert-limit", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

// Try to scrap with reverter, exceeding limit, partial scrap allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:partial-revert", a)
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
    AFL_CHECK_SUCCEEDS(a("01. call"), call(env, pl, game::interface::ipmBuildEngines, seg));
    interpreter::test::verifyNewInteger(a("02. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), -5);
}

// Failure, tech not allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:tech-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(9);
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

// Failure, bad index
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:bad-type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(11);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildEngines:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildEngines, seg), game::Exception);
}

/*
 *  ipmBuildHulls
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmBuildHulls, seg);

    a.checkEqual("hull storage", pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 5);
}

// Null amount
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:null-amount", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildHulls, seg);

    a.checkEqual("hull storage", pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 0);
}

// Null type
AFL_TEST_NOARG("game.interface.PlanetMethod:ipmBuildHulls:null-type")
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
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 20);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);     // costs 7 Duranium
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:partial", a)
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

    a.checkEqual("01. hull storage", pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 2);
    a.checkEqual("02", pl.getCargo(Element::Duranium).orElse(-1), 6);
    interpreter::test::verifyNewInteger(a("03. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 3);
}

// Failure, tech not allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:tech-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    env.shipList->hulls().get(HULL_ID)->setTechLevel(10);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
}

// Failure, bad index
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:bad-index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(111);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
}

// Failure, valid index but not buildable
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:not-buildable", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    env.shipList->hulls().create(HULL_ID+1);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID+1);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildHulls:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(HULL_ID);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildHulls, seg), game::Exception);
}

/*
 *  ipmBuildLaunchers
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmBuildLaunchers, seg);

    a.checkEqual("launcher storage", pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 5);
}

// Null amount
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:null-amount", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildLaunchers, seg);

    a.checkEqual("launcher storage", pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 0);
}

// Null type
AFL_TEST_NOARG("game.interface.PlanetMethod:ipmBuildLaunchers:null-type")
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
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 15);

    afl::data::Segment seg;
    seg.pushBackInteger(3);     // Mark 2 Photon costs 4 Duranium
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:partial", a)
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

    a.checkEqual("01. launcher storage", pl.getBaseStorage(game::TorpedoTech, 3).orElse(-1), 3);
    a.checkEqual("02. Duranium", pl.getCargo(Element::Duranium).orElse(-1), 3);
    interpreter::test::verifyNewInteger(a("03. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 2);
}

// Failure, tech not allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:tech-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(10);
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
}

// Failure, bad index
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:bad-index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(11);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildLaunchers:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildLaunchers, seg), game::Exception);
}

/*
 *  ipmBuildBeams
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmBuildBeams, seg);

    a.checkEqual("beam storage", pl.getBaseStorage(game::BeamTech, 4).orElse(-1), 5);
}

// Null amount
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:null-amount", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildBeams, seg);

    a.checkEqual("beam storage", pl.getBaseStorage(game::BeamTech, 4).orElse(-1), 0);
}

// Null type
AFL_TEST_NOARG("game.interface.PlanetMethod:ipmBuildBeams:null-type")
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
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 5);

    afl::data::Segment seg;
    seg.pushBackInteger(3);     // Plasma Bolt costs 2 Duranium
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:partial", a)
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

    a.checkEqual("01. beam storage", pl.getBaseStorage(game::BeamTech, 3).orElse(-1), 2);
    a.checkEqual("02. Duranium", pl.getCargo(Element::Duranium).orElse(-1), 1);
    interpreter::test::verifyNewInteger(a("03. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 3);
}

// Failure, tech not allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:tech-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(10);
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
}

// Failure, bad index
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:bad-index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(11);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildBeams:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildBeams, seg), game::Exception);
}

/*
 *  ipmBuildTorps
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:success", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    call(env, pl, game::interface::ipmBuildTorps, seg);

    a.checkEqual("torp storage", pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 5);
}

// Null amount
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:null-amount", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    call(env, pl, game::interface::ipmBuildTorps, seg);

    a.checkEqual("torp storage", pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 0);
}

// Null type
AFL_TEST_NOARG("game.interface.PlanetMethod:ipmBuildTorps:null-type")
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
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:resources", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);
    pl.setCargo(Element::Duranium, 2);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

// Partial build
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:partial", a)
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

    a.checkEqual("01. torp storage", pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 2);
    interpreter::test::verifyNewInteger(a("02. remainder"), env.proc.getVariable("BUILD.REMAINDER").release(), 3);
}

// Ship target
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:ship-target", a)
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

    a.checkEqual("ship torp count", sh.getCargo(Element::fromTorpedoType(4)).orElse(-1), 5);
}

// Failure, tech not allowed
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:tech-limit", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(10);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

// Failure, bad index
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:bad-index", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(11);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
}

// Failure, bad ship target
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:bad-ship", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    seg.pushBackInteger(66);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

// Failure, ship target has no torps
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:not-a-torper", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:arity", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:type", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), interpreter::Error);
}

// Not played
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:not-played", a)
{
    Environment env;
    Planet pl(111);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

// No base
AFL_TEST("game.interface.PlanetMethod:ipmBuildTorps:error:no-base", a)
{
    Environment env;
    Planet pl(111);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildTorps, seg), game::Exception);
}

/*
 *  ipmSellSupplies
 */


// Success case
AFL_TEST("game.interface.PlanetMethod:ipmSellSupplies:success", a)
{
    Environment env;
    Planet pl(55);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Supplies, 100);
    pl.setCargo(Element::Money, 50);

    afl::data::Segment seg;
    seg.pushBackInteger(30);
    call(env, pl, game::interface::ipmSellSupplies, seg);

    a.checkEqual("01. Supplies", pl.getCargo(Element::Supplies).orElse(-1), 70);
    a.checkEqual("02. Money", pl.getCargo(Element::Money).orElse(-1), 80);
}

// Overflow case
AFL_TEST("game.interface.PlanetMethod:ipmSellSupplies:overflow", a)
{
    Environment env;
    Planet pl(55);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Supplies, 100);
    pl.setCargo(Element::Money, 50);

    afl::data::Segment seg;
    seg.pushBackInteger(130);
    AFL_CHECK_THROWS(a("01. call"), call(env, pl, game::interface::ipmSellSupplies, seg), game::Exception);

    a.checkEqual("11. Supplies", pl.getCargo(Element::Supplies).orElse(-1), 100);
    a.checkEqual("12. Money", pl.getCargo(Element::Money).orElse(-1), 50);
}

// Partial
AFL_TEST("game.interface.PlanetMethod:ipmSellSupplies:partial", a)
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

    a.checkEqual("01. Supplies", pl.getCargo(Element::Supplies).orElse(-1), 0);
    a.checkEqual("02. Money", pl.getCargo(Element::Money).orElse(-1), 150);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmSellSupplies:error:arity", a)
{
    Environment env;
    Planet pl(55);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Supplies, 100);
    pl.setCargo(Element::Money, 50);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSellSupplies, seg), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmSellSupplies:error:type", a)
{
    Environment env;
    Planet pl(55);
    configurePlayablePlanet(env, pl);
    pl.setCargo(Element::Supplies, 100);
    pl.setCargo(Element::Money, 50);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmSellSupplies, seg), interpreter::Error);
}

/*
 *  ipmBuildShip
 */

// Success case
AFL_TEST("game.interface.PlanetMethod:ipmBuildShip:success", a)
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

    a.checkEqual("01. hull storage",     pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 1);
    a.checkEqual("02. engine storage",   pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 2);
    a.checkEqual("03. beam storage",     pl.getBaseStorage(game::BeamTech, 2).orElse(-1), 3);
    a.checkEqual("04. launcher storage", pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 7);

    a.checkEqual("11. HullTech",         pl.getBaseTechLevel(game::HullTech).orElse(-1), 5);
    a.checkEqual("12. EngineTech",       pl.getBaseTechLevel(game::EngineTech).orElse(-1), 5);
    a.checkEqual("13. BeamTech",         pl.getBaseTechLevel(game::BeamTech).orElse(-1), 1);
    a.checkEqual("14. TorpedoTech",      pl.getBaseTechLevel(game::TorpedoTech).orElse(-1), 3);

    a.checkEqual("21. getHullIndex",     pl.getBaseBuildOrder().getHullIndex(), HULL_SLOT);

    a.checkEqual("31. Money",            pl.getCargo(Element::Money).orElse(-1), 12502);

    // We can also cancel
    afl::data::Segment seg2;
    seg2.pushBackInteger(0);
    call(env, pl, game::interface::ipmBuildShip, seg2);

    a.checkEqual("41. getHullIndex",     pl.getBaseBuildOrder().getHullIndex(), 0);
}

// Failure case: no base
AFL_TEST("game.interface.PlanetMethod:ipmBuildShip:error:no-base", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildShip, seg), game::Exception);
}

// Failure case: no tech
AFL_TEST("game.interface.PlanetMethod:ipmBuildShip:error:tech-limit", a)
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
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmBuildShip, seg), game::Exception);
}

/*
 *  ipmCargoTransfer
 */


// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmCargoTransfer:normal", a)
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

    a.checkEqual("01. ship Neutronium",   sh.getCargo(Element::Neutronium).orElse(-1), 30);
    a.checkEqual("02. planet Neutronium", pl.getCargo(Element::Neutronium).orElse(-1), 100);
}

// Partial case
AFL_TEST("game.interface.PlanetMethod:ipmCargoTransfer:partial", a)
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

    a.checkEqual("01. ship Neutronium",   sh.getCargo(Element::Neutronium).orElse(-1), 100);
    a.checkEqual("02. planet Neutronium", pl.getCargo(Element::Neutronium).orElse(-1), 30);
    interpreter::test::verifyNewString(a("03. remainder"), env.proc.getVariable("CARGO.REMAINDER").release(), "110N");
}

// Error case, bad Id
AFL_TEST("game.interface.PlanetMethod:ipmCargoTransfer:error:bad-id", a)
{
    Environment env;
    Planet& pl = *env.turn->universe().planets().create(44);
    configurePlayablePlanet(env, pl);

    afl::data::Segment seg;
    seg.pushBackString("n200");
    seg.pushBackInteger(77);
    seg.pushBackString("n");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmCargoTransfer, seg), game::Exception);
}

/*
 *  ipmAutoTaxColonists
 */

// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxColonists:normal", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    call(env, pl, game::interface::ipmAutoTaxColonists, seg);
    a.checkEqual("getColonistTax", pl.getColonistTax().orElse(-1), 10);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxColonists:error:arity", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoTaxColonists, seg), interpreter::Error);
}

// Planet not played
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxColonists:error:unplayed", a)
{
    Environment env;
    Planet pl(77);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoTaxColonists, seg), game::Exception);
}

/*
 *  ipmAutoTaxNatives
 */

// Normal case
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxNatives:success", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    call(env, pl, game::interface::ipmAutoTaxNatives, seg);
    a.checkEqual("getNativeTax", pl.getNativeTax().orElse(-1), 2);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxNatives:error:arity", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoTaxNatives, seg), interpreter::Error);
}

// No natives
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxNatives:error:no-natives", a)
{
    Environment env;
    Planet pl(77);
    configurePlayablePlanet(env, pl);
    pl.setNatives(0);
    pl.setNativeRace(0);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoTaxNatives, seg), game::Exception);
}

// Planet not played
AFL_TEST("game.interface.PlanetMethod:ipmAutoTaxNatives:error:unplayed", a)
{
    Environment env;
    Planet pl(77);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmAutoTaxNatives, seg), game::Exception);
}

/*
 *  ipmApplyBuildGoals
 */


// Standard case: modify everything
AFL_TEST("game.interface.PlanetMethod:ipmApplyBuildGoals:full", a)
{
    Environment env;
    Planet pl(77);
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

    a.checkEqual("01", pl.getAutobuildGoal(game::MineBuilding), 100);
    a.checkEqual("02", pl.getAutobuildGoal(game::FactoryBuilding), 200);
    a.checkEqual("03", pl.getAutobuildGoal(game::DefenseBuilding), 300);
    a.checkEqual("04", pl.getAutobuildGoal(game::BaseDefenseBuilding), 400);

    a.checkEqual("11", pl.getAutobuildSpeed(game::MineBuilding), 11);
    a.checkEqual("12", pl.getAutobuildSpeed(game::FactoryBuilding), 22);
    a.checkEqual("13", pl.getAutobuildSpeed(game::DefenseBuilding), 33);
    a.checkEqual("14", pl.getAutobuildSpeed(game::BaseDefenseBuilding), 44);
}

// Modify parts
AFL_TEST("game.interface.PlanetMethod:ipmApplyBuildGoals:partial", a)
{
    Environment env;
    Planet pl(77);
    Planet::AutobuildSettings abs;
    abs.goal[game::MineBuilding] = 88;
    abs.speed[game::DefenseBuilding] = 55;

    afl::data::Segment seg;
    seg.pushBackNew(new game::interface::AutobuildSettingsValue_t(abs));
    call(env, pl, game::interface::ipmApplyBuildGoals, seg);

    a.checkEqual("01", pl.getAutobuildGoal(game::MineBuilding), 88);
    a.checkEqual("02", pl.getAutobuildGoal(game::FactoryBuilding), 1000);
    a.checkEqual("03", pl.getAutobuildGoal(game::DefenseBuilding), 1000);
    a.checkEqual("04", pl.getAutobuildGoal(game::BaseDefenseBuilding), 20);

    a.checkEqual("11", pl.getAutobuildSpeed(game::MineBuilding), 5);
    a.checkEqual("12", pl.getAutobuildSpeed(game::FactoryBuilding), 10);
    a.checkEqual("13", pl.getAutobuildSpeed(game::DefenseBuilding), 55);
    a.checkEqual("14", pl.getAutobuildSpeed(game::BaseDefenseBuilding), 2);
}

// Type error
AFL_TEST("game.interface.PlanetMethod:ipmApplyBuildGoals:error:type", a)
{
    Environment env;
    Planet pl(77);
    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmApplyBuildGoals, seg), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.PlanetMethod:ipmApplyBuildGoals:error:arity", a)
{
    Environment env;
    Planet pl(77);
    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, call(env, pl, game::interface::ipmApplyBuildGoals, seg), interpreter::Error);
}
