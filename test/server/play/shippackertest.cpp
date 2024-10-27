/**
  *  \file test/server/play/shippackertest.cpp
  *  \brief Test for server::play::ShipPacker
  */

#include "server/play/shippacker.hpp"

#include "afl/data/access.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/map/shipdata.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/values.hpp"

using game::Game;
using game::Session;
using game::Turn;
using game::config::HostConfiguration;
using game::map::Planet;
using game::map::Ship;
using game::spec::BasicHullFunction;

namespace {
    const int TURN_NR = 10;

    void addPlanetXY(Session& session, Game& g, game::Id_t id, int x, int y, String_t name)
    {
        Planet& pl = *g.currentTurn().universe().planets().create(id);
        pl.setPosition(game::map::Point(x, y));
        pl.setName(name);
        pl.internalCheck(g.mapConfiguration(), game::PlayerSet_t(), TURN_NR, session.translator(), session.log());
    }

    void addShipXY(Session& /*session*/, Game& g, game::Id_t id, int x, int y, int owner, int scanner, String_t name)
    {
        Ship& sh = *g.currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(x, y), owner, /* mass */ 400, game::PlayerSet_t(scanner));
        sh.setName(name);
        sh.internalCheck(game::PlayerSet_t(scanner), TURN_NR);
    }
}

AFL_TEST("server.play.ShipPacker", a)
{
    // Test setup lifted from "game.interface.ShipProperty:basics"
    const int PLAYER = 3;
    const int SHIP_ID = 77;
    const int PLANET_ID = 99;
    const int NEAR_SHIP_ID = 222;
    const int X = 1100;
    const int Y = 1300;
    const int DX = 100;
    const int DY = 200;
    const int BEAM_NR = 5;
    const int TORP_NR = 7;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    root->hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);
    root->hostConfiguration()[HostConfiguration::EPShipAging].set(32);
    root->hostConfiguration()[HostConfiguration::ExperienceLevelNames].set("Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm");
    session.setRoot(root.asPtr());

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);
    session.setShipList(shipList.asPtr());

    // - basic hull functions
    shipList->basicHullFunctions().addFunction(BasicHullFunction::Cloak, "Cloaking");
    shipList->basicHullFunctions().addFunction(BasicHullFunction::MerlinAlchemy, "Alchemy");

    // Game/Turn
    afl::base::Ref<Game> g(*new Game());
    afl::base::Ref<Turn> turn(g->currentTurn());
    g->setViewpointPlayer(PLAYER);
    session.setGame(g.asPtr());

    // - related units
    addPlanetXY(session, *g, PLANET_ID,    X,    Y,                      "Marble");
    addShipXY  (session, *g, NEAR_SHIP_ID, X,    Y,    PLAYER+1, PLAYER, "USS Near");

    // Ship under test
    game::map::ShipData sd;
    sd.owner                     = PLAYER;
    sd.friendlyCode              = "fcd";
    sd.warpFactor                = 7;
    sd.waypointDX                = DX;
    sd.waypointDY                = DY;
    sd.x                         = X;
    sd.y                         = Y;
    sd.engineType                = 9;
    sd.hullType                  = game::test::ANNIHILATION_HULL_ID;
    sd.beamType                  = BEAM_NR;
    sd.numBeams                  = 3;
    sd.numBays                   = 0;
    sd.torpedoType               = TORP_NR;
    sd.ammo                      = 200;
    sd.numLaunchers              = 2;
    sd.mission                   = 5;
    sd.primaryEnemy              = 1;
    sd.missionTowParameter       = 0;
    sd.damage                    = 5;
    sd.crew                      = 200;
    sd.colonists                 = 30;
    sd.name                      = "USS Cube";
    sd.neutronium                = 50;
    sd.tritanium                 = 10;
    sd.duranium                  = 9;
    sd.molybdenum                = 8;
    sd.supplies                  = 7;
    sd.unload.neutronium         = 20;
    sd.unload.tritanium          = 21;
    sd.unload.duranium           = 22;
    sd.unload.molybdenum         = 23;
    sd.unload.colonists          = 24;
    sd.unload.supplies           = 25;
    sd.unload.targetId           = PLANET_ID;
    sd.transfer.neutronium       = 3;
    sd.transfer.tritanium        = 4;
    sd.transfer.duranium         = 5;
    sd.transfer.molybdenum       = 6;
    sd.transfer.colonists        = 7;
    sd.transfer.supplies         = 8;
    sd.transfer.targetId         = NEAR_SHIP_ID;
    sd.missionInterceptParameter = 0;
    sd.money                     = 2000;

    // Create ship. Must be part of the universe because MovementPredictor resolves it through it.
    Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.addShipSpecialFunction(shipList->modifiedHullFunctions().getFunctionIdFromHostId(BasicHullFunction::Cloak));
    sh.messages().add(2);
    sh.messages().add(7);
    sh.setPlayability(game::map::Object::Playable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    // Level
    game::UnitScoreDefinitionList::Definition levelDef;
    levelDef.name  = "Level";
    levelDef.id    = game::ScoreId_ExpLevel;
    levelDef.limit = -1;
    sh.unitScores().set(g->shipScores().add(levelDef), 3, TURN_NR);

    // Comment
    session.world().shipProperties().create(SHIP_ID)->setNew(interpreter::World::sp_Comment, interpreter::makeStringValue("note!"));

    // Verify
    server::play::ShipPacker testee(session, SHIP_ID);
    a.checkEqual("01. name", testee.getName(), "ship77");

    // Verify data content
    std::auto_ptr<server::Value_t> value(testee.buildValue());
    afl::data::Access ap(value.get());

    a.checkEqual("11", ap("AUX")              .toInteger(), TORP_NR);
    a.checkEqual("12", ap("AUX.AMMO")         .toInteger(), 200);
    a.checkEqual("13", ap("AUX.COUNT")        .toInteger(), 2);
    a.checkEqual("14", ap("BEAM")             .toInteger(), BEAM_NR);
    a.checkEqual("15", ap("BEAM.COUNT")       .toInteger(), 3);
    a.checkEqual("16", ap("COMMENT")          .toString(), "note!");
    a.checkEqual("17", ap("CREW")             .toInteger(), 200);
    a.checkEqual("18", ap("DAMAGE")           .toInteger(), 5);
    a.checkEqual("19", ap("ENEMY")            .toInteger(), 1);
    a.checkEqual("20", ap("ENGINE")           .toInteger(), 9);
    a.checkEqual("21", ap("FCODE")            .toString(), "fcd");
    a.checkEqual("22", ap("HEADING")          .toInteger(), 26);
    a.checkEqual("23", ap("HULL")             .toInteger(), game::test::ANNIHILATION_HULL_ID);
    a.checkEqual("24", ap("LEVEL")            .toInteger(), 3);
    a.checkEqual("25", ap("MISSION")          .toInteger(), 5);
    a.checkEqual("26", ap("MISSION.INTERCEPT").toInteger(), 0);
    a.checkEqual("27", ap("MISSION.TOW")      .toInteger(), 0);
    a.checkEqual("28", ap("MOVE.ETA")         .toInteger(), 5);
    a.checkEqual("29", ap("MOVE.FUEL")        .toInteger(), 273);
    a.checkEqual("30", ap("OWNER.REAL")       .toInteger(), PLAYER);
    a.checkEqual("31", ap("SPEED")            .toInteger(), 7);
    a.checkEqual("32", ap("WAYPOINT.DX")      .toInteger(), DX);
    a.checkEqual("33", ap("WAYPOINT.DY")      .toInteger(), DY);

    // Cargo
    a.checkEqual("51", ap("CARGO")("COLONISTS").toInteger(), 30);
    a.checkEqual("52", ap("CARGO")("D")        .toInteger(), 9);
    a.checkEqual("53", ap("CARGO")("M")        .toInteger(), 8);
    a.checkEqual("54", ap("CARGO")("MC")       .toInteger(), 2000);
    a.checkEqual("55", ap("CARGO")("N")        .toInteger(), 50);
    a.checkEqual("56", ap("CARGO")("SUPPLIES") .toInteger(), 7);
    a.checkEqual("57", ap("CARGO")("T")        .toInteger(), 10);

    // Functions
    a.checkEqual("71", ap("FUNC").getArraySize(), 1U);
    a.checkEqual("72", ap("FUNC")[0]("ID")     .toInteger(), BasicHullFunction::Cloak);
    a.checkEqual("73", ap("FUNC")[0]("PLAYERS").toInteger(), -1);
    a.checkEqual("74", ap("FUNC")[0]("LEVELS") .toInteger(), 2047);
    a.checkEqual("75", ap("FUNC")[0]("KIND")   .toInteger(), 0);

    // Transfer
    a.checkEqual("81", ap("TRANSFER")("N")        .toInteger(), 3);
    a.checkEqual("82", ap("TRANSFER")("T")        .toInteger(), 4);
    a.checkEqual("83", ap("TRANSFER")("D")        .toInteger(), 5);
    a.checkEqual("84", ap("TRANSFER")("M")        .toInteger(), 6);
    a.checkEqual("85", ap("TRANSFER")("COLONISTS").toInteger(), 7);
    a.checkEqual("86", ap("TRANSFER")("SUPPLIES") .toInteger(), 8);
    a.checkEqual("87", ap("TRANSFER")("ID")       .toInteger(), NEAR_SHIP_ID);

    // Unload
    a.checkEqual("91", ap("UNLOAD")("N")        .toInteger(), 20);
    a.checkEqual("92", ap("UNLOAD")("T")        .toInteger(), 21);
    a.checkEqual("93", ap("UNLOAD")("D")        .toInteger(), 22);
    a.checkEqual("94", ap("UNLOAD")("M")        .toInteger(), 23);
    a.checkEqual("95", ap("UNLOAD")("COLONISTS").toInteger(), 24);
    a.checkEqual("96", ap("UNLOAD")("SUPPLIES") .toInteger(), 25);
    a.checkEqual("97", ap("UNLOAD")("ID")       .toInteger(), PLANET_ID);
}

AFL_TEST("server.play.ShipPacker:error:empty", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    Session session(tx, fs);

    AFL_CHECK_THROWS(a, server::play::ShipPacker(session, 50).buildValue(), std::exception);
}

AFL_TEST("server.play.ShipPacker:error:no-ship", a)
{
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    Session session(tx, fs);


    // Empty objects
    session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr());
    session.setShipList(new game::spec::ShipList());
    session.setGame(new Game());

    AFL_CHECK_THROWS(a, server::play::ShipPacker(session, 50).buildValue(), std::exception);
}
