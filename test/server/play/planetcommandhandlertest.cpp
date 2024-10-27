/**
  *  \file test/server/play/planetcommandhandlertest.cpp
  *  \brief Test for server::play::PlanetCommandHandler
  */

#include "server/play/planetcommandhandler.hpp"

#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/ship.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "game/types.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/values.hpp"
#include "server/play/packerlist.hpp"

using afl::data::Segment;
using game::Element;
using game::HostVersion;
using game::map::Planet;
using game::map::Ship;
using game::spec::Cost;
using interpreter::Arguments;

namespace {
    const int TURN_NR = 10;
    const int PLAYER = 4;
    const int HULL_ID = 5;
    const int HULL_SLOT = 7;

    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            {
                // Root
                session.setRoot(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,1,0))).asPtr());

                // Shiplist
                afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
                session.setShipList(shipList);
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
                h.cost().set(Cost::Tritanium, 5);
                h.cost().set(Cost::Duranium, 7);
                h.cost().set(Cost::Molybdenum, 9);
                h.cost().set(Cost::Money, 100);

                // More properties
                shipList->hullAssignments().add(PLAYER, HULL_SLOT, HULL_ID);
                game::test::initStandardBeams(*shipList);
                game::test::initStandardTorpedoes(*shipList);
                game::test::addNovaDrive(*shipList);
                game::test::addTranswarp(*shipList);

                // Game
                session.setGame(new game::Game());
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
        pd.minedNeutronium   = 200;
        pd.minedTritanium    = 500;
        pd.minedDuranium     = 500;
        pd.minedMolybdenum   = 500;
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
        pd.baseFlag          = 0;

        pl.setPosition(game::map::Point(1030, 2700));
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
        pl.setName("Earth 2");
        pl.setPlayability(game::map::Object::Playable);
        pl.internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, env.tx, env.session.log());
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
        pl.internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, env.tx, env.session.log());
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

    Planet& makePlanet(Environment& env, int id)
    {
        return *env.session.getGame()->currentTurn().universe().planets().create(id);
    }

    Ship& makeShip(Environment& env, int id)
    {
        return *env.session.getGame()->currentTurn().universe().ships().create(id);
    }

    void call(server::play::CommandHandler& testee, String_t cmd, Segment& seg)
    {
        server::play::PackerList list;
        Arguments args(seg, 0, seg.size());
        testee.processCommand(cmd, args, list);
    }
}

/*
 *  Happy path for all commands - test cases partially derived from PlanetMethodTest
 */

/** Test 'setcomment'. */
AFL_TEST("server.play.PlanetCommandHandler:setcomment", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackString("hi");
    call(testee, "setcomment", args);

    a.checkEqual("01. comment", interpreter::toString(env.session.world().planetProperties().get(100, interpreter::World::pp_Comment), false), "hi");
}

/** Test 'setfcode'. */
AFL_TEST("server.play.PlanetCommandHandler:setfcode", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackString("rtz");
    call(testee, "setfcode", args);

    a.checkEqual("01. fcode", pl.getFriendlyCode().orElse(""), "rtz");
}

/** Test 'fixship'. */
AFL_TEST("server.play.PlanetCommandHandler:fixship", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    Ship& sh = makeShip(env, 40);
    configurePlayableShip(env, sh);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(40);
    call(testee, "fixship", args);

    a.checkEqual("01. action", pl.getBaseShipyardAction().orElse(-1), game::FixShipyardAction);
    a.checkEqual("02. id",     pl.getBaseShipyardId().orElse(-1), 40);
}

/** Test 'recycleship'. */
AFL_TEST("server.play.PlanetCommandHandler:recycleship", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    Ship& sh = makeShip(env, 40);
    configurePlayableShip(env, sh);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(40);
    call(testee, "recycleship", args);

    a.checkEqual("01. action", pl.getBaseShipyardAction().orElse(-1), game::RecycleShipyardAction);
    a.checkEqual("02. id",     pl.getBaseShipyardId().orElse(-1), 40);
}

/** Test 'buildbase'. */
AFL_TEST("server.play.PlanetCommandHandler:buildbase", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    call(testee, "buildbase", args);

    a.checkEqual("01. baseFlag", pl.isBuildingBase(), true);
    a.checkEqual("02. tri",      pl.getCargo(Element::Tritanium).orElse(0), 98);
    a.checkEqual("03. dur",      pl.getCargo(Element::Duranium).orElse(0), 380);
    a.checkEqual("04. mol",      pl.getCargo(Element::Molybdenum).orElse(0), 160);
    a.checkEqual("05. mc",       pl.getCargo(Element::Money).orElse(0), 14100);
}

/** Test 'autobuild'. */
AFL_TEST("server.play.PlanetCommandHandler:autobuild", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    call(testee, "autobuild", args);

    a.checkEqual("01. mines",      pl.getNumBuildings(game::MineBuilding).orElse(0), 28);
    a.checkEqual("02. factories",  pl.getNumBuildings(game::FactoryBuilding).orElse(0), 50);
    a.checkEqual("03. defense",    pl.getNumBuildings(game::DefenseBuilding).orElse(0), 18);
}

/** Test 'builddefense'. */
AFL_TEST("server.play.PlanetCommandHandler:builddefense", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(7);
    call(testee, "builddefense", args);

    a.checkEqual("01. defense", pl.getNumBuildings(game::DefenseBuilding).orElse(0), 22);
}

/** Test 'buildfactories'. */
AFL_TEST("server.play.PlanetCommandHandler:buildfactories", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(10);
    call(testee, "buildfactories", args);

    a.checkEqual("01. factories", pl.getNumBuildings(game::FactoryBuilding).orElse(0), 40);
}

/** Test 'buildmines'. */
AFL_TEST("server.play.PlanetCommandHandler:buildmines", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(18);
    call(testee, "buildmines", args);

    a.checkEqual("01. mines", pl.getNumBuildings(game::MineBuilding).orElse(0), 38);
}

/** Test 'buildbasedefense'. */
AFL_TEST("server.play.PlanetCommandHandler:buildbasedefense", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(3);
    call(testee, "buildbasedefense", args);

    a.checkEqual("01. defense", pl.getNumBuildings(game::BaseDefenseBuilding).orElse(0), 13);
}

/** Test 'setcolonisttax'. */
AFL_TEST("server.play.PlanetCommandHandler:setcolonisttax", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(15);
    call(testee, "setcolonisttax", args);

    a.checkEqual("01. tax", pl.getColonistTax().orElse(0), 15);
}

/** Test 'setnativetax'. */
AFL_TEST("server.play.PlanetCommandHandler:setnativetax", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(15);
    call(testee, "setnativetax", args);

    a.checkEqual("01. tax", pl.getNativeTax().orElse(0), 15);
}

/** Test 'setmission'. */
AFL_TEST("server.play.PlanetCommandHandler:setmission", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(6);
    call(testee, "setmission", args);

    a.checkEqual("01. mission", pl.getBaseMission().orElse(0), 6);
}

/** Test 'settech'. */
AFL_TEST("server.play.PlanetCommandHandler:settech", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(1);
    args.pushBackInteger(6);
    call(testee, "settech", args);

    a.checkEqual("01. tech", pl.getBaseTechLevel(game::EngineTech).orElse(0), 6);
    a.checkEqual("02. money", pl.getCargo(Element::Money).orElse(0), 13500);
}

/** Test 'buildfighters'. */
AFL_TEST("server.play.PlanetCommandHandler:buildfighters", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(2);
    call(testee, "buildfighters", args);

    a.checkEqual("01. fighters", pl.getCargo(Element::Fighters).orElse(0), 7);
}

/** Test 'buildengines'. */
AFL_TEST("server.play.PlanetCommandHandler:buildengines", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(5);
    args.pushBackInteger(3);
    call(testee, "buildengines", args);

    a.checkEqual("01. count", pl.getBaseStorage(game::EngineTech, 5).orElse(-1), 3);
}

/** Test 'buildtorps'. */
AFL_TEST("server.play.PlanetCommandHandler:buildtorps", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(4);
    args.pushBackInteger(5);
    call(testee, "buildtorps", args);

    a.checkEqual("01. count", pl.getCargo(Element::fromTorpedoType(4)).orElse(-1), 5);
}

/** Test 'buildhulls'. */
AFL_TEST("server.play.PlanetCommandHandler:buildhulls", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(HULL_ID);
    args.pushBackInteger(5);
    call(testee, "buildhulls", args);

    a.checkEqual("01. count", pl.getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 5);
}

/** Test 'buildlaunchers'. */
AFL_TEST("server.play.PlanetCommandHandler:buildlaunchers", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(4);
    args.pushBackInteger(5);
    call(testee, "buildlaunchers", args);

    a.checkEqual("01. count", pl.getBaseStorage(game::TorpedoTech, 4).orElse(-1), 5);
}

/** Test 'buildbeams'. */
AFL_TEST("server.play.PlanetCommandHandler:buildbeams", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(4);
    args.pushBackInteger(5);
    call(testee, "buildbeams", args);

    a.checkEqual("01. count", pl.getBaseStorage(game::BeamTech, 4).orElse(-1), 5);
}

/** Test 'sellsupplies'. */
AFL_TEST("server.play.PlanetCommandHandler:sellsupplies", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(30);
    call(testee, "sellsupplies", args);

    a.checkEqual("01. sup", pl.getCargo(Element::Supplies).orElse(-1), 1);
    a.checkEqual("02. mc",  pl.getCargo(Element::Money).orElse(-1), 15030);
}

/** Test 'buildship'. */
AFL_TEST("server.play.PlanetCommandHandler:buildship", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);
    configurePlayableBase(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(HULL_ID);
    args.pushBackInteger(5);   // Nova drive
    args.pushBackInteger(2);
    args.pushBackInteger(3);   // 3 beams
    args.pushBackInteger(4);
    args.pushBackInteger(7);   // 7 launchers
    call(testee, "buildship", args);

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
}

/** Test 'cargotransfer'. */
AFL_TEST("server.play.PlanetCommandHandler:cargotransfer", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    Ship& sh = makeShip(env, 66);
    configurePlayableShip(env, sh);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackString("n20");
    args.pushBackInteger(66);
    call(testee, "cargotransfer", args);

    a.checkEqual("01. ship Neutronium",   sh.getCargo(Element::Neutronium).orElse(-1), 30);
    a.checkEqual("02. planet Neutronium", pl.getCargo(Element::Neutronium).orElse(-1), 180);
}

/** Test 'setbuildgoals'. */
AFL_TEST("server.play.PlanetCommandHandler:setbuildgoals", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackInteger(2);  // mines
    args.pushBackInteger(3);
    args.pushBackInteger(4);  // factories
    args.pushBackNew(0);
    args.pushBackNew(0);      // defense
    args.pushBackInteger(7);
    args.pushBackInteger(8);  // base defense
    args.pushBackInteger(9);
    call(testee, "setbuildgoals", args);

    a.checkEqual("mine goal", pl.getAutobuildGoal(game::MineBuilding), 2);
    a.checkEqual("mine speed", pl.getAutobuildSpeed(game::MineBuilding), 3);

    a.checkEqual("factory goal", pl.getAutobuildGoal(game::FactoryBuilding), 4);
    a.checkEqual("factory speed", pl.getAutobuildSpeed(game::FactoryBuilding), 10);

    a.checkEqual("defense goal", pl.getAutobuildGoal(game::DefenseBuilding), 1000);
    a.checkEqual("defense speed", pl.getAutobuildSpeed(game::DefenseBuilding), 7);

    a.checkEqual("base-defense goal", pl.getAutobuildGoal(game::BaseDefenseBuilding), 8);
    a.checkEqual("base-defense speed", pl.getAutobuildSpeed(game::BaseDefenseBuilding), 9);
}

/*
 *  Error cases
 *
 *  Only test some specimen; main error handling is in PlanetMethod/PlanetProperty
 */


/** Error: unknown verb. */
AFL_TEST("server.play.PlanetCommandHandler:error:verb", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "buyavowel", args), std::exception);
}

/** Error: unknown verb (this is case sensitive). */
AFL_TEST("server.play.PlanetCommandHandler:error:verb:2", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "SetFCode", args), std::exception);
}

/** Error: type error. */
AFL_TEST("server.play.PlanetCommandHandler:error:type", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    args.pushBackString("1");
    AFL_CHECK_THROWS(a, call(testee, "setcolonisttax", args), std::exception);
}

/** Error: arity error. */
AFL_TEST("server.play.PlanetCommandHandler:error:arity", a)
{
    Environment env;
    Planet& pl = makePlanet(env, 100);
    configurePlayablePlanet(env, pl);

    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "setcolonisttax", args), std::exception);
}

/** Error: missing planet. */
AFL_TEST("server.play.PlanetCommandHandler:error:no-planet", a)
{
    Environment env;
    server::play::PlanetCommandHandler testee(env.session, 100);
    Segment args;
    AFL_CHECK_THROWS(a, call(testee, "setcolonisttax", args), std::exception);
}
