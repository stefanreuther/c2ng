/**
  *  \file u/t_game_interface_planetproperty.cpp
  *  \brief Test for game::interface::PlanetProperty
  */

#include "game/interface/planetproperty.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/map/planet.hpp"
#include "game/map/planetdata.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::Reference;
using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    const int TURN_NR = 10;

    void addShip(game::Game& g, int id, int x, int y, int owner)
    {
        const game::PlayerSet_t SET(g.getViewpointPlayer());
        game::map::Ship& sh = *g.currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(x, y), owner, 100, SET);
        if (owner == g.getViewpointPlayer()) {
            game::map::ShipData sd;
            sd.x = x;
            sd.y = y;
            sd.owner = owner;
            sh.addCurrentShipData(sd, SET);
        }
        sh.internalCheck(SET, TURN_NR);
    }
}

/** Test operation on a fully-populated planet. */
void
TestGameInterfacePlanetProperty::testIt()
{
    const int PLAYER = 5;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(PLAYER);
    for (int i = 0; i < 10; ++i) {
        g->currentTurn().inbox().addMessage("msg...", TURN_NR);
    }
    session.setGame(g);

    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr();
    session.setRoot(r);

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

    game::map::Planet& pl = *g->currentTurn().universe().planets().create(42);
    pl.setPosition(game::map::Point(1030, 2700));
    pl.addCurrentPlanetData(pd, game::PlayerSet_t(5));
    pl.setName("Earth 2");
    pl.setPlayability(game::map::Object::Playable);
    pl.messages().add(2);
    pl.messages().add(6);
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, tx, session.log());

    // Ships: 2 own, 3 enemy, and 2 elsewhere
    addShip(*g, 1, 1030, 2700, PLAYER);
    addShip(*g, 2, 1030, 2700, PLAYER+1);
    addShip(*g, 3, 1030, 2700, PLAYER);
    addShip(*g, 4, 1030, 2700, PLAYER+2);
    addShip(*g, 5, 1030, 2700, PLAYER+3);
    addShip(*g, 6, 1031, 2700, PLAYER);
    addShip(*g, 7, 1030, 2701, PLAYER);

    // Level
    game::UnitScoreDefinitionList::Definition levelDef;
    levelDef.name  = "Level";
    levelDef.id    = game::ScoreId_ExpLevel;
    levelDef.limit = -1;
    pl.unitScores().set(g->planetScores().add(levelDef), 3, TURN_NR);

    // Player definition
    game::Player& player = *r->playerList().create(5);
    player.setName(game::Player::LongName, "The Orion Pirates");
    player.setName(game::Player::ShortName, "The Pirates");
    player.setName(game::Player::AdjectiveName, "Orion");

    // Verify the scalars
    verifyNewBoolean("ippBaseBuildFlag",     getPlanetProperty(pl, game::interface::ippBaseBuildFlag,     session, *r, *g), true);
    verifyNewInteger("ippBaseDefenseSpeed",  getPlanetProperty(pl, game::interface::ippBaseDefenseSpeed,  session, *r, *g), 2);
    verifyNewInteger("ippBaseDefenseWanted", getPlanetProperty(pl, game::interface::ippBaseDefenseWanted, session, *r, *g), 20);
    verifyNewBoolean("ippBaseFlag",          getPlanetProperty(pl, game::interface::ippBaseFlag,          session, *r, *g), false);
    verifyNewString ("ippBaseStr",           getPlanetProperty(pl, game::interface::ippBaseStr,           session, *r, *g), "being built");
    verifyNewInteger("ippCashTime",          getPlanetProperty(pl, game::interface::ippCashTime,          session, *r, *g), 10);
    verifyNewInteger("ippColonistChange",    getPlanetProperty(pl, game::interface::ippColonistChange,    session, *r, *g), 6);
    verifyNewString ("ippColonistChangeStr", getPlanetProperty(pl, game::interface::ippColonistChangeStr, session, *r, *g), "They LOVE you.");
    verifyNewInteger("ippColonistHappy",     getPlanetProperty(pl, game::interface::ippColonistHappy,     session, *r, *g), 97);
    verifyNewString ("ippColonistHappyStr",  getPlanetProperty(pl, game::interface::ippColonistHappyStr,  session, *r, *g), "happy");
    verifyNewInteger("ippColonistSupported", getPlanetProperty(pl, game::interface::ippColonistSupported, session, *r, *g), 99556);
    verifyNewInteger("ippColonistTax",       getPlanetProperty(pl, game::interface::ippColonistTax,       session, *r, *g), 3);
    verifyNewInteger("ippColonistTaxIncome", getPlanetProperty(pl, game::interface::ippColonistTaxIncome, session, *r, *g), 4);
    verifyNewInteger("ippColonistTime",      getPlanetProperty(pl, game::interface::ippColonistTime,      session, *r, *g), 10);
    verifyNewInteger("ippColonists",         getPlanetProperty(pl, game::interface::ippColonists,         session, *r, *g), 1200);
    verifyNewInteger("ippDefense",           getPlanetProperty(pl, game::interface::ippDefense,           session, *r, *g), 15);
    verifyNewInteger("ippDefenseMax",        getPlanetProperty(pl, game::interface::ippDefenseMax,        session, *r, *g), 84);
    verifyNewInteger("ippDefenseSpeed",      getPlanetProperty(pl, game::interface::ippDefenseSpeed,      session, *r, *g), 3);
    verifyNewInteger("ippDefenseWanted",     getPlanetProperty(pl, game::interface::ippDefenseWanted,     session, *r, *g), 1000);
    verifyNewInteger("ippDensityD",          getPlanetProperty(pl, game::interface::ippDensityD,          session, *r, *g), 29);
    verifyNewInteger("ippDensityM",          getPlanetProperty(pl, game::interface::ippDensityM,          session, *r, *g), 7);
    verifyNewInteger("ippDensityN",          getPlanetProperty(pl, game::interface::ippDensityN,          session, *r, *g), 14);
    verifyNewInteger("ippDensityT",          getPlanetProperty(pl, game::interface::ippDensityT,          session, *r, *g), 87);
    verifyNewString ("ippFCode",             getPlanetProperty(pl, game::interface::ippFCode,             session, *r, *g), "jkl");
    verifyNewInteger("ippFactories",         getPlanetProperty(pl, game::interface::ippFactories,         session, *r, *g), 30);
    verifyNewInteger("ippFactoriesMax",      getPlanetProperty(pl, game::interface::ippFactoriesMax,      session, *r, *g), 133);
    verifyNewInteger("ippFactoriesSpeed",    getPlanetProperty(pl, game::interface::ippFactoriesSpeed,    session, *r, *g), 10);
    verifyNewInteger("ippFactoriesWanted",   getPlanetProperty(pl, game::interface::ippFactoriesWanted,   session, *r, *g), 1000);
    verifyNewInteger("ippGroundD",           getPlanetProperty(pl, game::interface::ippGroundD,           session, *r, *g), 349);
    verifyNewInteger("ippGroundM",           getPlanetProperty(pl, game::interface::ippGroundM,           session, *r, *g), 781);
    verifyNewInteger("ippGroundN",           getPlanetProperty(pl, game::interface::ippGroundN,           session, *r, *g), 1092);
    verifyNewInteger("ippGroundT",           getPlanetProperty(pl, game::interface::ippGroundT,           session, *r, *g), 9102);
    verifyNewInteger("ippId",                getPlanetProperty(pl, game::interface::ippId,                session, *r, *g), 42);
    verifyNewString ("ippIndustry",          getPlanetProperty(pl, game::interface::ippIndustry,          session, *r, *g), "light");
    verifyNewInteger("ippIndustryCode",      getPlanetProperty(pl, game::interface::ippIndustryCode,      session, *r, *g), 1);
    verifyNewInteger("ippLevel",             getPlanetProperty(pl, game::interface::ippLevel,             session, *r, *g), 3);
    verifyNewInteger("ippLocX",              getPlanetProperty(pl, game::interface::ippLocX,              session, *r, *g), 1030);
    verifyNewInteger("ippLocY",              getPlanetProperty(pl, game::interface::ippLocY,              session, *r, *g), 2700);
    verifyNewBoolean("ippMarked",            getPlanetProperty(pl, game::interface::ippMarked,            session, *r, *g), false);
    verifyNewInteger("ippMinedD",            getPlanetProperty(pl, game::interface::ippMinedD,            session, *r, *g), 76);
    verifyNewInteger("ippMinedM",            getPlanetProperty(pl, game::interface::ippMinedM,            session, *r, *g), 230);
    verifyNewInteger("ippMinedN",            getPlanetProperty(pl, game::interface::ippMinedN,            session, *r, *g), 120);
    verifyNewString ("ippMinedStr",          getPlanetProperty(pl, game::interface::ippMinedStr,          session, *r, *g), "120N 84T 76D 230M");
    verifyNewInteger("ippMinedT",            getPlanetProperty(pl, game::interface::ippMinedT,            session, *r, *g), 84);
    verifyNewInteger("ippMineralTime",       getPlanetProperty(pl, game::interface::ippMineralTime,       session, *r, *g), 10);
    verifyNewInteger("ippMines",             getPlanetProperty(pl, game::interface::ippMines,             session, *r, *g), 20);
    verifyNewInteger("ippMinesMax",          getPlanetProperty(pl, game::interface::ippMinesMax,          session, *r, *g), 232);
    verifyNewInteger("ippMinesSpeed",        getPlanetProperty(pl, game::interface::ippMinesSpeed,        session, *r, *g), 5);
    verifyNewInteger("ippMinesWanted",       getPlanetProperty(pl, game::interface::ippMinesWanted,       session, *r, *g), 1000);
    verifyNewInteger("ippMoney",             getPlanetProperty(pl, game::interface::ippMoney,             session, *r, *g), 458);
    verifyNewString ("ippName",              getPlanetProperty(pl, game::interface::ippName,              session, *r, *g), "Earth 2");
    verifyNewInteger("ippNativeChange",      getPlanetProperty(pl, game::interface::ippNativeChange,      session, *r, *g), -4);
    verifyNewString ("ippNativeChangeStr",   getPlanetProperty(pl, game::interface::ippNativeChangeStr,   session, *r, *g), "They are angry about you!");
    verifyNewString ("ippNativeGov",         getPlanetProperty(pl, game::interface::ippNativeGov,         session, *r, *g), "Tribal");
    verifyNewInteger("ippNativeGovCode",     getPlanetProperty(pl, game::interface::ippNativeGovCode,     session, *r, *g), 4);
    verifyNewInteger("ippNativeHappy",       getPlanetProperty(pl, game::interface::ippNativeHappy,       session, *r, *g), 76);
    verifyNewString ("ippNativeHappyStr",    getPlanetProperty(pl, game::interface::ippNativeHappyStr,    session, *r, *g), "calm");
    verifyNewString ("ippNativeRace",        getPlanetProperty(pl, game::interface::ippNativeRace,        session, *r, *g), "Reptilian");
    verifyNewInteger("ippNativeRaceCode",    getPlanetProperty(pl, game::interface::ippNativeRaceCode,    session, *r, *g), 3);
    verifyNewInteger("ippNativeTax",         getPlanetProperty(pl, game::interface::ippNativeTax,         session, *r, *g), 12);
    verifyNewInteger("ippNativeTaxBase",     getPlanetProperty(pl, game::interface::ippNativeTaxBase,     session, *r, *g), 7);
    verifyNewInteger("ippNativeTaxIncome",   getPlanetProperty(pl, game::interface::ippNativeTaxIncome,   session, *r, *g), 75);
    verifyNewInteger("ippNativeTaxMax",      getPlanetProperty(pl, game::interface::ippNativeTaxMax,      session, *r, *g), 43);
    verifyNewInteger("ippNativeTime",        getPlanetProperty(pl, game::interface::ippNativeTime,        session, *r, *g), 10);
    verifyNewInteger("ippNatives",           getPlanetProperty(pl, game::interface::ippNatives,           session, *r, *g), 7821);
    verifyNewInteger("ippOrbitingEnemies",   getPlanetProperty(pl, game::interface::ippOrbitingEnemies,   session, *r, *g), 3);
    verifyNewInteger("ippOrbitingOwn",       getPlanetProperty(pl, game::interface::ippOrbitingOwn,       session, *r, *g), 2);
    verifyNewInteger("ippOrbitingShips",     getPlanetProperty(pl, game::interface::ippOrbitingShips,     session, *r, *g), 5);
    verifyNewBoolean("ippPlayed",            getPlanetProperty(pl, game::interface::ippPlayed,            session, *r, *g), true);
    verifyNewInteger("ippSupplies",          getPlanetProperty(pl, game::interface::ippSupplies,          session, *r, *g), 31);
    verifyNewBoolean("ippTask",              getPlanetProperty(pl, game::interface::ippTask,              session, *r, *g), false);
    verifyNewBoolean("ippTaskBase",          getPlanetProperty(pl, game::interface::ippTaskBase,          session, *r, *g), false);
    verifyNewInteger("ippTemp",              getPlanetProperty(pl, game::interface::ippTemp,              session, *r, *g), 53);
    verifyNewString ("ippTempStr",           getPlanetProperty(pl, game::interface::ippTempStr,           session, *r, *g), "warm");
    verifyNewString ("ippTypeChar",          getPlanetProperty(pl, game::interface::ippTypeChar,          session, *r, *g), "P");
    verifyNewString ("ippTypeStr",           getPlanetProperty(pl, game::interface::ippTypeStr,           session, *r, *g), "Planet");

    // Complex values
    {
        // ippEncodedMessage - long string, we don't want to check the entire content
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippEncodedMessage, session, *r, *g));
        String_t str;
        TS_ASSERT(interpreter::checkStringArg(str, p.get()));
        TS_ASSERT_DIFFERS(str, "OBJECT: Planet 42\n");
    }
    {
        // ippMessages - an iterable array
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippMessages, session, *r, *g));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        TS_ASSERT(ix != 0);
        interpreter::test::ValueVerifier verif(*ix, "ippMessages");
        verif.verifyBasics();
        verif.verifyNotSerializable();
        TS_ASSERT_EQUALS(ix->getDimension(0), 1);
        TS_ASSERT_EQUALS(ix->getDimension(1), 3);   // 2 messages

        // Quick test that messages can be retrieved
        std::auto_ptr<interpreter::Context> ctx(ix->makeFirstContext());
        TS_ASSERT(ctx.get() != 0);
        interpreter::test::ContextVerifier cv(*ctx, "ippMessages enum");
        cv.verifyBasics();
        cv.verifyNotSerializable();
        cv.verifyInteger("ID", 3);                  // 1-based, thus index 0 reported as 1 to user
        cv.verifyString("FULLTEXT", "msg...");
    }
    {
        // ippReference - reference
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippReference, session, *r, *g));
        Reference ref;
        TS_ASSERT(game::interface::checkReferenceArg(ref, p.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Planet, 42));
    }
    {
        // ippScore - function (not iterable)
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippScore, session, *r, *g));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        TS_ASSERT(ix != 0);
        interpreter::test::ValueVerifier verif(*ix, "ippScore");
        verif.verifyBasics();
        verif.verifyNotSerializable();
        TS_ASSERT_EQUALS(ix->getDimension(0), 0);
        TS_ASSERT_THROWS(ix->makeFirstContext(), interpreter::Error);

        // Retrieve existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewInteger("ippScore(Level)", ix->get(args), 3);
        }

        // Retrieve non-existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(999);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull("ippScore(999)", ix->get(args));
        }

        // Null index
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull("ippScore(999)", ix->get(args));
        }

        // Arity error
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 0);
            TS_ASSERT_THROWS(ix->get(args), interpreter::Error);
        }

        // Type error
        {
            afl::data::Segment seg;
            seg.pushBackString("X");
            interpreter::Arguments args(seg, 0, 1);
            TS_ASSERT_THROWS(ix->get(args), interpreter::Error);
        }

        // Not assignable
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            afl::data::IntegerValue iv(5);
            TS_ASSERT_THROWS(ix->set(args, &iv), interpreter::Error);
        }
    }

    // Writable properties
    {
        afl::data::IntegerValue iv(4);
        setPlanetProperty(pl, game::interface::ippMinesSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::MineBuilding), 4);
    }
    {
        afl::data::IntegerValue iv(140);
        setPlanetProperty(pl, game::interface::ippMinesWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::MineBuilding), 140);
    }
    {
        afl::data::IntegerValue iv(7);
        setPlanetProperty(pl, game::interface::ippFactoriesSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::FactoryBuilding), 7);
    }
    {
        afl::data::IntegerValue iv(170);
        setPlanetProperty(pl, game::interface::ippFactoriesWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::FactoryBuilding), 170);
    }
    {
        afl::data::IntegerValue iv(6);
        setPlanetProperty(pl, game::interface::ippDefenseSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::DefenseBuilding), 6);
    }
    {
        afl::data::IntegerValue iv(160);
        setPlanetProperty(pl, game::interface::ippDefenseWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::DefenseBuilding), 160);
    }
    {
        afl::data::IntegerValue iv(1);
        setPlanetProperty(pl, game::interface::ippBaseDefenseSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::BaseDefenseBuilding), 1);
    }
    {
        afl::data::IntegerValue iv(110);
        setPlanetProperty(pl, game::interface::ippBaseDefenseWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::BaseDefenseBuilding), 110);
    }
    {
        afl::data::IntegerValue iv(50);
        setPlanetProperty(pl, game::interface::ippColonistTax, &iv, *r);
        TS_ASSERT_EQUALS(pl.getColonistTax().orElse(-1), 50);
    }
    {
        afl::data::StringValue sv("wvx");
        setPlanetProperty(pl, game::interface::ippFCode, &sv, *r);
        TS_ASSERT_EQUALS(pl.getFriendlyCode().orElse(""), "wvx");
    }
    {
        afl::data::IntegerValue iv(60);
        setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r);
        TS_ASSERT_EQUALS(pl.getNativeTax().orElse(-1), 60);
    }

    // Error case: not assignable
    {
        afl::data::IntegerValue iv(60);
        TS_ASSERT_THROWS(setPlanetProperty(pl, game::interface::ippNativeChange, &iv, *r), interpreter::Error);
    }

    // Error case: range error
    {
        afl::data::IntegerValue iv(160);
        TS_ASSERT_THROWS(setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r), interpreter::Error);
    }
}

/** Test operation on an essentially-empty planet. */
void
TestGameInterfacePlanetProperty::testNull()
{
    const int PLAYER = 5;

    // Environment
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    afl::base::Ptr<game::Game> g = new game::Game();
    g->setViewpointPlayer(PLAYER);
    session.setGame(g);

    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))).asPtr();
    session.setRoot(r);

    // Planet
    game::map::Planet& pl = *g->currentTurn().universe().planets().create(42);
    pl.setPlayability(game::map::Object::NotPlayable);
    pl.internalCheck(g->mapConfiguration(), game::PlayerSet_t(PLAYER), TURN_NR, tx, session.log());

    // Verify the scalars
    verifyNewNull   ("ippBaseBuildFlag",     getPlanetProperty(pl, game::interface::ippBaseBuildFlag,     session, *r, *g));
    verifyNewInteger("ippBaseDefenseSpeed",  getPlanetProperty(pl, game::interface::ippBaseDefenseSpeed,  session, *r, *g), 2);
    verifyNewInteger("ippBaseDefenseWanted", getPlanetProperty(pl, game::interface::ippBaseDefenseWanted, session, *r, *g), 20);
    verifyNewBoolean("ippBaseFlag",          getPlanetProperty(pl, game::interface::ippBaseFlag,          session, *r, *g), false);
    verifyNewString ("ippBaseStr",           getPlanetProperty(pl, game::interface::ippBaseStr,           session, *r, *g), "-");
    verifyNewNull   ("ippCashTime",          getPlanetProperty(pl, game::interface::ippCashTime,          session, *r, *g));
    verifyNewNull   ("ippColonistChange",    getPlanetProperty(pl, game::interface::ippColonistChange,    session, *r, *g));
    verifyNewNull   ("ippColonistChangeStr", getPlanetProperty(pl, game::interface::ippColonistChangeStr, session, *r, *g));
    verifyNewNull   ("ippColonistHappy",     getPlanetProperty(pl, game::interface::ippColonistHappy,     session, *r, *g));
    verifyNewNull   ("ippColonistHappyStr",  getPlanetProperty(pl, game::interface::ippColonistHappyStr,  session, *r, *g));
    verifyNewNull   ("ippColonistSupported", getPlanetProperty(pl, game::interface::ippColonistSupported, session, *r, *g));
    verifyNewNull   ("ippColonistTax",       getPlanetProperty(pl, game::interface::ippColonistTax,       session, *r, *g));
    verifyNewNull   ("ippColonistTaxIncome", getPlanetProperty(pl, game::interface::ippColonistTaxIncome, session, *r, *g));
    verifyNewNull   ("ippColonistTime",      getPlanetProperty(pl, game::interface::ippColonistTime,      session, *r, *g));
    verifyNewNull   ("ippColonists",         getPlanetProperty(pl, game::interface::ippColonists,         session, *r, *g));
    verifyNewNull   ("ippDefense",           getPlanetProperty(pl, game::interface::ippDefense,           session, *r, *g));
    verifyNewNull   ("ippDefenseMax",        getPlanetProperty(pl, game::interface::ippDefenseMax,        session, *r, *g));
    verifyNewInteger("ippDefenseSpeed",      getPlanetProperty(pl, game::interface::ippDefenseSpeed,      session, *r, *g), 3);
    verifyNewInteger("ippDefenseWanted",     getPlanetProperty(pl, game::interface::ippDefenseWanted,     session, *r, *g), 1000);
    verifyNewNull   ("ippDensityD",          getPlanetProperty(pl, game::interface::ippDensityD,          session, *r, *g));
    verifyNewNull   ("ippDensityM",          getPlanetProperty(pl, game::interface::ippDensityM,          session, *r, *g));
    verifyNewNull   ("ippDensityN",          getPlanetProperty(pl, game::interface::ippDensityN,          session, *r, *g));
    verifyNewNull   ("ippDensityT",          getPlanetProperty(pl, game::interface::ippDensityT,          session, *r, *g));
    verifyNewNull   ("ippFCode",             getPlanetProperty(pl, game::interface::ippFCode,             session, *r, *g));
    verifyNewNull   ("ippFactories",         getPlanetProperty(pl, game::interface::ippFactories,         session, *r, *g));
    verifyNewNull   ("ippFactoriesMax",      getPlanetProperty(pl, game::interface::ippFactoriesMax,      session, *r, *g));
    verifyNewInteger("ippFactoriesSpeed",    getPlanetProperty(pl, game::interface::ippFactoriesSpeed,    session, *r, *g), 10);
    verifyNewInteger("ippFactoriesWanted",   getPlanetProperty(pl, game::interface::ippFactoriesWanted,   session, *r, *g), 1000);
    verifyNewNull   ("ippGroundD",           getPlanetProperty(pl, game::interface::ippGroundD,           session, *r, *g));
    verifyNewNull   ("ippGroundM",           getPlanetProperty(pl, game::interface::ippGroundM,           session, *r, *g));
    verifyNewNull   ("ippGroundN",           getPlanetProperty(pl, game::interface::ippGroundN,           session, *r, *g));
    verifyNewNull   ("ippGroundT",           getPlanetProperty(pl, game::interface::ippGroundT,           session, *r, *g));
    verifyNewInteger("ippId",                getPlanetProperty(pl, game::interface::ippId,                session, *r, *g), 42);
    verifyNewNull   ("ippIndustry",          getPlanetProperty(pl, game::interface::ippIndustry,          session, *r, *g));
    verifyNewNull   ("ippIndustryCode",      getPlanetProperty(pl, game::interface::ippIndustryCode,      session, *r, *g));
    verifyNewNull   ("ippLevel",             getPlanetProperty(pl, game::interface::ippLevel,             session, *r, *g));
    verifyNewNull   ("ippLocX",              getPlanetProperty(pl, game::interface::ippLocX,              session, *r, *g));
    verifyNewNull   ("ippLocY",              getPlanetProperty(pl, game::interface::ippLocY,              session, *r, *g));
    verifyNewBoolean("ippMarked",            getPlanetProperty(pl, game::interface::ippMarked,            session, *r, *g), false);
    verifyNewNull   ("ippMinedD",            getPlanetProperty(pl, game::interface::ippMinedD,            session, *r, *g));
    verifyNewNull   ("ippMinedM",            getPlanetProperty(pl, game::interface::ippMinedM,            session, *r, *g));
    verifyNewNull   ("ippMinedN",            getPlanetProperty(pl, game::interface::ippMinedN,            session, *r, *g));
    verifyNewNull   ("ippMinedStr",          getPlanetProperty(pl, game::interface::ippMinedStr,          session, *r, *g));
    verifyNewNull   ("ippMinedT",            getPlanetProperty(pl, game::interface::ippMinedT,            session, *r, *g));
    verifyNewNull   ("ippMineralTime",       getPlanetProperty(pl, game::interface::ippMineralTime,       session, *r, *g));
    verifyNewNull   ("ippMines",             getPlanetProperty(pl, game::interface::ippMines,             session, *r, *g));
    verifyNewNull   ("ippMinesMax",          getPlanetProperty(pl, game::interface::ippMinesMax,          session, *r, *g));
    verifyNewInteger("ippMinesSpeed",        getPlanetProperty(pl, game::interface::ippMinesSpeed,        session, *r, *g), 5);
    verifyNewInteger("ippMinesWanted",       getPlanetProperty(pl, game::interface::ippMinesWanted,       session, *r, *g), 1000);
    verifyNewNull   ("ippMoney",             getPlanetProperty(pl, game::interface::ippMoney,             session, *r, *g));
    verifyNewString ("ippName",              getPlanetProperty(pl, game::interface::ippName,              session, *r, *g), "?"); // Probably not contractual
    verifyNewNull   ("ippNativeChange",      getPlanetProperty(pl, game::interface::ippNativeChange,      session, *r, *g));
    verifyNewNull   ("ippNativeChangeStr",   getPlanetProperty(pl, game::interface::ippNativeChangeStr,   session, *r, *g));
    verifyNewNull   ("ippNativeGov",         getPlanetProperty(pl, game::interface::ippNativeGov,         session, *r, *g));
    verifyNewNull   ("ippNativeGovCode",     getPlanetProperty(pl, game::interface::ippNativeGovCode,     session, *r, *g));
    verifyNewNull   ("ippNativeHappy",       getPlanetProperty(pl, game::interface::ippNativeHappy,       session, *r, *g));
    verifyNewNull   ("ippNativeHappyStr",    getPlanetProperty(pl, game::interface::ippNativeHappyStr,    session, *r, *g));
    verifyNewNull   ("ippNativeRace",        getPlanetProperty(pl, game::interface::ippNativeRace,        session, *r, *g));
    verifyNewNull   ("ippNativeRaceCode",    getPlanetProperty(pl, game::interface::ippNativeRaceCode,    session, *r, *g));
    verifyNewNull   ("ippNativeTax",         getPlanetProperty(pl, game::interface::ippNativeTax,         session, *r, *g));
    verifyNewNull   ("ippNativeTaxBase",     getPlanetProperty(pl, game::interface::ippNativeTaxBase,     session, *r, *g));
    verifyNewNull   ("ippNativeTaxIncome",   getPlanetProperty(pl, game::interface::ippNativeTaxIncome,   session, *r, *g));
    verifyNewNull   ("ippNativeTaxMax",      getPlanetProperty(pl, game::interface::ippNativeTaxMax,      session, *r, *g));
    verifyNewNull   ("ippNativeTime",        getPlanetProperty(pl, game::interface::ippNativeTime,        session, *r, *g));
    verifyNewNull   ("ippNatives",           getPlanetProperty(pl, game::interface::ippNatives,           session, *r, *g));
    verifyNewNull   ("ippOrbitingEnemies",   getPlanetProperty(pl, game::interface::ippOrbitingEnemies,   session, *r, *g));
    verifyNewNull   ("ippOrbitingOwn",       getPlanetProperty(pl, game::interface::ippOrbitingOwn,       session, *r, *g));
    verifyNewNull   ("ippOrbitingShips",     getPlanetProperty(pl, game::interface::ippOrbitingShips,     session, *r, *g));
    verifyNewBoolean("ippPlayed",            getPlanetProperty(pl, game::interface::ippPlayed,            session, *r, *g), false);
    verifyNewNull   ("ippSupplies",          getPlanetProperty(pl, game::interface::ippSupplies,          session, *r, *g));
    verifyNewBoolean("ippTask",              getPlanetProperty(pl, game::interface::ippTask,              session, *r, *g), false);
    verifyNewBoolean("ippTaskBase",          getPlanetProperty(pl, game::interface::ippTaskBase,          session, *r, *g), false);
    verifyNewNull   ("ippTemp",              getPlanetProperty(pl, game::interface::ippTemp,              session, *r, *g));
    verifyNewNull   ("ippTempStr",           getPlanetProperty(pl, game::interface::ippTempStr,           session, *r, *g));
    verifyNewString ("ippTypeChar",          getPlanetProperty(pl, game::interface::ippTypeChar,          session, *r, *g), "P");
    verifyNewString ("ippTypeStr",           getPlanetProperty(pl, game::interface::ippTypeStr,           session, *r, *g), "Planet");

    // Complex values
    {
        // ippEncodedMessage - long string, we don't want to check the entire content; always valid even if planet is mostly unknown
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippEncodedMessage, session, *r, *g));
        String_t str;
        TS_ASSERT(interpreter::checkStringArg(str, p.get()));
        TS_ASSERT_DIFFERS(str, "OBJECT: Planet 42\n");
    }
    {
        // ippMessages - an iterable array, but null if nothing known
        verifyNewNull("ippMessages", getPlanetProperty(pl, game::interface::ippMessages, session, *r, *g));
    }
    {
        // ippReference - reference, always present
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippReference, session, *r, *g));
        Reference ref;
        TS_ASSERT(game::interface::checkReferenceArg(ref, p.get()));
        TS_ASSERT_EQUALS(ref, Reference(Reference::Planet, 42));
    }
    {
        // ippScore - function (not iterable), always present
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippScore, session, *r, *g));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        TS_ASSERT(ix != 0);
        interpreter::test::ValueVerifier verif(*ix, "ippScore");
        verif.verifyBasics();
        verif.verifyNotSerializable();
        TS_ASSERT_EQUALS(ix->getDimension(0), 0);
        TS_ASSERT_THROWS(ix->makeFirstContext(), interpreter::Error);

        // Score is null
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull("ippScore(Level)", ix->get(args));
        }
    }

    // Writable properties
    // -- build goals/speeds can always be written --
    {
        afl::data::IntegerValue iv(4);
        setPlanetProperty(pl, game::interface::ippMinesSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::MineBuilding), 4);
    }
    {
        afl::data::IntegerValue iv(140);
        setPlanetProperty(pl, game::interface::ippMinesWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::MineBuilding), 140);
    }
    {
        afl::data::IntegerValue iv(7);
        setPlanetProperty(pl, game::interface::ippFactoriesSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::FactoryBuilding), 7);
    }
    {
        afl::data::IntegerValue iv(170);
        setPlanetProperty(pl, game::interface::ippFactoriesWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::FactoryBuilding), 170);
    }
    {
        afl::data::IntegerValue iv(6);
        setPlanetProperty(pl, game::interface::ippDefenseSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::DefenseBuilding), 6);
    }
    {
        afl::data::IntegerValue iv(160);
        setPlanetProperty(pl, game::interface::ippDefenseWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::DefenseBuilding), 160);
    }
    {
        afl::data::IntegerValue iv(1);
        setPlanetProperty(pl, game::interface::ippBaseDefenseSpeed, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildSpeed(game::BaseDefenseBuilding), 1);
    }
    {
        afl::data::IntegerValue iv(110);
        setPlanetProperty(pl, game::interface::ippBaseDefenseWanted, &iv, *r);
        TS_ASSERT_EQUALS(pl.getAutobuildGoal(game::BaseDefenseBuilding), 110);
    }
    // -- cannot write others --
    {
        afl::data::IntegerValue iv(50);
        TS_ASSERT_THROWS(setPlanetProperty(pl, game::interface::ippColonistTax, &iv, *r), interpreter::Error);
    }
    {
        afl::data::StringValue sv("wvx");
        TS_ASSERT_THROWS(setPlanetProperty(pl, game::interface::ippFCode, &sv, *r), interpreter::Error);
    }

    {
        afl::data::IntegerValue iv(60);
        TS_ASSERT_THROWS(setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r), interpreter::Error);
    }
}

