/**
  *  \file test/game/interface/taskeditorcontexttest.cpp
  *  \brief Test for game::interface::TaskEditorContext
  */

#include "game/interface/taskeditorcontext.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    const int SHIP_X = 1000;
    const int SHIP_Y = 1000;
    const int PLANET_X = 1100;
    const int PLANET_Y = 1000;

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
        // Must specify a host version here; it is referenced by insertMovementCommand() -> isExactHyperjumpDistance2()
        env.session.setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr());
    }

    void addShipList(Environment& env)
    {
        afl::base::Ptr<game::spec::ShipList> shipList = new game::spec::ShipList();
        game::test::addTranswarp(*shipList);
        game::test::addOutrider(*shipList);
        env.session.setShipList(shipList);
    }

    void addGame(Environment& env)
    {
        afl::base::Ptr<game::Game> g = new game::Game();
        env.session.setGame(g);
    }

    void addShip(Environment& env, game::Id_t shipId, int player)
    {
        game::map::Ship& sh = *env.session.getGame()->currentTurn().universe().ships().create(shipId);
        game::map::ShipData sd;
        sd.owner                     = player;
        sd.friendlyCode              = "tep";
        sd.warpFactor                = 3;
        sd.waypointDX                = 0;
        sd.waypointDY                = 0;
        sd.x                         = SHIP_X;
        sd.y                         = SHIP_Y;
        sd.engineType                = game::test::TRANSWARP_ENGINE_ID;
        sd.hullType                  = game::test::OUTRIDER_HULL_ID;
        sd.beamType                  = 0;
        sd.numBeams                  = 0;
        sd.numBays                   = 0;
        sd.torpedoType               = 0;
        sd.ammo                      = 0;
        sd.numLaunchers              = 0;
        sd.mission                   = 3;
        sd.primaryEnemy              = 0;
        sd.missionTowParameter       = 0;
        sd.damage                    = 0;
        sd.crew                      = 10;
        sd.colonists                 = 0;
        sd.name                      = "Caroline";
        sd.neutronium                = 50;
        sd.tritanium                 = 0;
        sd.duranium                  = 0;
        sd.molybdenum                = 0;
        sd.supplies                  = 0;
        sd.missionInterceptParameter = 0;
        sd.money                     = 0;

        sh.addCurrentShipData(sd, game::PlayerSet_t(player));
        sh.internalCheck(game::PlayerSet_t(player), 10);
    }

    void addPlanet(Environment& env, game::Id_t planetId, int player)
    {
        game::map::Planet& pl = *env.session.getGame()->currentTurn().universe().planets().create(planetId);
        game::map::PlanetData pd;
        pd.owner             = player;
        pd.friendlyCode      = "pfc";
        pd.numMines          = 10;
        pd.numFactories      = 15;
        pd.numDefensePosts   = 17;
        pd.minedNeutronium   = 100;
        pd.minedTritanium    = 100;
        pd.minedDuranium     = 100;
        pd.minedMolybdenum   = 100;
        pd.colonistClans     = 70;
        pd.supplies          = 42;
        pd.money             = 1337;
        pd.colonistTax       = 1;
        pd.nativeTax         = 0;
        pd.colonistHappiness = 97;
        pd.nativeHappiness   = 100;
        pd.nativeGovernment  = 0;
        pd.nativeClans       = 0;
        pd.nativeRace        = 0;
        pd.temperature       = 50;
        pd.baseFlag          = 0;

        pl.setPosition(game::map::Point(PLANET_X, PLANET_Y));
        pl.setName("Marble");
        pl.addCurrentPlanetData(pd, game::PlayerSet_t(player));
        pl.internalCheck(env.session.getGame()->mapConfiguration(), game::PlayerSet_t(player), 10, env.tx, env.session.log());
    }

    afl::base::Ptr<interpreter::TaskEditor> prepareShipTask(afl::test::Assert a, Environment& env, game::Id_t shipId)
    {
        addRoot(env);
        addShipList(env);
        addGame(env);
        addShip(env, shipId, 7);

        afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(shipId, interpreter::Process::pkShipTask, true));
        a.checkNonNull("getAutoTaskEditor succeeds", edit.get());
        return edit;
    }

    void callMethod(Environment& env, interpreter::TaskEditor& edit, game::interface::TaskEditorMethod m, afl::data::Segment& seg)
    {
        interpreter::Arguments args(seg, 0, seg.size());
        callTaskEditorMethod(edit, m, env.session, args);
    }

    const String_t five_cmds[] = {"a", "b", "c", "d", "e"};
}

/** Test getTaskEditorProperty(), setTaskEditorProperty(). */
AFL_TEST("game.interface.TaskEditorContext:getTaskEditorProperty:null", a)
{
    // Environment
    Environment env;

    // Null editor
    afl::base::Ptr<interpreter::TaskEditor> edit;

    // Verify properties
    verifyNewNull(a("iteCursor"),                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session));
    verifyNewNull(a("itePC"),                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session));
    verifyNewNull(a("iteIsInSubroutine"),        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session));
    verifyNewNull(a("itePredictedCloakFuel"),    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session));
    verifyNewNull(a("itePredictedFCode"),        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session));
    verifyNewNull(a("itePredictedFuel"),         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session));
    verifyNewNull(a("itePredictedMission"),      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session));
    verifyNewNull(a("itePredictedMovementFuel"), getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session));
    verifyNewNull(a("itePredictedPositionX"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session));
    verifyNewNull(a("itePredictedPositionY"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session));
    verifyNewNull(a("itePredictedSpeed"),        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session));
    verifyNewNull(a("iteTypeStr"),               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session));
    verifyNewNull(a("iteTypeInt"),               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session));
    verifyNewNull(a("iteObjectId"),              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session));
}

/** Test getTaskEditorProperty(), setTaskEditorProperty() for ship task. */
AFL_TEST("game.interface.TaskEditorContext:getTaskEditorProperty:ship", a)
{
    const int SHIP_ID = 30;

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    addGame(env);
    addShip(env, SHIP_ID, 7);

    // A ship task
    afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(SHIP_ID, interpreter::Process::pkShipTask, true));
    a.checkNonNull("getAutoTaskEditor", edit.get());
    const String_t cmd[] = { "setspeed 8", "moveto 1050, 1000" };
    edit->addAsCurrent(cmd);

    // Verify properties
    verifyNewInteger(a("iteCursor"),                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session), 2);
    verifyNewInteger(a("itePC"),                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session), 0);
    verifyNewBoolean(a("iteIsInSubroutine"),        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session), false);
    verifyNewInteger(a("itePredictedCloakFuel"),    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session), 0);
    verifyNewString (a("itePredictedFCode"),        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session), "tep");
    verifyNewInteger(a("itePredictedFuel"),         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session), 44);
    verifyNewInteger(a("itePredictedMission"),      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session), 3);
    verifyNewInteger(a("itePredictedMovementFuel"), getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session), 6);
    verifyNewInteger(a("itePredictedPositionX"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session), 1050);
    verifyNewInteger(a("itePredictedPositionY"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session), 1000);
    verifyNewInteger(a("itePredictedSpeed"),        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session), 8);
    verifyNewString (a("iteTypeStr"),               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session), "ship");
    verifyNewInteger(a("iteTypeInt"),               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session), 1);
    verifyNewInteger(a("iteObjectId"),              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session), SHIP_ID);

    // Modify properties
    {
        // Setting cursor -> movement will no longer be predicted
        const afl::data::IntegerValue iv(1);
        setTaskEditorProperty(*edit, game::interface::iteCursor, &iv);
        verifyNewInteger(a("iteCursor after"),         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 1);
        verifyNewInteger(a("itePredictedFuel after"),  getTaskEditorProperty(edit, game::interface::itePredictedFuel,  env.session), 50);
        verifyNewInteger(a("itePredictedSpeed after"), getTaskEditorProperty(edit, game::interface::itePredictedSpeed, env.session), 8);
    }
    {
        // Setting PC -> speed change will no longer be predicted
        const afl::data::IntegerValue iv(1);
        setTaskEditorProperty(*edit, game::interface::itePC, &iv);
        verifyNewInteger(a("iteCursor after 2"),         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 1);
        verifyNewInteger(a("itePredictedFuel after 2"),  getTaskEditorProperty(edit, game::interface::itePredictedFuel,  env.session), 50);
        verifyNewInteger(a("itePredictedSpeed after 2"), getTaskEditorProperty(edit, game::interface::itePredictedSpeed, env.session), 3);
    }
    {
        // Error
        const afl::data::IntegerValue iv(1);
        AFL_CHECK_THROWS(a("set itePredictedSpeed"), setTaskEditorProperty(*edit, game::interface::itePredictedSpeed, &iv), interpreter::Error);
    }
}

/** Test getTaskEditorProperty(), setTaskEditorProperty() for planet task. */
AFL_TEST("game.interface.TaskEditorContext:getTaskEditorProperty:planet", a)
{
    const int PLANET_ID = 17;

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    addGame(env);
    addPlanet(env, PLANET_ID, 7);

    // A planet task
    afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(PLANET_ID, interpreter::Process::pkPlanetTask, true));
    a.checkNonNull("getAutoTaskEditor", edit.get());
    const String_t cmd[] = { "setfcode \"one\"", "setfcode \"two\"", "setfcode \"333\"" };
    edit->addAsCurrent(cmd);

    // Verify properties
    verifyNewInteger(a("iteCursor"),                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session), 3);
    verifyNewInteger(a("itePC"),                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session), 0);
    verifyNewBoolean(a("iteIsInSubroutine"),        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session), false);
    verifyNewNull   (a("itePredictedCloakFuel"),    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session));
    verifyNewString (a("itePredictedFCode"),        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session), "333");
    verifyNewNull   (a("itePredictedFuel"),         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session));
    verifyNewNull   (a("itePredictedMission"),      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session));
    verifyNewNull   (a("itePredictedMovementFuel"), getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session));
    verifyNewNull   (a("itePredictedPositionX"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session));
    verifyNewNull   (a("itePredictedPositionY"),    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session));
    verifyNewNull   (a("itePredictedSpeed"),        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session));
    verifyNewString (a("iteTypeStr"),               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session), "planet");
    verifyNewInteger(a("iteTypeInt"),               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session), 2);
    verifyNewInteger(a("iteObjectId"),              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session), PLANET_ID);

    // Modify properties
    {
        // Setting cursor
        const afl::data::IntegerValue iv(2);
        setTaskEditorProperty(*edit, game::interface::iteCursor, &iv);
        verifyNewInteger(a("iteCursor after"),         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 2);
        verifyNewString (a("itePredictedFCode after"), getTaskEditorProperty(edit, game::interface::itePredictedFCode, env.session), "two");
    }
    {
        // Setting PC to same as cursor -> fc will no longer be predicted
        const afl::data::IntegerValue iv(2);
        setTaskEditorProperty(*edit, game::interface::itePC, &iv);
        verifyNewInteger(a("iteCursor after 2"),         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 2);
        verifyNewString (a("itePredictedFCode after 2"), getTaskEditorProperty(edit, game::interface::itePredictedFCode, env.session), "pfc");
    }
    {
        // Error
        const afl::data::IntegerValue iv(1);
        AFL_CHECK_THROWS(a("set itePredictedSpeed"), setTaskEditorProperty(*edit, game::interface::itePredictedSpeed, &iv), interpreter::Error);
    }
}

/** Test "Lines" property (getTaskEditorProperty(iteLines)).
    Uses a ship task editor for testing. */
AFL_TEST("game.interface.TaskEditorContext:iteLines", a)
{
    const int SHIP_ID = 30;

    // Environment
    Environment env;
    addRoot(env);
    addShipList(env);
    addGame(env);
    addShip(env, SHIP_ID, 7);

    // A ship task
    afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(SHIP_ID, interpreter::Process::pkShipTask, true));
    a.checkNonNull("01. getAutoTaskEditor", edit.get());
    const String_t cmd[] = { "setspeed 8", "moveto 1050, 1000" };
    edit->addAsCurrent(cmd);

    // Lines property
    std::auto_ptr<afl::data::Value> linesValue(getTaskEditorProperty(edit, game::interface::iteLines, env.session));
    interpreter::IndexableValue* lines = dynamic_cast<interpreter::IndexableValue*>(linesValue.get());
    a.checkNonNull("11. iteLines", lines);
    interpreter::test::ValueVerifier verif(*lines, a("11. iteLines"));
    verif.verifyBasics();
    verif.verifyNotSerializable();
    a.checkEqual("12. isProcedureCall", lines->isProcedureCall(), false);
    a.checkEqual("13. getDimension 0", lines->getDimension(0), 1);
    a.checkEqual("14. getDimension 1", lines->getDimension(1), 2);
    AFL_CHECK_THROWS(a("15. makeFirstContext"), lines->makeFirstContext(), interpreter::Error);

    afl::data::StringValue sv("setmission 5");

    // Correct invocation of 'get'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString(a("Lines(0)"), lines->get(args), "setspeed 8");
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString(a("Lines(1)"), lines->get(args), "moveto 1050, 1000");
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull(a("Lines(Null)"), lines->get(args));
    }

    // Correct invocation of 'set'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_SUCCEEDS(a("21. set Lines(0)"), lines->set(args, &sv));
        a.checkEqual("22. result", (*edit)[0], "setmission 5");
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_SUCCEEDS(a("23. set Lines(null)"), lines->set(args, &sv));
    }

    // Assigning invalid command
    {
        afl::data::StringValue invalidSV("sub");
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("31. set Lines(0) with invalid"), lines->set(args, &invalidSV), interpreter::Error);
        a.checkEqual("32. result", (*edit)[0], "setmission 5");  // unchanged
    }

    // Range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("41. get Lines() out-of-range"), lines->get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("42. set Lines() out-of-range"), lines->set(args, &sv), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        AFL_CHECK_THROWS(a("51. get Lines() type error"), lines->get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("52. set Lines() type error"), lines->set(args, &sv), interpreter::Error);
    }

    // Arity
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        AFL_CHECK_THROWS(a("61. get Lines() arity error"), lines->get(args), interpreter::Error);
        AFL_CHECK_THROWS(a("62. set Lines() arity error"), lines->set(args, &sv), interpreter::Error);
    }
}

/*
 *  insertMovementCommand
 */

// Normal case
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:normal", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    addPlanet(env, 50, 9);
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X, PLANET_Y), 0, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("line 0", (*edit)[0], "MoveTo 1100, 1000   % Marble (#50)");
}

// With auto-warp
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:auto-warp", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    addPlanet(env, 50, 9);
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X-1, PLANET_Y), game::interface::imc_SetSpeed, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 2U);
    a.checkEqual("line 0", (*edit)[0], "SetSpeed 7");
    a.checkEqual("line 1", (*edit)[1], "MoveTo 1099, 1000   % near Marble (#50)");
}

// Duplicate
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:duplicate", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X, SHIP_Y), 0, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 0U);
}

// Duplicate, force addition of command
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:duplicate-forced", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X, SHIP_Y), game::interface::imc_AcceptDuplicate, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("line 0", (*edit)[0], "MoveTo 1000, 1000");
}

// Hyperjump
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:hyperjump", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    game::map::Ship& sh = *env.session.getGame()->currentTurn().universe().ships().get(10);
    sh.addShipSpecialFunction(env.session.getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Hyperdrive));
    sh.setWarpFactor(0);
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X + 350, SHIP_Y), game::interface::imc_SetSpeed, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 3U);
    a.checkEqual("line 0", (*edit)[0], "SetSpeed 9");
    a.checkEqual("line 1", (*edit)[1], "SetFCode \"HYP\"   % hyperjump");
    a.checkEqual("line 2", (*edit)[2], "MoveTo 1350, 1000");
}

// Cancel
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:cancel-hyperjump", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    game::map::Ship& sh = *env.session.getGame()->currentTurn().universe().ships().get(10);
    sh.addShipSpecialFunction(env.session.getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Hyperdrive));
    sh.setWarpFactor(0);
    sh.setFriendlyCode(String_t("HYP"));
    game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X + 50, SHIP_Y), game::interface::imc_SetSpeed, env.session);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 3U);
    a.checkEqual("line 0", (*edit)[0].substr(0, 9), "SetFCode ");
    a.checkEqual("line 1", (*edit)[1], "SetSpeed 8");
    a.checkEqual("line 2", (*edit)[2], "MoveTo 1050, 1000");
}

// Error case: invalid verb
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:invalid-verb", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    addPlanet(env, 50, 9);
    AFL_CHECK_THROWS(a, game::interface::insertMovementCommand(*edit, "Sub", game::map::Point(PLANET_X, PLANET_Y), 0, env.session), interpreter::Error);
}

// Error case: wrong task
AFL_TEST("game.interface.TaskEditorContext:insertMovementCommand:bad-task", a)
{
    Environment env;
    addRoot(env);
    addShipList(env);
    addGame(env);
    addPlanet(env, 44, 7);

    afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(44, interpreter::Process::pkPlanetTask, true));
    AFL_CHECK_THROWS(a, game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X, PLANET_Y), 0, env.session), interpreter::Error);
}

/*
 *  callTaskEditorMethod(itmAdd)
 */


// Add single command
AFL_TEST("game.interface.TaskEditorContext:itmAdd:single", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("SetSpeed 5");
    callMethod(env, *edit, game::interface::itmAdd, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 1U);
    a.checkEqual("line 0", (*edit)[0], "SetSpeed 5");
}

// Add multiple commands, mixed types
AFL_TEST("game.interface.TaskEditorContext:itmAdd:mixed", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->content().pushBackString("a1");
    ad->content().pushBackNew(0);
    ad->content().pushBackString("a2");

    afl::data::Segment seg;
    seg.pushBackString("before");
    seg.pushBackNew(new interpreter::ArrayValue(ad));
    seg.pushBackString("after");
    callMethod(env, *edit, game::interface::itmAdd, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 4U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 4U);
    a.checkEqual("line 0", (*edit)[0], "before");
    a.checkEqual("line 1", (*edit)[1], "a1");
    a.checkEqual("line 2", (*edit)[2], "a2");
    a.checkEqual("line 3", (*edit)[3], "after");
}

// Add multiple commands by using Lines()
AFL_TEST("game.interface.TaskEditorContext:itmAdd:lines", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    String_t cmds[] = {"a1","a2"};
    edit->addAsCurrent(cmds);

    afl::data::Segment seg;
    seg.pushBackNew(getTaskEditorProperty(edit, game::interface::iteLines, env.session));
    callMethod(env, *edit, game::interface::itmAdd, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 4U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 4U);
    a.checkEqual("line 0", (*edit)[0], "a1");
    a.checkEqual("line 1", (*edit)[1], "a2");
    a.checkEqual("line 2", (*edit)[2], "a1");
    a.checkEqual("line 3", (*edit)[3], "a2");
}

// Error case: multi-dimensional array
AFL_TEST("game.interface.TaskEditorContext:itmAdd:error:multi-dimensional", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
    ad->addDimension(3);
    ad->addDimension(1);
    ad->content().pushBackString("a1");
    ad->content().pushBackNew(0);
    ad->content().pushBackString("a2");

    afl::data::Segment seg;
    seg.pushBackNew(new interpreter::ArrayValue(ad));
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
}

// Error case: arity error
AFL_TEST("game.interface.TaskEditorContext:itmAdd:error:arity", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
}

// Error case: disallowed verb
AFL_TEST("game.interface.TaskEditorContext:itmAdd:error:invalid-verb", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("Sub foo");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
}

/*
 *  callTaskEditorMethod(itmAddMovement)
 */

// Standard case
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:normal", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackInteger(SHIP_X);
    seg.pushBackInteger(SHIP_Y + 30);
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 1U);
    a.checkEqual("line 0", (*edit)[0], "MoveTo 1000, 1030");
}

// With speed
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:speed", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackInteger(SHIP_X);
    seg.pushBackInteger(SHIP_Y + 30);
    seg.pushBackString("s");
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 2U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 2U);
    a.checkEqual("line 0", (*edit)[0], "SetSpeed 6");
    a.checkEqual("line 1", (*edit)[1], "MoveTo 1000, 1030");
}

// Duplicate - no-op
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:duplicate", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackInteger(SHIP_X);
    seg.pushBackInteger(SHIP_Y);
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 0U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 0U);
}

// Force duplicate
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:duplicate-forced", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackInteger(SHIP_X);
    seg.pushBackInteger(SHIP_Y);
    seg.pushBackString("d");
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("getPC", edit->getPC(), 0U);
    a.checkEqual("getCursor", edit->getCursor(), 1U);
    a.checkEqual("line 0", (*edit)[0], "MoveTo 1000, 1000");
}

// Null verb
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:null-verb", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackInteger(SHIP_X);
    seg.pushBackInteger(SHIP_Y + 30);
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 0U);
}

// Null X
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:null-x", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackNew(0);
    seg.pushBackInteger(SHIP_Y + 30);
    callMethod(env, *edit, game::interface::itmAddMovement, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 0U);
}

// Error: arity
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:error:arity", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmAddMovement, seg), interpreter::Error);
}

// Error: type
AFL_TEST("game.interface.TaskEditorContext:itmAddMovement:error:type", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackString("MoveTo");
    seg.pushBackInteger(SHIP_X);
    seg.pushBackString("Y");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmAddMovement, seg), interpreter::Error);
}

/*
 *  callTaskEditorMethod(itmConfirmMessage)
 */

// Normal case
AFL_TEST("game.interface.TaskEditorContext:itmConfirmMessage:normal", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    game::interface::NotificationStore::Message* msg = env.session.notifications().addMessage(edit->process().getProcessId(), "head", "body", game::Reference(game::Reference::Ship, 10));
    a.check("01. isMessageConfirmed", !env.session.notifications().isMessageConfirmed(msg));

    afl::data::Segment seg;
    callMethod(env, *edit, game::interface::itmConfirmMessage, seg);

    a.check("11. isMessageConfirmed", env.session.notifications().isMessageConfirmed(msg));
}

// Call without existing message is a no-op
AFL_TEST("game.interface.TaskEditorContext:itmConfirmMessage:no-op", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    AFL_CHECK_SUCCEEDS(a, callMethod(env, *edit, game::interface::itmConfirmMessage, seg));
}

// Error: arity
AFL_TEST("game.interface.TaskEditorContext:itmConfirmMessage:error:arity", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmConfirmMessage, seg), interpreter::Error);
}

/*
 *  callTaskEditorMethod(itmInsert)
 */

// Add 'next'
AFL_TEST("game.interface.TaskEditorContext:itmInsert:next", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackString("next");
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 7U);
    a.checkEqual("getPC", edit->getPC(), 1U);
    a.checkEqual("getCursor", edit->getCursor(), 3U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "x");
    a.checkEqual("line 2", (*edit)[2], "y");
    a.checkEqual("line 3", (*edit)[3], "b");
    a.checkEqual("line 4", (*edit)[4], "c");
    a.checkEqual("line 5", (*edit)[5], "d");
    a.checkEqual("line 6", (*edit)[6], "e");
}

// Add 'end'
AFL_TEST("game.interface.TaskEditorContext:itmInsert:end", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackString("end");
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 7U);
    a.checkEqual("getPC", edit->getPC(), 1U);
    a.checkEqual("getCursor", edit->getCursor(), 7U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "b");
    a.checkEqual("line 2", (*edit)[2], "c");
    a.checkEqual("line 3", (*edit)[3], "d");
    a.checkEqual("line 4", (*edit)[4], "e");
    a.checkEqual("line 5", (*edit)[5], "x");
    a.checkEqual("line 6", (*edit)[6], "y");
}

// Add at beginning
AFL_TEST("game.interface.TaskEditorContext:itmInsert:beginning", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 7U);
    a.checkEqual("getPC", edit->getPC(), 3U);
    a.checkEqual("getCursor", edit->getCursor(), 4U);
    a.checkEqual("line 0", (*edit)[0], "x");
    a.checkEqual("line 1", (*edit)[1], "y");
    a.checkEqual("line 2", (*edit)[2], "a");
    a.checkEqual("line 3", (*edit)[3], "b");
    a.checkEqual("line 4", (*edit)[4], "c");
    a.checkEqual("line 5", (*edit)[5], "d");
    a.checkEqual("line 6", (*edit)[6], "e");
}

// Add at specific place (PC)
AFL_TEST("game.interface.TaskEditorContext:itmInsert:at-pc", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 7U);
    a.checkEqual("getPC", edit->getPC(), 3U);
    a.checkEqual("getCursor", edit->getCursor(), 4U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "x");
    a.checkEqual("line 2", (*edit)[2], "y");
    a.checkEqual("line 3", (*edit)[3], "b");
    a.checkEqual("line 4", (*edit)[4], "c");
    a.checkEqual("line 5", (*edit)[5], "d");
    a.checkEqual("line 6", (*edit)[6], "e");
}

// Add at specific place (end)
AFL_TEST("game.interface.TaskEditorContext:itmInsert:at-end", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 7U);
    a.checkEqual("getPC", edit->getPC(), 1U);
    a.checkEqual("getCursor", edit->getCursor(), 2U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "b");
    a.checkEqual("line 2", (*edit)[2], "c");
    a.checkEqual("line 3", (*edit)[3], "d");
    a.checkEqual("line 4", (*edit)[4], "e");
    a.checkEqual("line 5", (*edit)[5], "x");
    a.checkEqual("line 6", (*edit)[6], "y");
}

// Null position
AFL_TEST("game.interface.TaskEditorContext:itmInsert:null", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    seg.pushBackString("x");
    seg.pushBackString("y");
    callMethod(env, *edit, game::interface::itmInsert, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Position range error
AFL_TEST("game.interface.TaskEditorContext:itmInsert:error:range", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(6);
    seg.pushBackString("x");
    seg.pushBackString("y");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Position range error
AFL_TEST("game.interface.TaskEditorContext:itmInsert:error:range2", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(-1);
    seg.pushBackString("x");
    seg.pushBackString("y");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Position range error
AFL_TEST("game.interface.TaskEditorContext:itmInsert:error:range3", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackString("what");
    seg.pushBackString("x");
    seg.pushBackString("y");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Arity error
AFL_TEST("game.interface.TaskEditorContext:itmInsert:error:arity", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);
    edit->setCursor(2);
    edit->setPC(1);

    afl::data::Segment seg;
    seg.pushBackInteger(0);
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

/*
 *  callTaskEditorMethod(itmDelete)
 */


// Delete one
AFL_TEST("game.interface.TaskEditorContext:itmDelete:delete-one", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackInteger(2);
    callMethod(env, *edit, game::interface::itmDelete, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 4U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "b");
    a.checkEqual("line 2", (*edit)[2], "d");
    a.checkEqual("line 3", (*edit)[3], "e");
}

// Delete multiple
AFL_TEST("game.interface.TaskEditorContext:itmDelete:delete-range", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackInteger(3);
    callMethod(env, *edit, game::interface::itmDelete, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 2U);
    a.checkEqual("line 0", (*edit)[0], "a");
    a.checkEqual("line 1", (*edit)[1], "e");
}

// Delete to end
AFL_TEST("game.interface.TaskEditorContext:itmDelete:to-end", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackInteger(1);
    seg.pushBackInteger(99999);
    callMethod(env, *edit, game::interface::itmDelete, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 1U);
    a.checkEqual("line 0", (*edit)[0], "a");
}

// Delete at end
AFL_TEST("game.interface.TaskEditorContext:itmDelete:at-end", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackInteger(5);
    callMethod(env, *edit, game::interface::itmDelete, seg);

    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Null position
AFL_TEST("game.interface.TaskEditorContext:itmDelete:null", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackNew(0);
    AFL_CHECK_SUCCEEDS(a, callMethod(env, *edit, game::interface::itmDelete, seg));
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Range error
AFL_TEST("game.interface.TaskEditorContext:itmDelete:error:range", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackInteger(6);
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Type error
AFL_TEST("game.interface.TaskEditorContext:itmDelete:error:type", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    seg.pushBackString("X");
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

// Arity error
AFL_TEST("game.interface.TaskEditorContext:itmDelete:error:arity", a)
{
    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, 10);
    edit->addAsCurrent(five_cmds);

    afl::data::Segment seg;
    AFL_CHECK_THROWS(a, callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
    a.checkEqual("getNumInstructions", edit->getNumInstructions(), 5U);
}

/*
 *  TaskEditorContext
 */

AFL_TEST("game.interface.TaskEditorContext:context", a)
{
    const int SHIP_ID = 42;
    const String_t cmds[] = {"SetSpeed 5", "SetEnemy 3"};

    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(a, env, SHIP_ID);
    edit->addAsCurrent(cmds);
    game::interface::TaskEditorContext testee(edit, env.session);

    // Verify general properties
    interpreter::test::ContextVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    a.checkNull("01. getObject", testee.getObject());
    a.check("02. next", !testee.next());

    // Verify specific properties
    verif.verifyInteger("ID", SHIP_ID);
    verif.verifyString("TYPE", "ship");
    verif.verifyInteger("CURRENT", 0);

    // Assignment
    verif.setIntegerValue("CURRENT", 1);
    a.checkEqual("11. getPC", edit->getPC(), 1U);

    AFL_CHECK_THROWS(a("21. set DELETE"), verif.setIntegerValue("DELETE", 1), interpreter::Error);

    // Retrieve and call a command
    {
        std::auto_ptr<afl::data::Value> v(verif.getValue("DELETE"));
        interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(v.get());
        a.checkNonNull("31. DELETE", cv);

        interpreter::test::ValueVerifier vv(*cv, a("32. DELETE"));
        vv.verifyBasics();
        vv.verifyNotSerializable();
        a.check("41. isProcedureCall", cv->isProcedureCall());
        a.checkEqual("42. getDimension", cv->getDimension(0), 0);

        interpreter::Process proc(env.session.world(), "tester", 777);
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        cv->call(proc, seg, false);

        a.checkEqual("51. getNumInstructions", edit->getNumInstructions(), 1U);
        a.checkEqual("52. line 0", (*edit)[0], "SetEnemy 3");
    }
}

// Success case
AFL_TEST("game.interface.TaskEditorContext:create:success", a)
{
    const int SHIP_ID = 30;
    Environment env;
    addRoot(env);
    addShipList(env);
    addGame(env);
    addShip(env, SHIP_ID, 7);

    std::auto_ptr<game::interface::TaskEditorContext> ctx(game::interface::TaskEditorContext::create(env.session, interpreter::Process::pkShipTask, SHIP_ID));
    a.checkNonNull("create", ctx.get());

    interpreter::test::ContextVerifier(*ctx, a).verifyInteger("ID", SHIP_ID);
}

// Failure case
AFL_TEST("game.interface.TaskEditorContext:create:error", a)
{
    Environment env;

    std::auto_ptr<game::interface::TaskEditorContext> ctx(game::interface::TaskEditorContext::create(env.session, interpreter::Process::pkShipTask, 99));
    a.checkNull("create", ctx.get());
}
