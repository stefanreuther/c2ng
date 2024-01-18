/**
  *  \file test/game/interface/globalfunctionstest.cpp
  *  \brief Test for game::interface::GlobalFunctions
  */

#include "game/interface/globalfunctions.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/interface/beamcontext.hpp"
#include "game/interface/minefieldcontext.hpp"
#include "game/interface/planetcontext.hpp"
#include "game/map/minefield.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/structuretypedata.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/values.hpp"

using game::config::HostConfiguration;
using game::config::UserConfiguration;
using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewFloat;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    struct Environment {
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        game::Session session;

        Environment()
            : tx(), fs(), session(tx, fs)
            { }
    };

    void addRoot(Environment& env)
    {
        env.session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    }

    void addGame(Environment& env)
    {
        env.session.setGame(new game::Game());
    }

    void addShipList(Environment& env)
    {
        env.session.setShipList(new game::spec::ShipList());
    }
}

/*
 *  IFAutoTask
 */

namespace {
    void prepareAutoTask(Environment& env)
    {
        addGame(env);               // Required to access objects
        addRoot(env);               // Required to create ShipContext/PlanetContext, ...
        addShipList(env);           // ... without those, the verifyInteger(ID) tests will fail.
        env.session.getGame()->currentTurn().universe().planets().create(100);
        env.session.getGame()->currentTurn().universe().ships().create(200);
    }
}

// Ship task
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:ship-task", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackInteger(200);
    interpreter::Arguments args(seg, 0, 2);
    std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
    a.checkNonNull("ctx", ctx);
    interpreter::test::ContextVerifier verif(*ctx, a);
    verif.verifyInteger("ID", 200);
    verif.verifyString("TYPE", "ship");
}

// Planet task
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:planet-task", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    seg.pushBackInteger(2);
    seg.pushBackInteger(100);
    interpreter::Arguments args(seg, 0, 2);
    std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
    a.checkNonNull("ctx", ctx);
    interpreter::test::ContextVerifier verif(*ctx, a);
    verif.verifyInteger("ID", 100);
    verif.verifyString("TYPE", "planet");
}

// Base task
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:base-task", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackInteger(100);
    interpreter::Arguments args(seg, 0, 2);
    std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

    interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
    a.checkNonNull("ctx", ctx);
    interpreter::test::ContextVerifier verif(*ctx, a);
    verif.verifyInteger("ID", 100);
    verif.verifyString("TYPE", "base");
}

// Null parameters
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:null", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 2);
    std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));
    a.checkNull("", p.get());
}

// Range error
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:error:range", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(100);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFAutoTask(env.session, args), interpreter::Error);
}

// Type error
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:error:type", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    seg.pushBackInteger(3);
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFAutoTask(env.session, args), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.GlobalFunctions:IFAutoTask:error:arity", a)
{
    Environment env;
    prepareAutoTask(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFAutoTask(env.session, args), interpreter::Error);
}

/*
 *  IFCfg
 */

namespace {
    void prepareCfg(Environment& env)
    {
        addRoot(env);
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);

        HostConfiguration& config = env.session.getRoot()->hostConfiguration();
        config[HostConfiguration::NumShips].set(600);
        config[HostConfiguration::AllowAlternativeTowing].set(true);
        config[HostConfiguration::AllowAnonymousMessages].set(false);
        config[HostConfiguration::FreeFighterCost].set("t10, t20, 30M, 40S");  // deliberately whacky format to prove it goes through the parser
        config[HostConfiguration::EModBayRechargeRate].set("1,2,3,4");
        config[HostConfiguration::GameName].set("G!");
        config[HostConfiguration::Language].set("en,de,ua,es,kr,ru");
    }
}

// Integer option
AFL_TEST("game.interface.GlobalFunctions:IFCfg:int", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("numShips");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewInteger(a, game::interface::IFCfg(env.session, args), 600);
}

// Boolean option
AFL_TEST("game.interface.GlobalFunctions:IFCfg:bool", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("AllowAlternativeTowing");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFCfg(env.session, args), true);
}

// Error case: index given for integer option
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:index-given-for-int", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("numShips");
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Standard option, no index given -- picks viewpoint player
AFL_TEST("game.interface.GlobalFunctions:IFCfg:int-array:default", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("PlayerRace");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewInteger(a, game::interface::IFCfg(env.session, args), 3);
}

// Standard option, index given
AFL_TEST("game.interface.GlobalFunctions:IFCfg:int-array:indexed", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("PlayerRace");
    seg.pushBackInteger(7);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFCfg(env.session, args), 7);
}

// Standard option, boolean
AFL_TEST("game.interface.GlobalFunctions:IFCfg:bool-array:default", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("AllowBuildFighters");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFCfg(env.session, args), false);
}

// Error case: standard option, index out of range
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:range:int-array", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("PlayerRace");
    seg.pushBackInteger(700);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Standard option, second parameter is null
AFL_TEST("game.interface.GlobalFunctions:IFCfg:int-array:null", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("PlayerRace");
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFCfg(env.session, args));
}

// Alias, pointing to single
AFL_TEST("game.interface.GlobalFunctions:IFCfg:aliased-int", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("CPEnableRumor");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFCfg(env.session, args), false);
}

// Cost option, no index given
AFL_TEST("game.interface.GlobalFunctions:IFCfg:cost:default", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("FreeFighterCost");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFCfg(env.session, args), "30M");
}

// Cost option, index given
AFL_TEST("game.interface.GlobalFunctions:IFCfg:cost:indexed", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("FreeFighterCost");
    seg.pushBackInteger(2);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewString(a, game::interface::IFCfg(env.session, args), "20T");
}

// Error case: cost option, index out of range
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:range:cost", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("FreeFighterCost");
    seg.pushBackInteger(700);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Further arrays
AFL_TEST("game.interface.GlobalFunctions:IFCfg:exp-array:indexed", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("EModBayRechargeRate");
    seg.pushBackInteger(2);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFCfg(env.session, args), 2);
}

// Error case: array, index out of range (1)
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:range:exp-array", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("EModBayRechargeRate");
    seg.pushBackInteger(200);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Error case: array, index out of range (2)
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:range:short-array", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("WraparoundRectangle");
    seg.pushBackInteger(5);                       // in MAX_PLAYERS range, but not in array range
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Error case: array, no index given
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:missing-index:exp-array", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("EModBayRechargeRate");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// String
AFL_TEST("game.interface.GlobalFunctions:IFCfg:string", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("GameName");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFCfg(env.session, args), "G!");
}

// Error case: index given for string option
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:index-given-for-string", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("GameName");
    seg.pushBackInteger(10);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// String array, returns entire array
AFL_TEST("game.interface.GlobalFunctions:IFCfg:string-array:whole", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("Language");
    interpreter::Arguments args(seg, 0, 1);
    String_t result = verifyNewString("Language", game::interface::IFCfg(env.session, args));
    a.checkEqual("", result.substr(0, 12), "en,de,ua,es,");
}

// String array, index given
AFL_TEST("game.interface.GlobalFunctions:IFCfg:string-array:indexed", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("Language");
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewString(a("Language"), game::interface::IFCfg(env.session, args), "en");
}

// String array, bad index
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:range:string-array", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("Language");
    seg.pushBackInteger(100);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Error case: bad name
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:bad-name", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    seg.pushBackString("WhySoSerious");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Null case
AFL_TEST("game.interface.GlobalFunctions:IFCfg:null", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFCfg(env.session, args));
}

// Null case 2
AFL_TEST("game.interface.GlobalFunctions:IFCfg:null:2", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFCfg(env.session, args));
}

// Arity error
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:too-few-args", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:too-many-args", a)
{
    Environment env;
    prepareCfg(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

/** Test IFCfg(), no Root.
    Function yields null. */
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:no-root", a)
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);

    afl::data::Segment seg;
    seg.pushBackString("NumShips");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFCfg(env.session, args));
}

/** Test IFCfg(), no Game.
    Accesses that would need viewpoint player fail. */

// Integer array
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:no-game:int-array", a)
{
    Environment env;
    addRoot(env);
    afl::data::Segment seg;
    seg.pushBackString("PlayerRace");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

// Cost array
AFL_TEST("game.interface.GlobalFunctions:IFCfg:error:no-game:cost-array", a)
{
    Environment env;
    addRoot(env);
    afl::data::Segment seg;
    seg.pushBackString("StarbaseCost");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFCfg(env.session, args), interpreter::Error);
}

/*
 *  IFDistance
 */

namespace {
    void prepareDistance(Environment& env)
    {
        addGame(env);               // for objects (PlanetContext)
        addRoot(env);               // for PlanetContext and BeamContext (used as non-position object)
        addShipList(env);           // for BeamContext
        game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
        univ.planets().create(222)->setPosition(game::map::Point(1000, 1200));
        univ.planets().create(333)->setPosition(game::map::Point(1400, 1500));
        env.session.getShipList()->beams().create(3);
    }
}

// Planet/Planet
AFL_TEST("game.interface.GlobalFunctions:IFDistance:planet+planet", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
    interpreter::Arguments args(seg, 0, 2);
    verifyNewFloat(a, game::interface::IFDistance(env.session, args), 500, 0.01);
}

// Planet/XY
AFL_TEST("game.interface.GlobalFunctions:IFDistance:planet+xy", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewFloat(a, game::interface::IFDistance(env.session, args), 10, 0.01);
}

// XY/Planet
AFL_TEST("game.interface.GlobalFunctions:IFDistance:xy+planet", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    interpreter::Arguments args(seg, 0, 3);
    verifyNewFloat(a, game::interface::IFDistance(env.session, args), 10, 0.01);
}

// XY/XY
AFL_TEST("game.interface.GlobalFunctions:IFDistance:xy+xy", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackInteger(1030);
    seg.pushBackInteger(1170);
    interpreter::Arguments args(seg, 0, 4);
    verifyNewFloat(a, game::interface::IFDistance(env.session, args), 50, 0.01);
}

// Planet/Null
AFL_TEST("game.interface.GlobalFunctions:IFDistance:planet+null", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFDistance(env.session, args));
}

// XY/X/Null
AFL_TEST("game.interface.GlobalFunctions:IFDistance:xy+x+null", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackInteger(1030);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 4);
    verifyNewNull(a, game::interface::IFDistance(env.session, args));
}

// Error: too few args
AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:too-few-args:planet", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:too-few-args:planet+x", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:too-few-args:xy", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

// Error: too many args
AFL_TEST("game.interface.GlobalFunctions:IFDistance:too-many-args", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
    seg.pushBackInteger(10);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

// Error: type error
AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:type:string", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackString("X");
    seg.pushBackInteger(1170);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:type:object", a)
{
    Environment env;
    prepareDistance(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
    seg.pushBackNew(game::interface::BeamContext::create(3, env.session));
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFDistance(env.session, args), interpreter::Error);
}

/** Test IFDistance, no game/root set.
    Without a map configuration, we cannot compute distances. */
AFL_TEST("game.interface.GlobalFunctions:IFDistance:error:no-game", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackInteger(1030);
    seg.pushBackInteger(1170);
    interpreter::Arguments args(seg, 0, 4);
    verifyNewNull(a, game::interface::IFDistance(env.session, args));
}

/*
 *  IFFormat
 */

// Standard case
AFL_TEST("game.interface.GlobalFunctions:IFFormat:normal", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("int %d, float %.2f, string %s");
    seg.pushBackInteger(42);
    seg.pushBackNew(interpreter::makeFloatValue(2.5));
    seg.pushBackString("x");
    interpreter::Arguments args(seg, 0, 4);
    verifyNewString(a, game::interface::IFFormat(env.session, args), "int 42, float 2.50, string x");
}

// Null format string
AFL_TEST("game.interface.GlobalFunctions:IFFormat:null-format", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(42);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFFormat(env.session, args));
}

// Null parameter
AFL_TEST("game.interface.GlobalFunctions:IFFormat:null-param", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("hi %d");
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFFormat(env.session, args));
}

// Error: too few args
AFL_TEST("game.interface.GlobalFunctions:IFFormat:error:too-few-args", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFFormat(env.session, args), interpreter::Error);
}

// Error: too many args
AFL_TEST("game.interface.GlobalFunctions:IFFormat:error:too-many-args", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 100);
    AFL_CHECK_THROWS(a, game::interface::IFFormat(env.session, args), interpreter::Error);
}

// Error: type error
AFL_TEST("game.interface.GlobalFunctions:IFFormat:error:type", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("hi %d");
    seg.pushBackNew(new interpreter::StructureType(*new interpreter::StructureTypeData()));
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFFormat(env.session, args), interpreter::Error);
}

/*
 *  IFIsSpecialFCode
 */

namespace {
    void prepareIsSpecialFCode(Environment& env)
    {
        addShipList(env);
        env.session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",", env.tx));
    }
}

// Normal
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:normal", a)
{
    Environment env;
    prepareIsSpecialFCode(env);
    afl::data::Segment seg;
    seg.pushBackString("abc");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFIsSpecialFCode(env.session, args), true);
}

// Case-blind
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:normal:case-blind", a)
{
    Environment env;
    prepareIsSpecialFCode(env);
    afl::data::Segment seg;
    seg.pushBackString("ABC");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFIsSpecialFCode(env.session, args), true);
}

// Mismatch
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:normal:mismatch", a)
{
    Environment env;
    prepareIsSpecialFCode(env);
    afl::data::Segment seg;
    seg.pushBackString("xyz");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFIsSpecialFCode(env.session, args), false);
}

// Overly long
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:normal:overlong", a)
{
    Environment env;
    prepareIsSpecialFCode(env);
    afl::data::Segment seg;
    seg.pushBackString("abcxyz");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFIsSpecialFCode(env.session, args), true);
}

// Null
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:null", a)
{
    Environment env;
    prepareIsSpecialFCode(env);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFIsSpecialFCode(env.session, args));
}

/** Test IFIsSpecialFCode, null ship list. */
AFL_TEST("game.interface.GlobalFunctions:IFIsSpecialFCode:error:no-shiplist", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("abc");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a("abc"), game::interface::IFIsSpecialFCode(env.session, args));
}

/*
 *  IFObjectIsAt
 */

namespace {
    void prepareObjectIsAt(Environment& env)
    {
        addGame(env);               // for objects
        addRoot(env);               // for objects and BeamContext (used as non-position object)
        addShipList(env);           // for BeamContext
        game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
        univ.planets().create(222)->setPosition(game::map::Point(1000, 1200));
        univ.planets().create(333);
        env.session.getShipList()->beams().create(3);

        game::map::Minefield& mf = *univ.minefields().create(444);
        mf.addReport(game::map::Point(2000, 2100), 1, game::map::Minefield::IsMine, game::map::Minefield::RadiusKnown, 30, 10, game::map::Minefield::MinefieldScanned);
        mf.internalCheck(10, env.session.getRoot()->hostVersion(), env.session.getRoot()->hostConfiguration());
    }
}

// Planet, match
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:planet:match", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewBoolean(a, game::interface::IFObjectIsAt(env.session, args), true);
}

// Planet, mismatch
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:planet:mismatch", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1201);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewBoolean(a, game::interface::IFObjectIsAt(env.session, args), false);
}

// Planet without position
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:planet:no-position", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewNull(a, game::interface::IFObjectIsAt(env.session, args));
}

// Minefield, exact match
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:minefield:exact-match", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackInteger(2000);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewBoolean(a, game::interface::IFObjectIsAt(env.session, args), true);
}

// Minefield, inexact match
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:minefield:inexact-match", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackInteger(2030);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewBoolean(a, game::interface::IFObjectIsAt(env.session, args), true);
}

// Minefield, mismatch
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:minefield:mismatch", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackInteger(2031);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewBoolean(a, game::interface::IFObjectIsAt(env.session, args), false);
}

// Null object
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:null:object", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(2031);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewNull(a, game::interface::IFObjectIsAt(env.session, args));
}

// Null X coordinate
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:null:x", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackNew(0);
    seg.pushBackInteger(2031);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewNull(a, game::interface::IFObjectIsAt(env.session, args));
}

// Null Y coordinate
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:null:y", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackInteger(2031);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewNull(a, game::interface::IFObjectIsAt(env.session, args));
}

// Type error, not an object
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:error:not-an-object", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackString("X");
    seg.pushBackInteger(2031);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
}

// Type error, not an object with position
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:error:not-a-mapobject", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::BeamContext::create(3, env.session));
    seg.pushBackInteger(2031);
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
}

// Type error, coordinate is not a number
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:error:bad-coordinate", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackString("X");
    seg.pushBackInteger(2100);
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
}

// Arity error, too few
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:error:too-few-args", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
}

// Arity error, too many
AFL_TEST("game.interface.GlobalFunctions:IFObjectIsAt:error:too-many-args", a)
{
    Environment env;
    prepareObjectIsAt(env);
    afl::data::Segment seg;
    seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
    seg.pushBackInteger(1000);
    seg.pushBackInteger(2000);
    seg.pushBackInteger(3000);
    interpreter::Arguments args(seg, 0, 4);
    AFL_CHECK_THROWS(a, game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
}

/*
 *  IFPlanetAt
 */

namespace {
    void preparePlanetAt(Environment& env)
    {
        addGame(env);               // for objects
        addRoot(env);               // for config
        game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
        univ.planets().create(222)->setPosition(game::map::Point(1000, 1200));
        univ.planets().get(222)->internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(), 10, env.tx, env.session.log());
    }
}

// Exact match
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:exact-match", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFPlanetAt(env.session, args), 222);
}

// Exact match, explicit false
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:exact-match:off", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewInteger(a, game::interface::IFPlanetAt(env.session, args), 222);
}

// Inexact match
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:inexact-match:on", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1202);
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewInteger(a, game::interface::IFPlanetAt(env.session, args), 222);
}

// Mismatch
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:mismatch", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1202);
    seg.pushBackInteger(0);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewInteger(a, game::interface::IFPlanetAt(env.session, args), 0);
}

// Null
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:null:y", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFPlanetAt(env.session, args));
}

// Null
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:null:flag", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 3);
    verifyNewNull(a, game::interface::IFPlanetAt(env.session, args));
}

// Type error
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:error:type", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFPlanetAt(env.session, args), interpreter::Error);
}

// Arity
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:error:arity", a)
{
    Environment env;
    preparePlanetAt(env);
    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFPlanetAt(env.session, args), interpreter::Error);
}

// No root
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:error:no-root", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFPlanetAt(env.session, args));
}

// No game
AFL_TEST("game.interface.GlobalFunctions:IFPlanetAt:error:no-game", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1200);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFPlanetAt(env.session, args));
}

/*
 *  IFPref
 */

namespace {
    void preparePref(Environment& env)
    {
        addRoot(env);

        UserConfiguration& config = env.session.getRoot()->userConfiguration();
        config[UserConfiguration::Sort_History].set(3);
        config[UserConfiguration::Display_ThousandsSep].set(true);
        config[UserConfiguration::Backup_Chart].set("/foo");
    }
}

// Integer option
AFL_TEST("game.interface.GlobalFunctions:IFPref:int", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("sort.history");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewInteger(a, game::interface::IFPref(env.session, args), 3);
}

// Boolean option
AFL_TEST("game.interface.GlobalFunctions:IFPref:bool", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("Display.ThousandsSep");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewBoolean(a, game::interface::IFPref(env.session, args), true);
}

// Error case: index given for integer option
AFL_TEST("game.interface.GlobalFunctions:IFPref:error:index-given-for-int", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("Display.ThousandsSep");
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFPref(env.session, args), interpreter::Error);
}

// String
AFL_TEST("game.interface.GlobalFunctions:IFPref:str", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("Backup.Chart");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFPref(env.session, args), "/foo");
}

// Error case: bad name
AFL_TEST("game.interface.GlobalFunctions:IFPref:error:bad-name", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("WhySoSerious");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFPref(env.session, args), interpreter::Error);
}

// Null case
AFL_TEST("game.interface.GlobalFunctions:IFPref:null", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFPref(env.session, args));
}

// Null case 2
AFL_TEST("game.interface.GlobalFunctions:IFPref:null:index", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    seg.pushBackString("Backup.Chart");
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFPref(env.session, args));
}

// Arity error
AFL_TEST("game.interface.GlobalFunctions:IFPref:error:too-few-args", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFPref(env.session, args), interpreter::Error);
}

AFL_TEST("game.interface.GlobalFunctions:IFPref:error:too-many-args", a)
{
    Environment env;
    preparePref(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 3);
    AFL_CHECK_THROWS(a, game::interface::IFPref(env.session, args), interpreter::Error);
}

/** Test IFPref(), no root. */
AFL_TEST("game.interface.GlobalFunctions:IFPref:error:no-root", a)
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("sort.history");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFPref(env.session, args));
}


/*
 *  IFQuote
 */

// Number
AFL_TEST("game.interface.GlobalFunctions:IFQuote:num", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackInteger(42);
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFQuote(env.session, args), "42");
}

// String
AFL_TEST("game.interface.GlobalFunctions:IFQuote:str", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("x");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFQuote(env.session, args), "\"x\"");
}

// Boolean
AFL_TEST("game.interface.GlobalFunctions:IFQuote:bool", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackNew(interpreter::makeBooleanValue(true));
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFQuote(env.session, args), "True");
}

// Empty
AFL_TEST("game.interface.GlobalFunctions:IFQuote:null", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFQuote(env.session, args), "Z(0)");
}

// Arity error
AFL_TEST("game.interface.GlobalFunctions:IFQuote:error:arity", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFQuote(env.session, args), interpreter::Error);
}

/*
 *  IFRandom
 */

namespace {
    class RandomTestCase {
     public:
        RandomTestCase(afl::test::Assert a)
            : m_assert(a)
            { }
        void run(afl::data::Segment& seg, int min, int max)
            {
                Environment env;
                for (int i = 0; i < 1000; ++i) {
                    interpreter::Arguments args(seg, 0, seg.size());
                    std::auto_ptr<afl::data::Value> p(game::interface::IFRandom(env.session, args));

                    afl::data::IntegerValue* iv = dynamic_cast<afl::data::IntegerValue*>(p.get());
                    m_assert.check("expect int", iv != 0);
                    m_assert.check("expect min", iv->getValue() >= min);
                    m_assert.check("expect max", iv->getValue() <= max);
                }
            }
     private:
        afl::test::Assert m_assert;
    };
}

// Single parameter
AFL_TEST("game.interface.GlobalFunctions:IFRandom:one-arg", a)
{
    afl::data::Segment seg;
    seg.pushBackInteger(10);
    RandomTestCase(a).run(seg, 0, 9);
}

// Two parameters
AFL_TEST("game.interface.GlobalFunctions:IFRandom:two-args", a)
{
    afl::data::Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackInteger(500);
    RandomTestCase(a).run(seg, 1, 499);
}

// Two parameters, reverse order
AFL_TEST("game.interface.GlobalFunctions:IFRandom:reverse-order", a)
{
    afl::data::Segment seg;
    seg.pushBackInteger(500);
    seg.pushBackInteger(1);
    RandomTestCase(a).run(seg, 2, 500);
}

// Empty interval
AFL_TEST("game.interface.GlobalFunctions:IFRandom:empty-interval", a)
{
    afl::data::Segment seg;
    seg.pushBackInteger(300);
    seg.pushBackInteger(300);
    RandomTestCase(a).run(seg, 300, 300);
}

// Size-1 interval
AFL_TEST("game.interface.GlobalFunctions:IFRandom:unit-interval", a)
{
    afl::data::Segment seg;
    seg.pushBackInteger(300);
    seg.pushBackInteger(301);
    RandomTestCase(a).run(seg, 300, 300);
}

// Error/abnormal cases
// - null error
AFL_TEST("game.interface.GlobalFunctions:IFRandom:null", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFRandom(env.session, args));
}

// - null error
AFL_TEST("game.interface.GlobalFunctions:IFRandom:null:2", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackInteger(1);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFRandom(env.session, args));
}

// - type error
AFL_TEST("game.interface.GlobalFunctions:IFRandom:error:type", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 1);
    AFL_CHECK_THROWS(a, game::interface::IFRandom(env.session, args), interpreter::Error);
}

// - arity error
AFL_TEST("game.interface.GlobalFunctions:IFRandom:error:arity", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFRandom(env.session, args), interpreter::Error);
}


/*
 *  IFRandomFCode
 */

// Normal case
AFL_TEST("game.interface.GlobalFunctions:IFRandomFCode:normal", a)
{
    Environment env;
    addRoot(env);
    addShipList(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);

    std::auto_ptr<afl::data::Value> p(game::interface::IFRandomFCode(env.session, args));
    afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(p.get());
    a.checkNonNull("01. type", sv);
    a.checkEqual("02. size", sv->getValue().size(), 3U);
}

// Missing root
AFL_TEST("game.interface.GlobalFunctions:IFRandomFCode:error:no-root", a)
{
    Environment env;
    addShipList(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);

    verifyNewNull(a, game::interface::IFRandomFCode(env.session, args));
}

// Missing ship list
AFL_TEST("game.interface.GlobalFunctions:IFRandomFCode:error:no-shiplist", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);

    verifyNewNull(a, game::interface::IFRandomFCode(env.session, args));
}

/*
 *  IFTranslate
 */

// Normal
AFL_TEST("game.interface.GlobalFunctions:IFTranslate:normal", a)
{
    Environment env;
    afl::data::Segment seg;
    seg.pushBackString("hi");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewString(a, game::interface::IFTranslate(env.session, args), "hi");
}

// Null
AFL_TEST("game.interface.GlobalFunctions:IFTranslate:null", a)
{
    Environment env;
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFTranslate(env.session, args));
}

/*
 *  IFTruehull
 */

namespace {
    void prepareTruehull(Environment& env)
    {
        addRoot(env);
        addGame(env);
        env.session.getGame()->setViewpointPlayer(3);
        addShipList(env);
        game::spec::HullAssignmentList& as = env.session.getShipList()->hullAssignments();
        as.add(3, 4, 20);
        as.add(4, 4, 30);
        as.add(5, 4, 10);
    }
}

// Player number given
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:player-given", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFTruehull(env.session, args), 10);
}

// Player number not given
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:player-not-given", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    interpreter::Arguments args(seg, 0, 1);
    verifyNewInteger(a, game::interface::IFTruehull(env.session, args), 20);
}

// Null case
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:null-index", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFTruehull(env.session, args));
}

// Out of range player
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:player-out-of-range", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(15);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFTruehull(env.session, args), 0);
}

// Out of range slot
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:slot-out-of-range", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(14);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFTruehull(env.session, args), 0);
}

// Null case 2
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:null-player", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackNew(0);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFTruehull(env.session, args));
}

// Type error
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:error:type", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackString("X");
    interpreter::Arguments args(seg, 0, 2);
    AFL_CHECK_THROWS(a, game::interface::IFTruehull(env.session, args), interpreter::Error);
}

// Arity error
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:error:arity", a)
{
    Environment env;
    prepareTruehull(env);
    afl::data::Segment seg;
    interpreter::Arguments args(seg, 0, 0);
    AFL_CHECK_THROWS(a, game::interface::IFTruehull(env.session, args), interpreter::Error);
}

/** Test IFTruehull(), no game. */
namespace {
    void prepareTruehullNoGame(Environment& env)
    {
        addRoot(env);
        addShipList(env);
        game::spec::HullAssignmentList& as = env.session.getShipList()->hullAssignments();
        as.add(3, 4, 20);
        as.add(4, 4, 30);
        as.add(5, 4, 10);
    }
}

// Player number given (same as testTruehull)
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:player-given-no-game", a)
{
    Environment env;
    prepareTruehullNoGame(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewInteger(a, game::interface::IFTruehull(env.session, args), 10);
}

// Player number not given (different from testTruehull)
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:no-game", a)
{
    Environment env;
    prepareTruehullNoGame(env);
    afl::data::Segment seg;
    seg.pushBackInteger(4);
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull(a, game::interface::IFTruehull(env.session, args));
}

/** Test IFTruehull(), no root. */
// No root
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:no-root", a)
{
    Environment env;
    addGame(env);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFTruehull(env.session, args));
}

// No game
AFL_TEST("game.interface.GlobalFunctions:IFTruehull:no-game-2", a)
{
    Environment env;
    addRoot(env);

    afl::data::Segment seg;
    seg.pushBackInteger(4);
    seg.pushBackInteger(5);
    interpreter::Arguments args(seg, 0, 2);
    verifyNewNull(a, game::interface::IFTruehull(env.session, args));
}
