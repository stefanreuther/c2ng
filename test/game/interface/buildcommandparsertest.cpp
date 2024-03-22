/**
  *  \file test/game/interface/buildcommandparsertest.cpp
  *  \brief Test for game::interface::BuildCommandParser
  */

#include "game/interface/buildcommandparser.hpp"

#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "game/hostversion.hpp"
#include "game/map/basedata.hpp"
#include "game/map/configuration.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "interpreter/process.hpp"
#include "interpreter/taskeditor.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using afl::base::Ref;
using game::Element;
using game::HostVersion;
using game::Root;
using game::interface::BuildCommandParser;
using game::map::Planet;
using game::spec::ShipList;
using interpreter::BCORef_t;
using interpreter::BytecodeObject;
using interpreter::Opcode;
using interpreter::Process;
using interpreter::TaskEditor;
using interpreter::World;

namespace {
    const int PLAYER = 1;

    struct Environment {
        Planet planet;
        ShipList shipList;
        Ref<Root> root;
        afl::string::NullTranslator tx;

        Environment()
            : planet(42),
              shipList(),
              root(game::test::makeRoot(HostVersion(HostVersion::PHost, MKVERSION(4,0,0)))),
              tx()
            { }
    };

    void prepare(Environment& env)
    {
        // Shiplist
        game::test::initPListBeams(env.shipList);
        game::test::initPListTorpedoes(env.shipList);
        game::test::addTranswarp(env.shipList);
        game::test::addOutrider(env.shipList);
        env.shipList.hullAssignments().add(PLAYER, 3, game::test::OUTRIDER_HULL_ID);

        // Planet
        env.planet.setPosition(game::map::Point(1000, 1000));

        game::map::PlanetData pd;
        pd.owner = PLAYER;
        pd.minedTritanium = 50;
        pd.minedDuranium = 60;
        pd.minedMolybdenum = 70;
        pd.money = 100;
        pd.supplies = 10;
        pd.colonistClans = 1000;
        env.planet.addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER));

        game::map::Configuration config;
        afl::sys::Log log;
        env.planet.internalCheck(config, game::PlayerSet_t(PLAYER), 10, env.tx, log);
        env.planet.setPlayability(Planet::Playable);
    }

    void prepareBase(Environment& env)
    {
        prepare(env);

        game::map::BaseData bd;
        bd.engineStorage.set(20, 0);
        bd.hullStorage.set(20, 0);
        bd.beamStorage.set(20, 0);
        bd.launcherStorage.set(20, 0);
        bd.torpedoStorage.set(20, 0);
        env.planet.addCurrentBaseData(bd, game::PlayerSet_t(PLAYER));

        game::map::Configuration config;
        afl::sys::Log log;
        env.planet.internalCheck(config, game::PlayerSet_t(PLAYER), 10, env.tx, log);
        env.planet.setPlayability(Planet::Playable);
    }
}

/* Test initialisation. */
AFL_TEST("game.interface.BuildCommandParser:init", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);

    a.checkNull("getResult", testee.getResult().get());
}

/*
 *  BuildShip
 */

/* Success case */
AFL_TEST("game.interface.BuildCommandParser:BuildShip", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildShip 1, 9, 3, 1");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Ship);

    a.checkEqual("11. info", result->info.size(), 3U);
    a.checkEqual("12. info", result->info[0], "OUTRIDER CLASS SCOUT");
    a.checkEqual("13. info", result->info[1], "Transwarp Drive");
    a.checkEqual("14. info", result->info[2], "Desintegrator");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "46T 37D 42M 5460$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "5350S");
}

/* Error: action fails. Errors are not supposed to be fatal. */
AFL_TEST("game.interface.BuildCommandParser:BuildShip:error:action", a)
{
    Environment env;
    prepareBase(env);
    env.planet.setPlayability(Planet::ReadOnly);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildShip 1, 9, 3, 1");

    a.checkNull("getResult", testee.getResult().get());
}

/* Error: syntax. Errors are not supposed to be fatal. */
AFL_TEST("game.interface.BuildCommandParser:BuildShip:error:syntax", a)
{
    Environment env;
    prepareBase(env);
    env.planet.setPlayability(Planet::ReadOnly);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildShip 'x'");

    a.checkNull("getResult", testee.getResult().get());
}

/*
 *  BuildBase
 */

/* Success case */
AFL_TEST("game.interface.BuildCommandParser:BuildBase", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBaseWait");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 1U);
    a.checkEqual("12. info", result->info[0], "Starbase");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "402T 120D 340M 900$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "352T 60D 270M 790S");
}

/* Command not executed because of parameter '0'. */
AFL_TEST("game.interface.BuildCommandParser:BuildBase:skip", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBase 0");

    a.checkNull("getResult", testee.getResult().get());
}

/* Error in action (base already present) */
AFL_TEST("game.interface.BuildCommandParser:BuildBase:error:action", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBaseWait");

    a.checkNull("getResult", testee.getResult().get());
}

/* Syntax error */
AFL_TEST("game.interface.BuildCommandParser:BuildBase:error:syntax", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBaseWait 1, 2, 3");

    a.checkNull("getResult", testee.getResult().get());
}

/*
 *  BuildDefense
 */

/* Success case */
AFL_TEST("game.interface.BuildCommandParser:BuildDefense", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildDefenseWait 100");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 3U);
    a.checkEqual("12. info", result->info[0], "Defense Posts");
    a.checkEqual("13. info", result->info[1], "To build: 100");
    a.checkEqual("14. info", result->info[2], "Only 81 more supported!");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "81S 810$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "781S");
}

/* Partial build using setLimit */
AFL_TEST("game.interface.BuildCommandParser:BuildDefense:partial", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.setLimit(10);
    testee.predictStatement("BuildDefenseWait 100");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Defense Posts");
    a.checkEqual("13. info", result->info[1], "To build: 10/100");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "10S 100$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "");
}

/* Error in action (not playable) */
AFL_TEST("game.interface.BuildCommandParser:BuildDefense:error:action", a)
{
    Environment env;
    prepare(env);
    env.planet.setPlayability(Planet::ReadOnly);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildDefenseWait 10");

    a.checkNull("getResult", testee.getResult().get());
}

/* Syntax error */
AFL_TEST("game.interface.BuildCommandParser:BuildDefense:error:syntax", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildDefenseWait 'x'");

    a.checkNull("getResult", testee.getResult().get());
}

/*
 *  Other structures (happy case only; bad cases are same as for BuildDefense)
 */

/* BuildFactories */
AFL_TEST("game.interface.BuildCommandParser:BuildFactories", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildFactoriesWait 20");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Factories");
    a.checkEqual("13. info", result->info[1], "To build: 20");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "20S 60$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "10S");
}

/* BuildBaseDefense */
AFL_TEST("game.interface.BuildCommandParser:BuildBaseDefense", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBaseDefenseWait 15");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Starbase Defense");
    a.checkEqual("13. info", result->info[1], "To build: 15");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "15D 150$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "40S");
}

/* BuildMines */
AFL_TEST("game.interface.BuildCommandParser:BuildMines", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildMines 40");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Mineral Mines");
    a.checkEqual("13. info", result->info[1], "To build: 40");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "40S 160$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "90S");
}

/*
 *  BuildEngines
 */

/* Success case */
AFL_TEST("game.interface.BuildCommandParser:BuildEngines", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildEngines 9, 3");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Transwarp Drive");
    a.checkEqual("13. info", result->info[1], "To build: 3");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "9T 48D 105M 5400$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "35M 5290S");
}

/* Partial build using setLimit */
AFL_TEST("game.interface.BuildCommandParser:BuildEngines:partial", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.setLimit(1);
    testee.predictStatement("BuildEngines 9, 3");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Transwarp Drive");
    a.checkEqual("13. info", result->info[1], "To build: 1/3");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "3T 16D 35M 4800$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "4690S");
}

/* Error in action (no base) */
AFL_TEST("game.interface.BuildCommandParser:BuildEngines:error:action", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildEngines 9, 3");

    a.checkNull("getResult", testee.getResult().get());
}

/* Syntax error */
AFL_TEST("game.interface.BuildCommandParser:BuildEngines:error:syntax", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildEngines 7");

    a.checkNull("getResult", testee.getResult().get());
}

/* Range error */
AFL_TEST("game.interface.BuildCommandParser:BuildEngines:error:range", a)
{
    Environment env;
    prepare(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildEngines 77, 3");

    a.checkNull("getResult", testee.getResult().get());
}

/*
 *  Other components
 */

/* BuildHulls */
AFL_TEST("game.interface.BuildCommandParser:BuildHulls", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildHulls 1, 5");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "OUTRIDER CLASS SCOUT");
    a.checkEqual("13. info", result->info[1], "To build: 5");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "200T 100D 25M 250$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "150T 40D 140S");
}

/* BuildHulls, error case: hull not mapped */
AFL_TEST("game.interface.BuildCommandParser:BuildHulls:error:range", a)
{
    Environment env;
    prepareBase(env);
    env.shipList.hulls().create(77);      // hull exists but is not mapped
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildHulls 77, 5");

    a.checkNull("getResult", testee.getResult().get());
}

/* BuildBeams */
AFL_TEST("game.interface.BuildCommandParser:BuildBeams", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildBeams 2, 7");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Kill-O-Zap");
    a.checkEqual("13. info", result->info[1], "To build: 7");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "7T 14D 135$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "25S");
}

/* BuildLaunchers */
AFL_TEST("game.interface.BuildCommandParser:BuildLaunchers", a)
{
    Environment env;
    prepareBase(env);
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.predictStatement("BuildLaunchersWait 4, 3");

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("01. getResult", result.get());
    a.checkEqual("02. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("11. info", result->info.size(), 2U);
    a.checkEqual("12. info", result->info[0], "Initial Bomb");
    a.checkEqual("13. info", result->info[1], "To build: 3");

    a.checkEqual("21. cost",    result->cost.toCargoSpecString(), "15T 3D 6M 2280$");
    a.checkEqual("22. missing", result->missingAmount.toCargoSpecString(), "2170S");
}

/*
 *  Test loadLimit
 */

AFL_TEST("game.interface.BuildCommandParser:loadLimit", a)
{
    Environment env;
    prepare(env);

    // World, Process
    afl::sys::Log log;
    afl::io::NullFileSystem fs;
    World world(log, env.tx, fs);
    Process proc(world, "proc", 99);

    // Code inside process
    // - outer (auto task code)
    BCORef_t outerBCO = BytecodeObject::create(true);
    afl::data::StringValue sv("BuildFactoriesWait 20");
    outerBCO->addPushLiteral(&sv);
    outerBCO->addInstruction(Opcode::maPush, Opcode::sNamedVariable, outerBCO->addName("CC$AUTOEXEC"));
    outerBCO->addInstruction(Opcode::maIndirect, Opcode::miIMCall, 1);

    Process::Frame& outerFrame = proc.pushFrame(outerBCO, false);
    outerFrame.pc = 3;

    // - inner (implementation of BuildFactoriesWait)
    Process::Frame& innerFrame = proc.pushFrame(BytecodeObject::create(true), false);
    size_t innerAddr = innerFrame.localNames.add("BUILD.REMAINDER");
    innerFrame.localValues.setNew(innerAddr, interpreter::makeIntegerValue(5));

    // TaskEditor
    TaskEditor edit(proc);
    a.checkEqual("01. getNumInstructions", edit.getNumInstructions(), 1U);
    a.checkEqual("02. getPC",              edit.getNumInstructions(), 1U);
    a.checkEqual("03. isInSubroutineCall", edit.isInSubroutineCall(), true);
    a.checkEqual("04. code",               edit[0], "BuildFactoriesWait 20");

    // Verify predictor
    BuildCommandParser testee(env.planet, env.shipList, *env.root, env.tx);
    testee.loadLimit(edit, 0);
    testee.predictStatement(edit, 0);

    std::auto_ptr<BuildCommandParser::Result> result(testee.getResult());
    a.checkNonNull("11. getResult", result.get());
    a.checkEqual("12. type", result->type, BuildCommandParser::OrderType_Other);

    a.checkEqual("21. info", result->info.size(), 2U);
    a.checkEqual("22. info", result->info[0], "Factories");
    a.checkEqual("23. info", result->info[1], "To build: 5/20");

    a.checkEqual("31. cost",    result->cost.toCargoSpecString(), "5S 15$");
    a.checkEqual("32. missing", result->missingAmount.toCargoSpecString(), "");
}
