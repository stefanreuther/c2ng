/**
  *  \file test/game/interface/planetpropertytest.cpp
  *  \brief Test for game::interface::PlanetProperty
  */

#include "game/interface/planetproperty.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
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
using game::config::HostConfiguration;
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
AFL_TEST("game.interface.PlanetProperty:full", a)
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
    r->hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);
    r->hostConfiguration()[HostConfiguration::EPPlanetAging].set(42);
    r->hostConfiguration()[HostConfiguration::EPPlanetGovernment].set(50);
    r->hostConfiguration()[HostConfiguration::ExperienceLevelNames].set("Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm");
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

    game::UnitScoreDefinitionList::Definition pointDef;
    pointDef.name  = "Point";
    pointDef.id    = game::ScoreId_ExpPoints;
    pointDef.limit = -1;
    pl.unitScores().set(g->planetScores().add(pointDef), 3333, TURN_NR);

    // Player definition
    game::Player& player = *r->playerList().create(5);
    player.setName(game::Player::LongName, "The Orion Pirates");
    player.setName(game::Player::ShortName, "The Pirates");
    player.setName(game::Player::AdjectiveName, "Orion");

    // Verify the scalars
    verifyNewBoolean(a("ippBaseBuildFlag"),     getPlanetProperty(pl, game::interface::ippBaseBuildFlag,     session, *r, *g, g->currentTurn()), true);
    verifyNewInteger(a("ippBaseDefenseSpeed"),  getPlanetProperty(pl, game::interface::ippBaseDefenseSpeed,  session, *r, *g, g->currentTurn()), 2);
    verifyNewInteger(a("ippBaseDefenseWanted"), getPlanetProperty(pl, game::interface::ippBaseDefenseWanted, session, *r, *g, g->currentTurn()), 20);
    verifyNewBoolean(a("ippBaseFlag"),          getPlanetProperty(pl, game::interface::ippBaseFlag,          session, *r, *g, g->currentTurn()), false);
    verifyNewString (a("ippBaseStr"),           getPlanetProperty(pl, game::interface::ippBaseStr,           session, *r, *g, g->currentTurn()), "being built");
    verifyNewInteger(a("ippCashTime"),          getPlanetProperty(pl, game::interface::ippCashTime,          session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippColonistChange"),    getPlanetProperty(pl, game::interface::ippColonistChange,    session, *r, *g, g->currentTurn()), 6);
    verifyNewString (a("ippColonistChangeStr"), getPlanetProperty(pl, game::interface::ippColonistChangeStr, session, *r, *g, g->currentTurn()), "They LOVE you.");
    verifyNewInteger(a("ippColonistHappy"),     getPlanetProperty(pl, game::interface::ippColonistHappy,     session, *r, *g, g->currentTurn()), 97);
    verifyNewString (a("ippColonistHappyStr"),  getPlanetProperty(pl, game::interface::ippColonistHappyStr,  session, *r, *g, g->currentTurn()), "happy");
    verifyNewInteger(a("ippColonistSupported"), getPlanetProperty(pl, game::interface::ippColonistSupported, session, *r, *g, g->currentTurn()), 99556);
    verifyNewInteger(a("ippColonistTax"),       getPlanetProperty(pl, game::interface::ippColonistTax,       session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippColonistTaxIncome"), getPlanetProperty(pl, game::interface::ippColonistTaxIncome, session, *r, *g, g->currentTurn()), 4);
    verifyNewInteger(a("ippColonistTime"),      getPlanetProperty(pl, game::interface::ippColonistTime,      session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippColonists"),         getPlanetProperty(pl, game::interface::ippColonists,         session, *r, *g, g->currentTurn()), 1200);
    verifyNewInteger(a("ippDefense"),           getPlanetProperty(pl, game::interface::ippDefense,           session, *r, *g, g->currentTurn()), 15);
    verifyNewInteger(a("ippDefenseMax"),        getPlanetProperty(pl, game::interface::ippDefenseMax,        session, *r, *g, g->currentTurn()), 84);
    verifyNewInteger(a("ippDefenseSpeed"),      getPlanetProperty(pl, game::interface::ippDefenseSpeed,      session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippDefenseWanted"),     getPlanetProperty(pl, game::interface::ippDefenseWanted,     session, *r, *g, g->currentTurn()), 1000);
    verifyNewInteger(a("ippDensityD"),          getPlanetProperty(pl, game::interface::ippDensityD,          session, *r, *g, g->currentTurn()), 29);
    verifyNewInteger(a("ippDensityM"),          getPlanetProperty(pl, game::interface::ippDensityM,          session, *r, *g, g->currentTurn()), 7);
    verifyNewInteger(a("ippDensityN"),          getPlanetProperty(pl, game::interface::ippDensityN,          session, *r, *g, g->currentTurn()), 14);
    verifyNewInteger(a("ippDensityT"),          getPlanetProperty(pl, game::interface::ippDensityT,          session, *r, *g, g->currentTurn()), 87);
    verifyNewString (a("ippFCode"),             getPlanetProperty(pl, game::interface::ippFCode,             session, *r, *g, g->currentTurn()), "jkl");
    verifyNewInteger(a("ippFactories"),         getPlanetProperty(pl, game::interface::ippFactories,         session, *r, *g, g->currentTurn()), 30);
    verifyNewInteger(a("ippFactoriesMax"),      getPlanetProperty(pl, game::interface::ippFactoriesMax,      session, *r, *g, g->currentTurn()), 133);
    verifyNewInteger(a("ippFactoriesSpeed"),    getPlanetProperty(pl, game::interface::ippFactoriesSpeed,    session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippFactoriesWanted"),   getPlanetProperty(pl, game::interface::ippFactoriesWanted,   session, *r, *g, g->currentTurn()), 1000);
    verifyNewInteger(a("ippGroundD"),           getPlanetProperty(pl, game::interface::ippGroundD,           session, *r, *g, g->currentTurn()), 349);
    verifyNewInteger(a("ippGroundM"),           getPlanetProperty(pl, game::interface::ippGroundM,           session, *r, *g, g->currentTurn()), 781);
    verifyNewInteger(a("ippGroundN"),           getPlanetProperty(pl, game::interface::ippGroundN,           session, *r, *g, g->currentTurn()), 1092);
    verifyNewInteger(a("ippGroundT"),           getPlanetProperty(pl, game::interface::ippGroundT,           session, *r, *g, g->currentTurn()), 9102);
    verifyNewInteger(a("ippId"),                getPlanetProperty(pl, game::interface::ippId,                session, *r, *g, g->currentTurn()), 42);
    verifyNewString (a("ippIndustry"),          getPlanetProperty(pl, game::interface::ippIndustry,          session, *r, *g, g->currentTurn()), "light");
    verifyNewInteger(a("ippIndustryCode"),      getPlanetProperty(pl, game::interface::ippIndustryCode,      session, *r, *g, g->currentTurn()), 1);
    verifyNewInteger(a("ippLevel"),             getPlanetProperty(pl, game::interface::ippLevel,             session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippLevelGain"),         getPlanetProperty(pl, game::interface::ippLevelGain,         session, *r, *g, g->currentTurn()), 78); /* 42 aging + 50% * 72 (= nhappy + nchange) */
    verifyNewString (a("ippLevelName"),         getPlanetProperty(pl, game::interface::ippLevelName,         session, *r, *g, g->currentTurn()), "Ladehugo");
    verifyNewInteger(a("ippLevelPoints"),       getPlanetProperty(pl, game::interface::ippLevelPoints,       session, *r, *g, g->currentTurn()), 3333);
    verifyNewInteger(a("ippLocX"),              getPlanetProperty(pl, game::interface::ippLocX,              session, *r, *g, g->currentTurn()), 1030);
    verifyNewInteger(a("ippLocY"),              getPlanetProperty(pl, game::interface::ippLocY,              session, *r, *g, g->currentTurn()), 2700);
    verifyNewBoolean(a("ippMarked"),            getPlanetProperty(pl, game::interface::ippMarked,            session, *r, *g, g->currentTurn()), false);
    verifyNewInteger(a("ippMinedD"),            getPlanetProperty(pl, game::interface::ippMinedD,            session, *r, *g, g->currentTurn()), 76);
    verifyNewInteger(a("ippMinedM"),            getPlanetProperty(pl, game::interface::ippMinedM,            session, *r, *g, g->currentTurn()), 230);
    verifyNewInteger(a("ippMinedN"),            getPlanetProperty(pl, game::interface::ippMinedN,            session, *r, *g, g->currentTurn()), 120);
    verifyNewString (a("ippMinedStr"),          getPlanetProperty(pl, game::interface::ippMinedStr,          session, *r, *g, g->currentTurn()), "120N 84T 76D 230M");
    verifyNewInteger(a("ippMinedT"),            getPlanetProperty(pl, game::interface::ippMinedT,            session, *r, *g, g->currentTurn()), 84);
    verifyNewInteger(a("ippMineralTime"),       getPlanetProperty(pl, game::interface::ippMineralTime,       session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippMines"),             getPlanetProperty(pl, game::interface::ippMines,             session, *r, *g, g->currentTurn()), 20);
    verifyNewInteger(a("ippMinesMax"),          getPlanetProperty(pl, game::interface::ippMinesMax,          session, *r, *g, g->currentTurn()), 232);
    verifyNewInteger(a("ippMinesSpeed"),        getPlanetProperty(pl, game::interface::ippMinesSpeed,        session, *r, *g, g->currentTurn()), 5);
    verifyNewInteger(a("ippMinesWanted"),       getPlanetProperty(pl, game::interface::ippMinesWanted,       session, *r, *g, g->currentTurn()), 1000);
    verifyNewInteger(a("ippMoney"),             getPlanetProperty(pl, game::interface::ippMoney,             session, *r, *g, g->currentTurn()), 458);
    verifyNewString (a("ippName"),              getPlanetProperty(pl, game::interface::ippName,              session, *r, *g, g->currentTurn()), "Earth 2");
    verifyNewInteger(a("ippNativeChange"),      getPlanetProperty(pl, game::interface::ippNativeChange,      session, *r, *g, g->currentTurn()), -4);
    verifyNewString (a("ippNativeChangeStr"),   getPlanetProperty(pl, game::interface::ippNativeChangeStr,   session, *r, *g, g->currentTurn()), "They are angry about you!");
    verifyNewString (a("ippNativeGov"),         getPlanetProperty(pl, game::interface::ippNativeGov,         session, *r, *g, g->currentTurn()), "Tribal");
    verifyNewInteger(a("ippNativeGovCode"),     getPlanetProperty(pl, game::interface::ippNativeGovCode,     session, *r, *g, g->currentTurn()), 4);
    verifyNewInteger(a("ippNativeHappy"),       getPlanetProperty(pl, game::interface::ippNativeHappy,       session, *r, *g, g->currentTurn()), 76);
    verifyNewString (a("ippNativeHappyStr"),    getPlanetProperty(pl, game::interface::ippNativeHappyStr,    session, *r, *g, g->currentTurn()), "calm");
    verifyNewString (a("ippNativeRace"),        getPlanetProperty(pl, game::interface::ippNativeRace,        session, *r, *g, g->currentTurn()), "Reptilian");
    verifyNewInteger(a("ippNativeRaceCode"),    getPlanetProperty(pl, game::interface::ippNativeRaceCode,    session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippNativeTax"),         getPlanetProperty(pl, game::interface::ippNativeTax,         session, *r, *g, g->currentTurn()), 12);
    verifyNewInteger(a("ippNativeTaxBase"),     getPlanetProperty(pl, game::interface::ippNativeTaxBase,     session, *r, *g, g->currentTurn()), 7);
    verifyNewInteger(a("ippNativeTaxIncome"),   getPlanetProperty(pl, game::interface::ippNativeTaxIncome,   session, *r, *g, g->currentTurn()), 75);
    verifyNewInteger(a("ippNativeTaxMax"),      getPlanetProperty(pl, game::interface::ippNativeTaxMax,      session, *r, *g, g->currentTurn()), 43);
    verifyNewInteger(a("ippNativeTime"),        getPlanetProperty(pl, game::interface::ippNativeTime,        session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippNatives"),           getPlanetProperty(pl, game::interface::ippNatives,           session, *r, *g, g->currentTurn()), 7821);
    verifyNewInteger(a("ippOrbitingEnemies"),   getPlanetProperty(pl, game::interface::ippOrbitingEnemies,   session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippOrbitingOwn"),       getPlanetProperty(pl, game::interface::ippOrbitingOwn,       session, *r, *g, g->currentTurn()), 2);
    verifyNewInteger(a("ippOrbitingShips"),     getPlanetProperty(pl, game::interface::ippOrbitingShips,     session, *r, *g, g->currentTurn()), 5);
    verifyNewBoolean(a("ippPlayed"),            getPlanetProperty(pl, game::interface::ippPlayed,            session, *r, *g, g->currentTurn()), true);
    verifyNewInteger(a("ippSupplies"),          getPlanetProperty(pl, game::interface::ippSupplies,          session, *r, *g, g->currentTurn()), 31);
    verifyNewBoolean(a("ippTask"),              getPlanetProperty(pl, game::interface::ippTask,              session, *r, *g, g->currentTurn()), false);
    verifyNewBoolean(a("ippTaskBase"),          getPlanetProperty(pl, game::interface::ippTaskBase,          session, *r, *g, g->currentTurn()), false);
    verifyNewInteger(a("ippTemp"),              getPlanetProperty(pl, game::interface::ippTemp,              session, *r, *g, g->currentTurn()), 53);
    verifyNewString (a("ippTempStr"),           getPlanetProperty(pl, game::interface::ippTempStr,           session, *r, *g, g->currentTurn()), "warm");
    verifyNewString (a("ippTypeChar"),          getPlanetProperty(pl, game::interface::ippTypeChar,          session, *r, *g, g->currentTurn()), "P");
    verifyNewString (a("ippTypeStr"),           getPlanetProperty(pl, game::interface::ippTypeStr,           session, *r, *g, g->currentTurn()), "Planet");

    // Complex values
    {
        // ippEncodedMessage - long string, we don't want to check the entire content
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippEncodedMessage, session, *r, *g, g->currentTurn()));
        String_t str;
        a.check("ippEncodedMessage", interpreter::checkStringArg(str, p.get()));
        a.checkDifferent("ippEncodedMessage", str, "OBJECT: Planet 42\n");
    }
    {
        // ippMessages - an iterable array
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippMessages, session, *r, *g, g->currentTurn()));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ippMessages: IndexableValue", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ippMessages"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ippMessages: dim 0", ix->getDimension(0), 1);
        a.checkEqual("ippMessages: dim 1", ix->getDimension(1), 3);   // 2 messages

        // Quick test that messages can be retrieved
        std::auto_ptr<interpreter::Context> ctx(ix->makeFirstContext());
        a.checkNonNull("ippMessages: ctx", ctx.get());
        interpreter::test::ContextVerifier cv(*ctx, a("ippMessages enum"));
        cv.verifyBasics();
        cv.verifyNotSerializable();
        cv.verifyInteger("ID", 3);                  // 1-based, thus index 0 reported as 1 to user
        cv.verifyString("FULLTEXT", "msg...");
    }
    {
        // ippReference - reference
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippReference, session, *r, *g, g->currentTurn()));
        Reference ref;
        a.check("ippReference", game::interface::checkReferenceArg(ref, p.get()));
        a.checkEqual("ippReference", ref, Reference(Reference::Planet, 42));
    }
    {
        // ippScore - function (not iterable)
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippScore, session, *r, *g, g->currentTurn()));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ippScore", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ippScore"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ippScore: dim 0", ix->getDimension(0), 0);
        AFL_CHECK_THROWS(a("ippScore: makeFirstContext"), ix->makeFirstContext(), interpreter::Error);

        // Retrieve existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewInteger(a("ippScore(Level)"), ix->get(args), 3);
        }

        // Retrieve non-existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(999);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ippScore(999)"), ix->get(args));
        }

        // Null index
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ippScore(999)"), ix->get(args));
        }

        // Arity error
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 0);
            AFL_CHECK_THROWS(a("ippScore: arity error"), ix->get(args), interpreter::Error);
        }

        // Type error
        {
            afl::data::Segment seg;
            seg.pushBackString("X");
            interpreter::Arguments args(seg, 0, 1);
            AFL_CHECK_THROWS(a("ippScore: type error"), ix->get(args), interpreter::Error);
        }

        // Not assignable
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            afl::data::IntegerValue iv(5);
            AFL_CHECK_THROWS(a("ippScore: set"), ix->set(args, &iv), interpreter::Error);
        }
    }

    // Writable properties
    {
        afl::data::IntegerValue iv(4);
        setPlanetProperty(pl, game::interface::ippMinesSpeed, &iv, *r);
        a.checkEqual("ippMinesSpeed", pl.getAutobuildSpeed(game::MineBuilding), 4);
    }
    {
        afl::data::IntegerValue iv(140);
        setPlanetProperty(pl, game::interface::ippMinesWanted, &iv, *r);
        a.checkEqual("ippMinesWanted", pl.getAutobuildGoal(game::MineBuilding), 140);
    }
    {
        afl::data::IntegerValue iv(7);
        setPlanetProperty(pl, game::interface::ippFactoriesSpeed, &iv, *r);
        a.checkEqual("ippFactoriesSpeed", pl.getAutobuildSpeed(game::FactoryBuilding), 7);
    }
    {
        afl::data::IntegerValue iv(170);
        setPlanetProperty(pl, game::interface::ippFactoriesWanted, &iv, *r);
        a.checkEqual("ippFactoriesWanted", pl.getAutobuildGoal(game::FactoryBuilding), 170);
    }
    {
        afl::data::IntegerValue iv(6);
        setPlanetProperty(pl, game::interface::ippDefenseSpeed, &iv, *r);
        a.checkEqual("ippDefenseSpeed", pl.getAutobuildSpeed(game::DefenseBuilding), 6);
    }
    {
        afl::data::IntegerValue iv(160);
        setPlanetProperty(pl, game::interface::ippDefenseWanted, &iv, *r);
        a.checkEqual("ippDefenseWanted", pl.getAutobuildGoal(game::DefenseBuilding), 160);
    }
    {
        afl::data::IntegerValue iv(1);
        setPlanetProperty(pl, game::interface::ippBaseDefenseSpeed, &iv, *r);
        a.checkEqual("ippBaseDefenseSpeed", pl.getAutobuildSpeed(game::BaseDefenseBuilding), 1);
    }
    {
        afl::data::IntegerValue iv(110);
        setPlanetProperty(pl, game::interface::ippBaseDefenseWanted, &iv, *r);
        a.checkEqual("ippBaseDefenseWanted", pl.getAutobuildGoal(game::BaseDefenseBuilding), 110);
    }
    {
        afl::data::IntegerValue iv(50);
        setPlanetProperty(pl, game::interface::ippColonistTax, &iv, *r);
        a.checkEqual("ippColonistTax", pl.getColonistTax().orElse(-1), 50);
    }
    {
        afl::data::StringValue sv("wvx");
        setPlanetProperty(pl, game::interface::ippFCode, &sv, *r);
        a.checkEqual("ippFCode", pl.getFriendlyCode().orElse(""), "wvx");
    }
    {
        afl::data::IntegerValue iv(60);
        setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r);
        a.checkEqual("ippNativeTax", pl.getNativeTax().orElse(-1), 60);
    }

    // Error case: not assignable
    {
        afl::data::IntegerValue iv(60);
        AFL_CHECK_THROWS(a("ippNativeChange: not assignable"), setPlanetProperty(pl, game::interface::ippNativeChange, &iv, *r), interpreter::Error);
    }

    // Error case: range error
    {
        afl::data::IntegerValue iv(160);
        AFL_CHECK_THROWS(a("ippNativeTax: range error"), setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r), interpreter::Error);
    }
}

/** Test operation on an essentially-empty planet. */
AFL_TEST("game.interface.PlanetProperty:empty", a)
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
    verifyNewNull   (a("ippBaseBuildFlag"),     getPlanetProperty(pl, game::interface::ippBaseBuildFlag,     session, *r, *g, g->currentTurn()));
    verifyNewInteger(a("ippBaseDefenseSpeed"),  getPlanetProperty(pl, game::interface::ippBaseDefenseSpeed,  session, *r, *g, g->currentTurn()), 2);
    verifyNewInteger(a("ippBaseDefenseWanted"), getPlanetProperty(pl, game::interface::ippBaseDefenseWanted, session, *r, *g, g->currentTurn()), 20);
    verifyNewBoolean(a("ippBaseFlag"),          getPlanetProperty(pl, game::interface::ippBaseFlag,          session, *r, *g, g->currentTurn()), false);
    verifyNewString (a("ippBaseStr"),           getPlanetProperty(pl, game::interface::ippBaseStr,           session, *r, *g, g->currentTurn()), "-");
    verifyNewNull   (a("ippCashTime"),          getPlanetProperty(pl, game::interface::ippCashTime,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistChange"),    getPlanetProperty(pl, game::interface::ippColonistChange,    session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistChangeStr"), getPlanetProperty(pl, game::interface::ippColonistChangeStr, session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistHappy"),     getPlanetProperty(pl, game::interface::ippColonistHappy,     session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistHappyStr"),  getPlanetProperty(pl, game::interface::ippColonistHappyStr,  session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistSupported"), getPlanetProperty(pl, game::interface::ippColonistSupported, session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistTax"),       getPlanetProperty(pl, game::interface::ippColonistTax,       session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistTaxIncome"), getPlanetProperty(pl, game::interface::ippColonistTaxIncome, session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonistTime"),      getPlanetProperty(pl, game::interface::ippColonistTime,      session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippColonists"),         getPlanetProperty(pl, game::interface::ippColonists,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippDefense"),           getPlanetProperty(pl, game::interface::ippDefense,           session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippDefenseMax"),        getPlanetProperty(pl, game::interface::ippDefenseMax,        session, *r, *g, g->currentTurn()));
    verifyNewInteger(a("ippDefenseSpeed"),      getPlanetProperty(pl, game::interface::ippDefenseSpeed,      session, *r, *g, g->currentTurn()), 3);
    verifyNewInteger(a("ippDefenseWanted"),     getPlanetProperty(pl, game::interface::ippDefenseWanted,     session, *r, *g, g->currentTurn()), 1000);
    verifyNewNull   (a("ippDensityD"),          getPlanetProperty(pl, game::interface::ippDensityD,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippDensityM"),          getPlanetProperty(pl, game::interface::ippDensityM,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippDensityN"),          getPlanetProperty(pl, game::interface::ippDensityN,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippDensityT"),          getPlanetProperty(pl, game::interface::ippDensityT,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippFCode"),             getPlanetProperty(pl, game::interface::ippFCode,             session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippFactories"),         getPlanetProperty(pl, game::interface::ippFactories,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippFactoriesMax"),      getPlanetProperty(pl, game::interface::ippFactoriesMax,      session, *r, *g, g->currentTurn()));
    verifyNewInteger(a("ippFactoriesSpeed"),    getPlanetProperty(pl, game::interface::ippFactoriesSpeed,    session, *r, *g, g->currentTurn()), 10);
    verifyNewInteger(a("ippFactoriesWanted"),   getPlanetProperty(pl, game::interface::ippFactoriesWanted,   session, *r, *g, g->currentTurn()), 1000);
    verifyNewNull   (a("ippGroundD"),           getPlanetProperty(pl, game::interface::ippGroundD,           session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippGroundM"),           getPlanetProperty(pl, game::interface::ippGroundM,           session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippGroundN"),           getPlanetProperty(pl, game::interface::ippGroundN,           session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippGroundT"),           getPlanetProperty(pl, game::interface::ippGroundT,           session, *r, *g, g->currentTurn()));
    verifyNewInteger(a("ippId"),                getPlanetProperty(pl, game::interface::ippId,                session, *r, *g, g->currentTurn()), 42);
    verifyNewNull   (a("ippIndustry"),          getPlanetProperty(pl, game::interface::ippIndustry,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippIndustryCode"),      getPlanetProperty(pl, game::interface::ippIndustryCode,      session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLevel"),             getPlanetProperty(pl, game::interface::ippLevel,             session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLevelGain"),         getPlanetProperty(pl, game::interface::ippLevelGain,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLevelName"),         getPlanetProperty(pl, game::interface::ippLevelName,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLevelPoints"),       getPlanetProperty(pl, game::interface::ippLevelPoints,       session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLocX"),              getPlanetProperty(pl, game::interface::ippLocX,              session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippLocY"),              getPlanetProperty(pl, game::interface::ippLocY,              session, *r, *g, g->currentTurn()));
    verifyNewBoolean(a("ippMarked"),            getPlanetProperty(pl, game::interface::ippMarked,            session, *r, *g, g->currentTurn()), false);
    verifyNewNull   (a("ippMinedD"),            getPlanetProperty(pl, game::interface::ippMinedD,            session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMinedM"),            getPlanetProperty(pl, game::interface::ippMinedM,            session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMinedN"),            getPlanetProperty(pl, game::interface::ippMinedN,            session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMinedStr"),          getPlanetProperty(pl, game::interface::ippMinedStr,          session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMinedT"),            getPlanetProperty(pl, game::interface::ippMinedT,            session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMineralTime"),       getPlanetProperty(pl, game::interface::ippMineralTime,       session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMines"),             getPlanetProperty(pl, game::interface::ippMines,             session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippMinesMax"),          getPlanetProperty(pl, game::interface::ippMinesMax,          session, *r, *g, g->currentTurn()));
    verifyNewInteger(a("ippMinesSpeed"),        getPlanetProperty(pl, game::interface::ippMinesSpeed,        session, *r, *g, g->currentTurn()), 5);
    verifyNewInteger(a("ippMinesWanted"),       getPlanetProperty(pl, game::interface::ippMinesWanted,       session, *r, *g, g->currentTurn()), 1000);
    verifyNewNull   (a("ippMoney"),             getPlanetProperty(pl, game::interface::ippMoney,             session, *r, *g, g->currentTurn()));
    verifyNewString (a("ippName"),              getPlanetProperty(pl, game::interface::ippName,              session, *r, *g, g->currentTurn()), "?"); // Probably not contractual
    verifyNewNull   (a("ippNativeChange"),      getPlanetProperty(pl, game::interface::ippNativeChange,      session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeChangeStr"),   getPlanetProperty(pl, game::interface::ippNativeChangeStr,   session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeGov"),         getPlanetProperty(pl, game::interface::ippNativeGov,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeGovCode"),     getPlanetProperty(pl, game::interface::ippNativeGovCode,     session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeHappy"),       getPlanetProperty(pl, game::interface::ippNativeHappy,       session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeHappyStr"),    getPlanetProperty(pl, game::interface::ippNativeHappyStr,    session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeRace"),        getPlanetProperty(pl, game::interface::ippNativeRace,        session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeRaceCode"),    getPlanetProperty(pl, game::interface::ippNativeRaceCode,    session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeTax"),         getPlanetProperty(pl, game::interface::ippNativeTax,         session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeTaxBase"),     getPlanetProperty(pl, game::interface::ippNativeTaxBase,     session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeTaxIncome"),   getPlanetProperty(pl, game::interface::ippNativeTaxIncome,   session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeTaxMax"),      getPlanetProperty(pl, game::interface::ippNativeTaxMax,      session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNativeTime"),        getPlanetProperty(pl, game::interface::ippNativeTime,        session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippNatives"),           getPlanetProperty(pl, game::interface::ippNatives,           session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippOrbitingEnemies"),   getPlanetProperty(pl, game::interface::ippOrbitingEnemies,   session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippOrbitingOwn"),       getPlanetProperty(pl, game::interface::ippOrbitingOwn,       session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippOrbitingShips"),     getPlanetProperty(pl, game::interface::ippOrbitingShips,     session, *r, *g, g->currentTurn()));
    verifyNewBoolean(a("ippPlayed"),            getPlanetProperty(pl, game::interface::ippPlayed,            session, *r, *g, g->currentTurn()), false);
    verifyNewNull   (a("ippSupplies"),          getPlanetProperty(pl, game::interface::ippSupplies,          session, *r, *g, g->currentTurn()));
    verifyNewBoolean(a("ippTask"),              getPlanetProperty(pl, game::interface::ippTask,              session, *r, *g, g->currentTurn()), false);
    verifyNewBoolean(a("ippTaskBase"),          getPlanetProperty(pl, game::interface::ippTaskBase,          session, *r, *g, g->currentTurn()), false);
    verifyNewNull   (a("ippTemp"),              getPlanetProperty(pl, game::interface::ippTemp,              session, *r, *g, g->currentTurn()));
    verifyNewNull   (a("ippTempStr"),           getPlanetProperty(pl, game::interface::ippTempStr,           session, *r, *g, g->currentTurn()));
    verifyNewString (a("ippTypeChar"),          getPlanetProperty(pl, game::interface::ippTypeChar,          session, *r, *g, g->currentTurn()), "P");
    verifyNewString (a("ippTypeStr"),           getPlanetProperty(pl, game::interface::ippTypeStr,           session, *r, *g, g->currentTurn()), "Planet");

    // Complex values
    {
        // ippEncodedMessage - long string, we don't want to check the entire content; always valid even if planet is mostly unknown
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippEncodedMessage, session, *r, *g, g->currentTurn()));
        String_t str;
        a.check("ippEncodedMessage", interpreter::checkStringArg(str, p.get()));
        a.checkDifferent("ippEncodedMessage", str, "OBJECT: Planet 42\n");
    }
    {
        // ippMessages - an iterable array, but null if nothing known
        verifyNewNull(a("ippMessages"), getPlanetProperty(pl, game::interface::ippMessages, session, *r, *g, g->currentTurn()));
    }
    {
        // ippReference - reference, always present
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippReference, session, *r, *g, g->currentTurn()));
        Reference ref;
        a.check("ippReference", game::interface::checkReferenceArg(ref, p.get()));
        a.checkEqual("ippReference", ref, Reference(Reference::Planet, 42));
    }
    {
        // ippScore - function (not iterable), always present
        std::auto_ptr<afl::data::Value> p(getPlanetProperty(pl, game::interface::ippScore, session, *r, *g, g->currentTurn()));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ippScore", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ippScore"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ippScore: dim", ix->getDimension(0), 0);
        AFL_CHECK_THROWS(a("ippScore: makeFirstContext"), ix->makeFirstContext(), interpreter::Error);

        // Score is null
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ippScore(Level)"), ix->get(args));
        }
    }

    // Writable properties
    // -- build goals/speeds can always be written --
    {
        afl::data::IntegerValue iv(4);
        setPlanetProperty(pl, game::interface::ippMinesSpeed, &iv, *r);
        a.checkEqual("ippMinesSpeed", pl.getAutobuildSpeed(game::MineBuilding), 4);
    }
    {
        afl::data::IntegerValue iv(140);
        setPlanetProperty(pl, game::interface::ippMinesWanted, &iv, *r);
        a.checkEqual("ippMinesWanted", pl.getAutobuildGoal(game::MineBuilding), 140);
    }
    {
        afl::data::IntegerValue iv(7);
        setPlanetProperty(pl, game::interface::ippFactoriesSpeed, &iv, *r);
        a.checkEqual("ippFactoriesSpeed", pl.getAutobuildSpeed(game::FactoryBuilding), 7);
    }
    {
        afl::data::IntegerValue iv(170);
        setPlanetProperty(pl, game::interface::ippFactoriesWanted, &iv, *r);
        a.checkEqual("ippFactoriesWanted", pl.getAutobuildGoal(game::FactoryBuilding), 170);
    }
    {
        afl::data::IntegerValue iv(6);
        setPlanetProperty(pl, game::interface::ippDefenseSpeed, &iv, *r);
        a.checkEqual("ippDefenseSpeed", pl.getAutobuildSpeed(game::DefenseBuilding), 6);
    }
    {
        afl::data::IntegerValue iv(160);
        setPlanetProperty(pl, game::interface::ippDefenseWanted, &iv, *r);
        a.checkEqual("ippDefenseWanted", pl.getAutobuildGoal(game::DefenseBuilding), 160);
    }
    {
        afl::data::IntegerValue iv(1);
        setPlanetProperty(pl, game::interface::ippBaseDefenseSpeed, &iv, *r);
        a.checkEqual("ippBaseDefenseSpeed", pl.getAutobuildSpeed(game::BaseDefenseBuilding), 1);
    }
    {
        afl::data::IntegerValue iv(110);
        setPlanetProperty(pl, game::interface::ippBaseDefenseWanted, &iv, *r);
        a.checkEqual("ippBaseDefenseWanted", pl.getAutobuildGoal(game::BaseDefenseBuilding), 110);
    }
    // -- cannot write others --
    {
        afl::data::IntegerValue iv(50);
        AFL_CHECK_THROWS(a("ippColonists: not assignable"), setPlanetProperty(pl, game::interface::ippColonistTax, &iv, *r), interpreter::Error);
    }
    {
        afl::data::StringValue sv("wvx");
        AFL_CHECK_THROWS(a("ippFCode: not assignable"), setPlanetProperty(pl, game::interface::ippFCode, &sv, *r), interpreter::Error);
    }

    {
        afl::data::IntegerValue iv(60);
        AFL_CHECK_THROWS(a("ippNativeTax: not assignable"), setPlanetProperty(pl, game::interface::ippNativeTax, &iv, *r), interpreter::Error);
    }
}
