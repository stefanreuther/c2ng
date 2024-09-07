/**
  *  \file test/server/play/planetpackertest.cpp
  *  \brief Test for server::play::PlanetPacker
  */

#include <stdexcept>
#include "server/play/planetpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/hostversion.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using game::Game;
using game::config::HostConfiguration;
using game::map::Planet;

/** Test all planet properties. */
AFL_TEST("server.play.PlanetPacker:planet", a)
{
    const int ID = 42;
    const int PLAYER = 7;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Configuration for experience
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr();
    r->hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);
    r->hostConfiguration()[HostConfiguration::EPPlanetAging].set(42);
    r->hostConfiguration()[HostConfiguration::EPPlanetGovernment].set(50);
    r->hostConfiguration()[HostConfiguration::ExperienceLevelNames].set("Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm");
    session.setRoot(r);

    // Shiplist must be present, but can be empty
    session.setShipList(new game::spec::ShipList());

    // Game containing the planet we show
    afl::base::Ptr<Game> g = new Game();
    session.setGame(g);

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
    pd.money             = 458;
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

    Planet& pl = *g->currentTurn().universe().planets().create(ID);
    pl.setPosition(game::map::Point(1030, 2700));
    pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
    pl.setPlayability(game::map::Object::Playable);

    // Level
    game::UnitScoreDefinitionList::Definition levelDef;
    levelDef.name  = "Level";
    levelDef.id    = game::ScoreId_ExpLevel;
    levelDef.limit = -1;
    pl.unitScores().set(g->planetScores().add(levelDef), 3, 10);

    // Comment
    session.world().planetProperties().create(ID)->setNew(interpreter::World::pp_Comment, interpreter::makeStringValue("note"));

    // Test it!
    server::play::PlanetPacker testee(session, ID);
    a.checkEqual("01. name", testee.getName(), "planet42");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());
    a.checkEqual("11", ap("BASE.BUILDING").toInteger(), 1);
    a.checkEqual("12", ap("COLONISTS.HAPPY").toInteger(), 97);
    a.checkEqual("13", ap("COLONISTS.SUPPORTED").toInteger(), 53000);
    a.checkEqual("14", ap("COLONISTS.TAX").toInteger(), 3);
    a.checkEqual("15", ap("COMMENT").toString(), "note");
    a.checkNull ("16", ap("DAMAGE").getValue());
    a.checkEqual("17", ap("DEFENSE").toInteger(), 15);
    a.checkEqual("18", ap("DEFENSE.SPEED").toInteger(), 3);
    a.checkEqual("19", ap("DEFENSE.WANT").toInteger(), 1000);
    a.checkEqual("20", ap("DENSITY.N").toInteger(), 14);
    a.checkEqual("21", ap("DENSITY.T").toInteger(), 87);
    a.checkEqual("22", ap("DENSITY.D").toInteger(), 29);
    a.checkEqual("23", ap("DENSITY.M").toInteger(), 7);
    a.checkEqual("24", ap("FACTORIES").toInteger(), 30);
    a.checkEqual("25", ap("FACTORIES.SPEED").toInteger(), 10);
    a.checkEqual("26", ap("FACTORIES.WANT").toInteger(), 1000);
    a.checkEqual("27", ap("FCODE").toString(), "jkl");
    a.checkEqual("28", ap("GROUND.N").toInteger(), 1092);
    a.checkEqual("29", ap("GROUND.T").toInteger(), 9102);
    a.checkEqual("30", ap("GROUND.D").toInteger(), 349);
    a.checkEqual("31", ap("GROUND.M").toInteger(), 781);
    a.checkEqual("32", ap("INDUSTRY").toInteger(), 1);
    a.checkEqual("33", ap("LEVEL").toInteger(), 3);
    a.checkEqual("34", ap("MINES").toInteger(), 20);
    a.checkEqual("35", ap("MINES.SPEED").toInteger(), 5);
    a.checkEqual("36", ap("MINES.WANT").toInteger(), 1000);
    a.checkEqual("37", ap("NATIVES").toInteger(), 7821);
    a.checkEqual("38", ap("NATIVES.GOV").toInteger(), 4);
    a.checkEqual("39", ap("NATIVES.HAPPY").toInteger(), 76);
    a.checkEqual("40", ap("NATIVES.RACE").toInteger(), 3);
    a.checkEqual("41", ap("NATIVES.TAX").toInteger(), 12);
    a.checkEqual("42", ap("TEMP").toInteger(), 53);
    a.checkEqual("43", ap("G")("N").toInteger(), 120);
    a.checkEqual("44", ap("G")("T").toInteger(), 84);
    a.checkEqual("45", ap("G")("D").toInteger(), 76);
    a.checkEqual("46", ap("G")("M").toInteger(), 230);
    a.checkEqual("47", ap("G")("COLONISTS").toInteger(), 1200);
    a.checkEqual("48", ap("G")("SUPPLIES").toInteger(), 31);
    a.checkEqual("49", ap("G")("MC").toInteger(), 458);
}

/** Test with a starbase, and a selection of properties. */
AFL_TEST("server.play.PlanetPacker:base", a)
{
    const int ID = 42;
    const int PLAYER = 5;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Empty root
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());

    // Shiplist provides dimensions for storage attributes
    afl::base::Ptr<game::spec::ShipList> sl = new game::spec::ShipList();
    sl->hulls().create(100);
    sl->beams().create(10);
    sl->launchers().create(10);
    sl->engines().create(9);
    for (int i = 1; i < 17; ++i) {
        sl->hullAssignments().add(PLAYER, i, 100);
    }
    session.setShipList(sl);

    // Game containing the planet we show
    afl::base::Ptr<Game> g = new Game();
    session.setGame(g);

    // Planet
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
    bd.beamStorage.set(2, 10);
    bd.engineStorage.set(3, 20);
    bd.launcherStorage.set(4, 30);
    bd.hullStorage.set(9, 5);
    bd.torpedoStorage.set(5, 15);
    bd.mission = 2;
    bd.damage = 7;
    bd.numFighters = 22;

    bd.shipBuildOrder.setHullIndex(3);
    bd.shipBuildOrder.setBeamType(5);
    bd.shipBuildOrder.setNumBeams(6);
    bd.shipBuildOrder.setTorpedoType(7);
    bd.shipBuildOrder.setNumLaunchers(8);
    bd.shipBuildOrder.setEngineType(9);

    Planet& pl = *g->currentTurn().universe().planets().create(ID);
    pl.setPosition(game::map::Point(1030, 2700));
    pl.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));
    pl.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));
    pl.setPlayability(game::map::Object::Playable);
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), 10, tx, session.log());

    // Test it!
    server::play::PlanetPacker testee(session, ID);
    a.checkEqual("01. name", testee.getName(), "planet42");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());
    a.checkEqual("11", ap("BUILD")("BEAM").toInteger(), 5);
    a.checkEqual("12", ap("BUILD")("BEAM.COUNT").toInteger(), 6);
    a.checkEqual("13", ap("BUILD")("ENGINE").toInteger(), 9);
    a.checkEqual("14", ap("BUILD")("HULL").toInteger(), 100);
    a.checkEqual("15", ap("BUILD")("TORP").toInteger(), 7);
    a.checkEqual("16", ap("BUILD")("TORP.COUNT").toInteger(), 8);

    a.checkEqual("21", ap("DAMAGE").toInteger(), 7);
    a.checkEqual("22", ap("DEFENSE.BASE").toInteger(), 10);
    a.checkEqual("23", ap("FIGHTERS").toInteger(), 22);

    a.checkEqual("31", ap("STORAGE.AMMO")[11].toInteger(), 22);
    a.checkEqual("32", ap("STORAGE.AMMO")[5].toInteger(), 15);
    a.checkEqual("33", ap("STORAGE.BEAMS")[2].toInteger(), 10);
    a.checkEqual("34", ap("STORAGE.ENGINES")[3].toInteger(), 20);
    a.checkEqual("35", ap("STORAGE.HULLS")[9].toInteger(), 5);
    a.checkEqual("36", ap("STORAGE.LAUNCHERS")[4].toInteger(), 30);
}

/** Test error case. */
AFL_TEST("server.play.PlanetPacker:error:no-planet", a)
{
    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Empty environment
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new Game());

    // Test it!
    server::play::PlanetPacker testee(session, 77);
    AFL_CHECK_THROWS(a, testee.buildValue(), std::exception);
}

/** Test error case. */
AFL_TEST("server.play.PlanetPacker:error:no-env", a)
{
    const int ID = 99;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Game with planet present, rest missing
    afl::base::Ptr<Game> g = new Game();
    session.setGame(g);
    g->currentTurn().universe().planets().create(ID);

    // Test it!
    server::play::PlanetPacker testee(session, ID);
    AFL_CHECK_THROWS(a, testee.buildValue(), std::exception);
}
