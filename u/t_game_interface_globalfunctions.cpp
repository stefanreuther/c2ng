/**
  *  \file u/t_game_interface_globalfunctions.cpp
  *  \brief Test for game::interface::GlobalFunctions
  */

#include "game/interface/globalfunctions.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
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
#include "afl/data/stringvalue.hpp"

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

/** Test IFAutoTask(). */
void
TestGameInterfaceGlobalFunctions::testAutoTask()
{
    Environment env;
    addGame(env);               // Required to access objects
    addRoot(env);               // Required to create ShipContext/PlanetContext, ...
    addShipList(env);           // ... without those, the verifyInteger(ID) tests will fail.
    env.session.getGame()->currentTurn().universe().planets().create(100);
    env.session.getGame()->currentTurn().universe().ships().create(200);

    // Ship task
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(200);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
        TS_ASSERT(ctx != 0);
        interpreter::test::ContextVerifier verif(*ctx, "ship task");
        verif.verifyInteger("ID", 200);
        verif.verifyString("TYPE", "ship");
    }

    // Planet task
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
        TS_ASSERT(ctx != 0);
        interpreter::test::ContextVerifier verif(*ctx, "planet task");
        verif.verifyInteger("ID", 100);
        verif.verifyString("TYPE", "planet");
    }

    // Base task
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));

        interpreter::Context* ctx = dynamic_cast<interpreter::Context*>(p.get());
        TS_ASSERT(ctx != 0);
        interpreter::test::ContextVerifier verif(*ctx, "base task");
        verif.verifyInteger("ID", 100);
        verif.verifyString("TYPE", "base");
    }

    // Null parameters
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        std::auto_ptr<afl::data::Value> p(game::interface::IFAutoTask(env.session, args));
        TS_ASSERT(p.get() == 0);
    }

    // Range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFAutoTask(env.session, args), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(3);
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFAutoTask(env.session, args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFAutoTask(env.session, args), interpreter::Error);
    }
}

/** Test IFCfg(). */
void
TestGameInterfaceGlobalFunctions::testCfg()
{
    Environment env;
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

    // Integer option
    {
        afl::data::Segment seg;
        seg.pushBackString("numShips");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewInteger("numShips", game::interface::IFCfg(env.session, args), 600);
    }

    // Boolean option
    {
        afl::data::Segment seg;
        seg.pushBackString("AllowAlternativeTowing");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("AllowAlternativeTowing", game::interface::IFCfg(env.session, args), true);
    }

    // Error case: index given for integer option
    {
        afl::data::Segment seg;
        seg.pushBackString("numShips");
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Standard option, no index given -- picks viewpoint player
    {
        afl::data::Segment seg;
        seg.pushBackString("PlayerRace");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewInteger("PlayerRace/1", game::interface::IFCfg(env.session, args), 3);
    }

    // Standard option, index given
    {
        afl::data::Segment seg;
        seg.pushBackString("PlayerRace");
        seg.pushBackInteger(7);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("PlayerRace/2", game::interface::IFCfg(env.session, args), 7);
    }

    // Standard option, boolean
    {
        afl::data::Segment seg;
        seg.pushBackString("AllowBuildFighters");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("AllowBuildFighters/1", game::interface::IFCfg(env.session, args), false);
    }

    // Error case: standard option, index out of range
    {
        afl::data::Segment seg;
        seg.pushBackString("PlayerRace");
        seg.pushBackInteger(700);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Standard option, second parameter is null
    {
        afl::data::Segment seg;
        seg.pushBackString("PlayerRace");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("PlayerRace null", game::interface::IFCfg(env.session, args));
    }

    // Alias, pointing to single
    {
        afl::data::Segment seg;
        seg.pushBackString("CPEnableRumor");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("CPEnableRumor", game::interface::IFCfg(env.session, args), false);
    }

    // Cost option, no index given
    {
        afl::data::Segment seg;
        seg.pushBackString("FreeFighterCost");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("FreeFighterCost", game::interface::IFCfg(env.session, args), "30M");
    }

    // Cost option, index given
    {
        afl::data::Segment seg;
        seg.pushBackString("FreeFighterCost");
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewString("FreeFighterCost", game::interface::IFCfg(env.session, args), "20T");
    }

    // Error case: cost option, index out of range
    {
        afl::data::Segment seg;
        seg.pushBackString("FreeFighterCost");
        seg.pushBackInteger(700);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Further arrays
    {
        afl::data::Segment seg;
        seg.pushBackString("EModBayRechargeRate");
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("EModBayRechargeRate", game::interface::IFCfg(env.session, args), 2);
    }

    // Error case: array, index out of range (1)
    {
        afl::data::Segment seg;
        seg.pushBackString("EModBayRechargeRate");
        seg.pushBackInteger(200);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Error case: array, index out of range (2)
    {
        afl::data::Segment seg;
        seg.pushBackString("WraparoundRectangle");
        seg.pushBackInteger(5);                       // in MAX_PLAYERS range, but not in array range
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Error case: array, no index given
    {
        afl::data::Segment seg;
        seg.pushBackString("EModBayRechargeRate");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // String
    {
        afl::data::Segment seg;
        seg.pushBackString("GameName");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("GameName", game::interface::IFCfg(env.session, args), "G!");
    }

    // Error case: index given for string option
    {
        afl::data::Segment seg;
        seg.pushBackString("GameName");
        seg.pushBackInteger(10);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // String array, returns entire array
    {
        afl::data::Segment seg;
        seg.pushBackString("Language");
        interpreter::Arguments args(seg, 0, 1);
        String_t result = verifyNewString("Language", game::interface::IFCfg(env.session, args));
        TS_ASSERT_EQUALS(result.substr(0, 12), "en,de,ua,es,");
    }

    // String array, index given
    {
        afl::data::Segment seg;
        seg.pushBackString("Language");
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewString("Language", game::interface::IFCfg(env.session, args), "en");
    }

    // String array, bad index
    {
        afl::data::Segment seg;
        seg.pushBackString("Language");
        seg.pushBackInteger(100);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Error case: bad name
    {
        afl::data::Segment seg;
        seg.pushBackString("WhySoSerious");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Null case
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("null 1", game::interface::IFCfg(env.session, args));
    }

    // Null case 2
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null 2", game::interface::IFCfg(env.session, args));
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }
}

/** Test IFCfg(), no Root.
    Function yields null. */
void
TestGameInterfaceGlobalFunctions::testCfgNoRoot()
{
    Environment env;
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);

    afl::data::Segment seg;
    seg.pushBackString("NumShips");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull("NumShips", game::interface::IFCfg(env.session, args));
}

/** Test IFCfg(), no Game.
    Accesses that would need viewpoint player fail. */
void
TestGameInterfaceGlobalFunctions::testCfgNoGame()
{
    Environment env;
    addRoot(env);

    // Integer array
    {
        afl::data::Segment seg;
        seg.pushBackString("PlayerRace");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }

    // Cost array
    {
        afl::data::Segment seg;
        seg.pushBackString("StarbaseCost");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFCfg(env.session, args), interpreter::Error);
    }
}

/** Test IFDistance. */
void
TestGameInterfaceGlobalFunctions::testDistance()
{
    Environment env;
    addGame(env);               // for objects (PlanetContext)
    addRoot(env);               // for PlanetContext and BeamContext (used as non-position object)
    addShipList(env);           // for BeamContext
    game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
    univ.planets().create(222)->setPosition(game::map::Point(1000, 1200));
    univ.planets().create(333)->setPosition(game::map::Point(1400, 1500));
    env.session.getShipList()->beams().create(3);

    // Planet/Planet
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
        interpreter::Arguments args(seg, 0, 2);
        verifyNewFloat("planet/planet", game::interface::IFDistance(env.session, args), 500, 0.01);
    }

    // Planet/XY
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1210);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewFloat("planet/XY", game::interface::IFDistance(env.session, args), 10, 0.01);
    }

    // XY/Planet
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1210);
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        interpreter::Arguments args(seg, 0, 3);
        verifyNewFloat("XY/planet", game::interface::IFDistance(env.session, args), 10, 0.01);
    }

    // XY/XY
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1210);
        seg.pushBackInteger(1030);
        seg.pushBackInteger(1170);
        interpreter::Arguments args(seg, 0, 4);
        verifyNewFloat("XY/XY", game::interface::IFDistance(env.session, args), 50, 0.01);
    }

    // Planet/Null
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("planet/null", game::interface::IFDistance(env.session, args));
    }

    // XY/X/Null
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1210);
        seg.pushBackInteger(1030);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 4);
        verifyNewNull("XY/X/Null", game::interface::IFDistance(env.session, args));
    }

    // Error: too few args
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }

    // Error: too many args
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
        seg.pushBackInteger(10);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }

    // Error: type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1210);
        seg.pushBackString("X");
        seg.pushBackInteger(1170);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
        seg.pushBackNew(game::interface::BeamContext::create(3, env.session));
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFDistance(env.session, args), interpreter::Error);
    }
}

/** Test IFDistance, no game/root set.
    Without a map configuration, we cannot compute distances. */
void
TestGameInterfaceGlobalFunctions::testDistanceNoGame()
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackInteger(1000);
    seg.pushBackInteger(1210);
    seg.pushBackInteger(1030);
    seg.pushBackInteger(1170);
    interpreter::Arguments args(seg, 0, 4);
    verifyNewNull("XY/XY", game::interface::IFDistance(env.session, args));
}

/** Test IFFormat. */
void
TestGameInterfaceGlobalFunctions::testFormat()
{
    Environment env;

    // Standard case
    {
        afl::data::Segment seg;
        seg.pushBackString("int %d, float %.2f, string %s");
        seg.pushBackInteger(42);
        seg.pushBackNew(interpreter::makeFloatValue(2.5));
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, 4);
        verifyNewString("standard", game::interface::IFFormat(env.session, args), "int 42, float 2.50, string x");
    }

    // Null format string
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(42);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null format", game::interface::IFFormat(env.session, args));
    }

    // Null parameter
    {
        afl::data::Segment seg;
        seg.pushBackString("hi %d");
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null param", game::interface::IFFormat(env.session, args));
    }

    // Error: too few args
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFFormat(env.session, args), interpreter::Error);
    }

    // Error: too many args
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 100);
        TS_ASSERT_THROWS(game::interface::IFFormat(env.session, args), interpreter::Error);
    }

    // Error: type error
    {
        afl::data::Segment seg;
        seg.pushBackString("hi %d");
        seg.pushBackNew(new interpreter::StructureType(*new interpreter::StructureTypeData()));
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFFormat(env.session, args), interpreter::Error);
    }
}

/** Test IFIsSpecialFCode. */
void
TestGameInterfaceGlobalFunctions::testIsSpecialFCode()
{
    Environment env;
    addShipList(env);
    env.session.getShipList()->friendlyCodes().addCode(game::spec::FriendlyCode("abc", ",", env.tx));

    // Normal
    {
        afl::data::Segment seg;
        seg.pushBackString("abc");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("abc", game::interface::IFIsSpecialFCode(env.session, args), true);
    }

    // Case-blind
    {
        afl::data::Segment seg;
        seg.pushBackString("ABC");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("ABC", game::interface::IFIsSpecialFCode(env.session, args), true);
    }

    // Mismatch
    {
        afl::data::Segment seg;
        seg.pushBackString("xyz");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("xyz", game::interface::IFIsSpecialFCode(env.session, args), false);
    }

    // Overly long
    {
        afl::data::Segment seg;
        seg.pushBackString("abcxyz");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("abcxyz", game::interface::IFIsSpecialFCode(env.session, args), true);
    }

    // Null
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("null", game::interface::IFIsSpecialFCode(env.session, args));
    }
}

/** Test IFIsSpecialFCode, null ship list. */
void
TestGameInterfaceGlobalFunctions::testIsSpecialFCodeNoShipList()
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("abc");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull("abc", game::interface::IFIsSpecialFCode(env.session, args));
}

/** Test IFObjectIsAt. */
void
TestGameInterfaceGlobalFunctions::testObjectIsAt()
{
    Environment env;
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

    // Planet, match
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewBoolean("planet match", game::interface::IFObjectIsAt(env.session, args), true);
    }

    // Planet, mismatch
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(222, env.session));
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1201);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewBoolean("planet mismatch", game::interface::IFObjectIsAt(env.session, args), false);
    }

    // Planet without position
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::PlanetContext::create(333, env.session));
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewNull("planet no pos", game::interface::IFObjectIsAt(env.session, args));
    }

    // Minefield, exact match
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackInteger(2000);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewBoolean("minefield exact", game::interface::IFObjectIsAt(env.session, args), true);
    }

    // Minefield, inexact match
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackInteger(2030);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewBoolean("minefield inexact", game::interface::IFObjectIsAt(env.session, args), true);
    }

    // Minefield, mismatch
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackInteger(2031);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewBoolean("minefield mismatch", game::interface::IFObjectIsAt(env.session, args), false);
    }

    // Null object
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(2031);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewNull("null object", game::interface::IFObjectIsAt(env.session, args));
    }

    // Null X coordinate
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackNew(0);
        seg.pushBackInteger(2031);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewNull("null X", game::interface::IFObjectIsAt(env.session, args));
    }

    // Null Y coordinate
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackInteger(2031);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewNull("null Y", game::interface::IFObjectIsAt(env.session, args));
    }

    // Type error, not an object
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        seg.pushBackInteger(2031);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
    }

    // Type error, not an object with position
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::BeamContext::create(3, env.session));
        seg.pushBackInteger(2031);
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
    }

    // Type error, coordinate is not a number
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackString("X");
        seg.pushBackInteger(2100);
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
    }

    // Arity error, too few
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
    }

    // Arity error, too many
    {
        afl::data::Segment seg;
        seg.pushBackNew(game::interface::MinefieldContext::create(444, env.session, false));
        seg.pushBackInteger(1000);
        seg.pushBackInteger(2000);
        seg.pushBackInteger(3000);
        interpreter::Arguments args(seg, 0, 4);
        TS_ASSERT_THROWS(game::interface::IFObjectIsAt(env.session, args), interpreter::Error);
    }
}

/** Test IFPlanetAt(). */
void
TestGameInterfaceGlobalFunctions::testPlanetAt()
{
    Environment env;
    addGame(env);               // for objects
    addRoot(env);               // for config
    game::map::Universe& univ = env.session.getGame()->currentTurn().universe();
    univ.planets().create(222)->setPosition(game::map::Point(1000, 1200));
    univ.planets().get(222)->internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(), 10, env.tx, env.session.log());

    // Exact match
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("exact match", game::interface::IFPlanetAt(env.session, args), 222);
    }

    // Exact match, explicit false
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewInteger("exact match explicit", game::interface::IFPlanetAt(env.session, args), 222);
    }

    // Inexact match
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1202);
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewInteger("inexact match explicit", game::interface::IFPlanetAt(env.session, args), 222);
    }

    // Mismatch
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1202);
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewInteger("mismatch", game::interface::IFPlanetAt(env.session, args), 0);
    }

    // Null
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null", game::interface::IFPlanetAt(env.session, args));
    }

    // Null
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 3);
        verifyNewNull("null 2", game::interface::IFPlanetAt(env.session, args));
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFPlanetAt(env.session, args), interpreter::Error);
    }

    // Arity
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFPlanetAt(env.session, args), interpreter::Error);
    }
}

/** Test IFPlanetAt(), empty session. */
void
TestGameInterfaceGlobalFunctions::testPlanetAtEmpty()
{
    // No root
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null no-root", game::interface::IFPlanetAt(env.session, args));
    }

    // No game
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackInteger(1000);
        seg.pushBackInteger(1200);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null no-game", game::interface::IFPlanetAt(env.session, args));
    }
}

/** Test IFPref(). */
void
TestGameInterfaceGlobalFunctions::testPref()
{
    Environment env;
    addRoot(env);

    UserConfiguration& config = env.session.getRoot()->userConfiguration();
    config[UserConfiguration::Sort_History].set(3);
    config[UserConfiguration::Display_ThousandsSep].set(true);
    config[UserConfiguration::Backup_Chart].set("/foo");

    // Integer option
    {
        afl::data::Segment seg;
        seg.pushBackString("sort.history");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewInteger("sort.history", game::interface::IFPref(env.session, args), 3);
    }

    // Boolean option
    {
        afl::data::Segment seg;
        seg.pushBackString("Display.ThousandsSep");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewBoolean("Display.ThousandsSep", game::interface::IFPref(env.session, args), true);
    }

    // Error case: index given for integer option
    {
        afl::data::Segment seg;
        seg.pushBackString("Display.ThousandsSep");
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFPref(env.session, args), interpreter::Error);
    }

    // String
    {
        afl::data::Segment seg;
        seg.pushBackString("Backup.Chart");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("Backup.Chart", game::interface::IFPref(env.session, args), "/foo");
    }

    // Error case: bad name
    {
        afl::data::Segment seg;
        seg.pushBackString("WhySoSerious");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFPref(env.session, args), interpreter::Error);
    }

    // Null case
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("null 1", game::interface::IFPref(env.session, args));
    }

    // Null case 2
    {
        afl::data::Segment seg;
        seg.pushBackString("Backup.Chart");
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null 2", game::interface::IFPref(env.session, args));
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFPref(env.session, args), interpreter::Error);
    }
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 3);
        TS_ASSERT_THROWS(game::interface::IFPref(env.session, args), interpreter::Error);
    }
}

/** Test IFPref(), no root. */
void
TestGameInterfaceGlobalFunctions::testPrefNoRoot()
{
    Environment env;

    afl::data::Segment seg;
    seg.pushBackString("sort.history");
    interpreter::Arguments args(seg, 0, 1);
    verifyNewNull("no root", game::interface::IFPref(env.session, args));
}

/** Test IFQuote(). */
void
TestGameInterfaceGlobalFunctions::testQuote()
{
    Environment env;

    // Number
    {
        afl::data::Segment seg;
        seg.pushBackInteger(42);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("42", game::interface::IFQuote(env.session, args), "42");
    }

    // String
    {
        afl::data::Segment seg;
        seg.pushBackString("x");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("x", game::interface::IFQuote(env.session, args), "\"x\"");
    }

    // Boolean
    {
        afl::data::Segment seg;
        seg.pushBackNew(interpreter::makeBooleanValue(true));
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("true", game::interface::IFQuote(env.session, args), "True");
    }

    // Empty
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("empty", game::interface::IFQuote(env.session, args), "Z(0)");
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFQuote(env.session, args), interpreter::Error);
    }
}

/** Test IFRandom(). */
void
TestGameInterfaceGlobalFunctions::testRandom()
{
    class TestCase {
     public:
        TestCase(afl::test::Assert a)
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

    // Single parameter
    {
        afl::data::Segment seg;
        seg.pushBackInteger(10);
        TestCase("(10)").run(seg, 0, 9);
    }

    // Two parameters
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(500);
        TestCase("(1,500)").run(seg, 1, 499);
    }

    // Two parameters, reverse order
    {
        afl::data::Segment seg;
        seg.pushBackInteger(500);
        seg.pushBackInteger(1);
        TestCase("(500,1)").run(seg, 2, 500);
    }

    // Empty interval
    {
        afl::data::Segment seg;
        seg.pushBackInteger(300);
        seg.pushBackInteger(300);
        TestCase("(300,300)").run(seg, 300, 300);
    }

    // Size-1 interval
    {
        afl::data::Segment seg;
        seg.pushBackInteger(300);
        seg.pushBackInteger(301);
        TestCase("(300,301)").run(seg, 300, 300);
    }

    // Error/abnormal cases
    Environment env;

    // - null error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("null", game::interface::IFRandom(env.session, args));
    }

    // - null error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("null2", game::interface::IFRandom(env.session, args));
    }

    // - type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(game::interface::IFRandom(env.session, args), interpreter::Error);
    }

    // - arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFRandom(env.session, args), interpreter::Error);
    }
}

/** Test IFRandomFCode(). */
void
TestGameInterfaceGlobalFunctions::testRandomFCode()
{
    // Normal case
    {
        Environment env;
        addRoot(env);
        addShipList(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        std::auto_ptr<afl::data::Value> p(game::interface::IFRandomFCode(env.session, args));
        afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(p.get());
        TS_ASSERT(sv != 0);
        TS_ASSERT_EQUALS(sv->getValue().size(), 3U);
    }

    // Missing root
    {
        Environment env;
        addShipList(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        verifyNewNull("no root", game::interface::IFRandomFCode(env.session, args));
    }

    // Missing ship list
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);

        verifyNewNull("no ship list", game::interface::IFRandomFCode(env.session, args));
    }
}

/** Test IFTranslate(). */
void
TestGameInterfaceGlobalFunctions::testTranslate()
{
    Environment env;

    // Normal
    {
        afl::data::Segment seg;
        seg.pushBackString("hi");
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("Translate normal", game::interface::IFTranslate(env.session, args), "hi");
    }

    // Null
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("Translate null", game::interface::IFTranslate(env.session, args));
    }
}

/** Test IFTruehull(). */
void
TestGameInterfaceGlobalFunctions::testTruehull()
{
    Environment env;
    addRoot(env);
    addGame(env);
    env.session.getGame()->setViewpointPlayer(3);
    addShipList(env);
    game::spec::HullAssignmentList& as = env.session.getShipList()->hullAssignments();
    as.add(3, 4, 20);
    as.add(4, 4, 30);
    as.add(5, 4, 10);

    // Player number given
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("(4,5)", game::interface::IFTruehull(env.session, args), 10);
    }

    // Player number not given
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewInteger("(4)", game::interface::IFTruehull(env.session, args), 20);
    }

    // Null case
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("(null)", game::interface::IFTruehull(env.session, args));
    }

    // Out of range player
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(15);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("(4,15)", game::interface::IFTruehull(env.session, args), 0);
    }

    // Out of range slot
    {
        afl::data::Segment seg;
        seg.pushBackInteger(14);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("(14,5)", game::interface::IFTruehull(env.session, args), 0);
    }

    // Null case 2
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("(4,null)", game::interface::IFTruehull(env.session, args));
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 2);
        TS_ASSERT_THROWS(game::interface::IFTruehull(env.session, args), interpreter::Error);
    }

    // Arity error
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(game::interface::IFTruehull(env.session, args), interpreter::Error);
    }
}

/** Test IFTruehull(), no game. */
void
TestGameInterfaceGlobalFunctions::testTruehullNoGame()
{
    Environment env;
    addRoot(env);
    addShipList(env);
    game::spec::HullAssignmentList& as = env.session.getShipList()->hullAssignments();
    as.add(3, 4, 20);
    as.add(4, 4, 30);
    as.add(5, 4, 10);

    // Player number given (same as testTruehull)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewInteger("(4,5)", game::interface::IFTruehull(env.session, args), 10);
    }

    // Player number not given (different from testTruehull)
    {
        afl::data::Segment seg;
        seg.pushBackInteger(4);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("(4)", game::interface::IFTruehull(env.session, args));
    }
}

/** Test IFTruehull(), no root. */
void
TestGameInterfaceGlobalFunctions::testTruehullNoRoot()
{
    // No root
    {
        Environment env;
        addGame(env);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("no root", game::interface::IFTruehull(env.session, args));
    }

    // No game
    {
        Environment env;
        addRoot(env);

        afl::data::Segment seg;
        seg.pushBackInteger(4);
        seg.pushBackInteger(5);
        interpreter::Arguments args(seg, 0, 2);
        verifyNewNull("no game", game::interface::IFTruehull(env.session, args));
    }
}

