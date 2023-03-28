/**
  *  \file u/t_game_interface_taskeditorcontext.cpp
  *  \brief Test for game::interface::TaskEditorContext
  */

#include "game/interface/taskeditorcontext.hpp"

#include "t_game_interface.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/segment.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/map/ship.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/valueverifier.hpp"
#include "interpreter/arrayvalue.hpp"
#include "interpreter/test/contextverifier.hpp"

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

    afl::base::Ptr<interpreter::TaskEditor> prepareShipTask(Environment& env, game::Id_t shipId)
    {
        addRoot(env);
        addShipList(env);
        addGame(env);
        addShip(env, shipId, 7);

        afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(shipId, interpreter::Process::pkShipTask, true));
        TS_ASSERT(edit.get() != 0);
        return edit;
    }

    void callMethod(Environment& env, interpreter::TaskEditor& edit, game::interface::TaskEditorMethod m, afl::data::Segment& seg)
    {
        interpreter::Arguments args(seg, 0, seg.size());
        callTaskEditorMethod(edit, m, env.session, args);
    }
}

/** Test getTaskEditorProperty(), setTaskEditorProperty(). */
void
TestGameInterfaceTaskEditorContext::testTaskEditorPropertyNull()
{
    // Environment
    Environment env;

    // Null editor
    afl::base::Ptr<interpreter::TaskEditor> edit;

    // Verify properties
    verifyNewNull("iteCursor",                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session));
    verifyNewNull("itePC",                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session));
    verifyNewNull("iteIsInSubroutine",        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session));
    verifyNewNull("itePredictedCloakFuel",    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session));
    verifyNewNull("itePredictedFCode",        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session));
    verifyNewNull("itePredictedFuel",         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session));
    verifyNewNull("itePredictedMission",      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session));
    verifyNewNull("itePredictedMovementFuel", getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session));
    verifyNewNull("itePredictedPositionX",    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session));
    verifyNewNull("itePredictedPositionY",    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session));
    verifyNewNull("itePredictedSpeed",        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session));
    verifyNewNull("iteTypeStr",               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session));
    verifyNewNull("iteTypeInt",               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session));
    verifyNewNull("iteObjectId",              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session));
}

/** Test getTaskEditorProperty(), setTaskEditorProperty() for ship task. */
void
TestGameInterfaceTaskEditorContext::testTaskEditorPropertyShip()
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
    TS_ASSERT(edit.get() != 0);
    const String_t cmd[] = { "setspeed 8", "moveto 1050, 1000" };
    edit->addAsCurrent(cmd);

    // Verify properties
    verifyNewInteger("iteCursor",                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session), 2);
    verifyNewInteger("itePC",                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session), 0);
    verifyNewBoolean("iteIsInSubroutine",        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session), false);
    verifyNewInteger("itePredictedCloakFuel",    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session), 0);
    verifyNewString ("itePredictedFCode",        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session), "tep");
    verifyNewInteger("itePredictedFuel",         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session), 44);
    verifyNewInteger("itePredictedMission",      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session), 3);
    verifyNewInteger("itePredictedMovementFuel", getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session), 6);
    verifyNewInteger("itePredictedPositionX",    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session), 1050);
    verifyNewInteger("itePredictedPositionY",    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session), 1000);
    verifyNewInteger("itePredictedSpeed",        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session), 8);
    verifyNewString ("iteTypeStr",               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session), "ship");
    verifyNewInteger("iteTypeInt",               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session), 1);
    verifyNewInteger("iteObjectId",              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session), SHIP_ID);

    // Modify properties
    {
        // Setting cursor -> movement will no longer be predicted
        const afl::data::IntegerValue iv(1);
        setTaskEditorProperty(*edit, game::interface::iteCursor, &iv);
        verifyNewInteger("iteCursor after",         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 1);
        verifyNewInteger("itePredictedFuel after",  getTaskEditorProperty(edit, game::interface::itePredictedFuel,  env.session), 50);
        verifyNewInteger("itePredictedSpeed after", getTaskEditorProperty(edit, game::interface::itePredictedSpeed, env.session), 8);
    }
    {
        // Setting PC -> speed change will no longer be predicted
        const afl::data::IntegerValue iv(1);
        setTaskEditorProperty(*edit, game::interface::itePC, &iv);
        verifyNewInteger("iteCursor after 2",         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 1);
        verifyNewInteger("itePredictedFuel after 2",  getTaskEditorProperty(edit, game::interface::itePredictedFuel,  env.session), 50);
        verifyNewInteger("itePredictedSpeed after 2", getTaskEditorProperty(edit, game::interface::itePredictedSpeed, env.session), 3);
    }
    {
        // Error
        const afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS(setTaskEditorProperty(*edit, game::interface::itePredictedSpeed, &iv), interpreter::Error);
    }
}

/** Test getTaskEditorProperty(), setTaskEditorProperty() for planet task. */
void
TestGameInterfaceTaskEditorContext::testTaskEditorPropertyPlanet()
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
    TS_ASSERT(edit.get() != 0);
    const String_t cmd[] = { "setfcode \"one\"", "setfcode \"two\"", "setfcode \"333\"" };
    edit->addAsCurrent(cmd);

    // Verify properties
    verifyNewInteger("iteCursor",                getTaskEditorProperty(edit, game::interface::iteCursor,                env.session), 3);
    verifyNewInteger("itePC",                    getTaskEditorProperty(edit, game::interface::itePC,                    env.session), 0);
    verifyNewBoolean("iteIsInSubroutine",        getTaskEditorProperty(edit, game::interface::iteIsInSubroutine,        env.session), false);
    verifyNewNull   ("itePredictedCloakFuel",    getTaskEditorProperty(edit, game::interface::itePredictedCloakFuel,    env.session));
    verifyNewString ("itePredictedFCode",        getTaskEditorProperty(edit, game::interface::itePredictedFCode,        env.session), "333");
    verifyNewNull   ("itePredictedFuel",         getTaskEditorProperty(edit, game::interface::itePredictedFuel,         env.session));
    verifyNewNull   ("itePredictedMission",      getTaskEditorProperty(edit, game::interface::itePredictedMission,      env.session));
    verifyNewNull   ("itePredictedMovementFuel", getTaskEditorProperty(edit, game::interface::itePredictedMovementFuel, env.session));
    verifyNewNull   ("itePredictedPositionX",    getTaskEditorProperty(edit, game::interface::itePredictedPositionX,    env.session));
    verifyNewNull   ("itePredictedPositionY",    getTaskEditorProperty(edit, game::interface::itePredictedPositionY,    env.session));
    verifyNewNull   ("itePredictedSpeed",        getTaskEditorProperty(edit, game::interface::itePredictedSpeed,        env.session));
    verifyNewString ("iteTypeStr",               getTaskEditorProperty(edit, game::interface::iteTypeStr,               env.session), "planet");
    verifyNewInteger("iteTypeInt",               getTaskEditorProperty(edit, game::interface::iteTypeInt,               env.session), 2);
    verifyNewInteger("iteObjectId",              getTaskEditorProperty(edit, game::interface::iteObjectId,              env.session), PLANET_ID);

    // Modify properties
    {
        // Setting cursor
        const afl::data::IntegerValue iv(2);
        setTaskEditorProperty(*edit, game::interface::iteCursor, &iv);
        verifyNewInteger("iteCursor after",         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 2);
        verifyNewString ("itePredictedFCode after", getTaskEditorProperty(edit, game::interface::itePredictedFCode, env.session), "two");
    }
    {
        // Setting PC to same as cursor -> fc will no longer be predicted
        const afl::data::IntegerValue iv(2);
        setTaskEditorProperty(*edit, game::interface::itePC, &iv);
        verifyNewInteger("iteCursor after 2",         getTaskEditorProperty(edit, game::interface::iteCursor,         env.session), 2);
        verifyNewString ("itePredictedFCode after 2", getTaskEditorProperty(edit, game::interface::itePredictedFCode, env.session), "pfc");
    }
    {
        // Error
        const afl::data::IntegerValue iv(1);
        TS_ASSERT_THROWS(setTaskEditorProperty(*edit, game::interface::itePredictedSpeed, &iv), interpreter::Error);
    }
}

/** Test "Lines" property (getTaskEditorProperty(iteLines)).
    Uses a ship task editor for testing. */
void
TestGameInterfaceTaskEditorContext::testTaskEditorLinesProperty()
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
    TS_ASSERT(edit.get() != 0);
    const String_t cmd[] = { "setspeed 8", "moveto 1050, 1000" };
    edit->addAsCurrent(cmd);

    // Lines property
    std::auto_ptr<afl::data::Value> linesValue(getTaskEditorProperty(edit, game::interface::iteLines, env.session));
    interpreter::IndexableValue* lines = dynamic_cast<interpreter::IndexableValue*>(linesValue.get());
    TS_ASSERT(lines != 0);
    interpreter::test::ValueVerifier verif(*lines, "iteLines");
    verif.verifyBasics();
    verif.verifyNotSerializable();
    TS_ASSERT_EQUALS(lines->isProcedureCall(), false);
    TS_ASSERT_EQUALS(lines->getDimension(0), 1);
    TS_ASSERT_EQUALS(lines->getDimension(1), 2);
    TS_ASSERT_THROWS(lines->makeFirstContext(), interpreter::Error);

    afl::data::StringValue sv("setmission 5");

    // Correct invocation of 'get'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("Lines(0)", lines->get(args), "setspeed 8");
    }
    {
        afl::data::Segment seg;
        seg.pushBackInteger(1);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewString("Lines(1)", lines->get(args), "moveto 1050, 1000");
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        verifyNewNull("Lines(Null)", lines->get(args));
    }

    // Correct invocation of 'set'
    {
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(lines->set(args, &sv));
        TS_ASSERT_EQUALS((*edit)[0], "setmission 5");
    }
    {
        afl::data::Segment seg;
        seg.pushBackNew(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS_NOTHING(lines->set(args, &sv));
    }

    // Assigning invalid command
    {
        afl::data::StringValue invalidSV("sub");
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(lines->set(args, &invalidSV), interpreter::Error);
        TS_ASSERT_EQUALS((*edit)[0], "setmission 5");  // unchanged
    }

    // Range error
    {
        afl::data::Segment seg;
        seg.pushBackInteger(2);
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(lines->get(args), interpreter::Error);
        TS_ASSERT_THROWS(lines->set(args, &sv), interpreter::Error);
    }

    // Type error
    {
        afl::data::Segment seg;
        seg.pushBackString("X");
        interpreter::Arguments args(seg, 0, 1);
        TS_ASSERT_THROWS(lines->get(args), interpreter::Error);
        TS_ASSERT_THROWS(lines->set(args, &sv), interpreter::Error);
    }

    // Arity
    {
        afl::data::Segment seg;
        interpreter::Arguments args(seg, 0, 0);
        TS_ASSERT_THROWS(lines->get(args), interpreter::Error);
        TS_ASSERT_THROWS(lines->set(args, &sv), interpreter::Error);
    }
}

/** Test insertMovementCommand(). */
void
TestGameInterfaceTaskEditorContext::testInsertMovementCommand()
{
    // Normal case
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        addPlanet(env, 50, 9);
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X, PLANET_Y), 0, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "MoveTo 1100, 1000   % Marble (#50)");
    }

    // With auto-warp
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        addPlanet(env, 50, 9);
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X-1, PLANET_Y), game::interface::imc_SetSpeed, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 2U);
        TS_ASSERT_EQUALS((*edit)[0], "SetSpeed 7");
        TS_ASSERT_EQUALS((*edit)[1], "MoveTo 1099, 1000   % near Marble (#50)");
    }

    // Duplicate
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X, SHIP_Y), 0, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 0U);
    }

    // Duplicate, force addition of command
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X, SHIP_Y), game::interface::imc_AcceptDuplicate, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "MoveTo 1000, 1000");
    }

    // Hyperjump
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        game::map::Ship& sh = *env.session.getGame()->currentTurn().universe().ships().get(10);
        sh.addShipSpecialFunction(env.session.getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Hyperdrive));
        sh.setWarpFactor(0);
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X + 350, SHIP_Y), game::interface::imc_SetSpeed, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 3U);
        TS_ASSERT_EQUALS((*edit)[0], "SetSpeed 2");
        TS_ASSERT_EQUALS((*edit)[1], "SetFCode \"HYP\"   % hyperjump");
        TS_ASSERT_EQUALS((*edit)[2], "MoveTo 1350, 1000");
    }

    // Cancel
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        game::map::Ship& sh = *env.session.getGame()->currentTurn().universe().ships().get(10);
        sh.addShipSpecialFunction(env.session.getShipList()->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Hyperdrive));
        sh.setWarpFactor(0);
        sh.setFriendlyCode(String_t("HYP"));
        game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(SHIP_X + 50, SHIP_Y), game::interface::imc_SetSpeed, env.session);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 3U);
        TS_ASSERT_EQUALS((*edit)[0].substr(0, 9), "SetFCode ");
        TS_ASSERT_EQUALS((*edit)[1], "SetSpeed 8");
        TS_ASSERT_EQUALS((*edit)[2], "MoveTo 1050, 1000");
    }

    // Error case: invalid verb
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        addPlanet(env, 50, 9);
        TS_ASSERT_THROWS(game::interface::insertMovementCommand(*edit, "Sub", game::map::Point(PLANET_X, PLANET_Y), 0, env.session), interpreter::Error);
    }

    // Error case: wrong task
    {
        Environment env;
        addRoot(env);
        addShipList(env);
        addGame(env);
        addPlanet(env, 44, 7);

        afl::base::Ptr<interpreter::TaskEditor> edit(env.session.getAutoTaskEditor(44, interpreter::Process::pkPlanetTask, true));
        TS_ASSERT_THROWS(game::interface::insertMovementCommand(*edit, "MoveTo", game::map::Point(PLANET_X, PLANET_Y), 0, env.session), interpreter::Error);
    }
}

/** Test callTaskEditorMethod(itmAdd). */
void
TestGameInterfaceTaskEditorContext::testCommandAdd()
{
    // Add single command
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("SetSpeed 5");
        callMethod(env, *edit, game::interface::itmAdd, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "SetSpeed 5");
    }

    // Add multiple commands, mixed types
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

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

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 4U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 4U);
        TS_ASSERT_EQUALS((*edit)[0], "before");
        TS_ASSERT_EQUALS((*edit)[1], "a1");
        TS_ASSERT_EQUALS((*edit)[2], "a2");
        TS_ASSERT_EQUALS((*edit)[3], "after");
    }

    // Add multiple commands by using Lines()
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        String_t cmds[] = {"a1","a2"};
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackNew(getTaskEditorProperty(edit, game::interface::iteLines, env.session));
        callMethod(env, *edit, game::interface::itmAdd, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 4U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 4U);
        TS_ASSERT_EQUALS((*edit)[0], "a1");
        TS_ASSERT_EQUALS((*edit)[1], "a2");
        TS_ASSERT_EQUALS((*edit)[2], "a1");
        TS_ASSERT_EQUALS((*edit)[3], "a2");
    }

    // Error case: multi-dimensional array
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::base::Ref<interpreter::ArrayData> ad = *new interpreter::ArrayData();
        ad->addDimension(3);
        ad->addDimension(1);
        ad->content().pushBackString("a1");
        ad->content().pushBackNew(0);
        ad->content().pushBackString("a2");

        afl::data::Segment seg;
        seg.pushBackNew(new interpreter::ArrayValue(ad));
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
    }

    // Error case: arity error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
    }

    // Error case: disallowed verb
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("Sub foo");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmAdd, seg), interpreter::Error);
    }
}

/** Test callTaskEditorMethod(itmAddMovement). */
void
TestGameInterfaceTaskEditorContext::testCommandAddMovement()
{
    // Standard case
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackInteger(SHIP_X);
        seg.pushBackInteger(SHIP_Y + 30);
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "MoveTo 1000, 1030");
    }

    // With speed
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackInteger(SHIP_X);
        seg.pushBackInteger(SHIP_Y + 30);
        seg.pushBackString("s");
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 2U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 2U);
        TS_ASSERT_EQUALS((*edit)[0], "SetSpeed 6");
        TS_ASSERT_EQUALS((*edit)[1], "MoveTo 1000, 1030");
    }

    // Duplicate - no-op
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackInteger(SHIP_X);
        seg.pushBackInteger(SHIP_Y);
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 0U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 0U);
    }

    // Force duplicate
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackInteger(SHIP_X);
        seg.pushBackInteger(SHIP_Y);
        seg.pushBackString("d");
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS(edit->getPC(), 0U);
        TS_ASSERT_EQUALS(edit->getCursor(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "MoveTo 1000, 1000");
    }

    // Null verb
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackInteger(SHIP_X);
        seg.pushBackInteger(SHIP_Y + 30);
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 0U);
    }

    // Null X
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackNew(0);
        seg.pushBackInteger(SHIP_Y + 30);
        callMethod(env, *edit, game::interface::itmAddMovement, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 0U);
    }

    // Error: arity
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmAddMovement, seg), interpreter::Error);
    }

    // Error: type
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackString("MoveTo");
        seg.pushBackInteger(SHIP_X);
        seg.pushBackString("Y");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmAddMovement, seg), interpreter::Error);
    }
}

/** Test callTaskEditorMethod(itmConfirmMessage). */
void
TestGameInterfaceTaskEditorContext::testCommandConfirmMessage()
{
    // Normal case
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        game::interface::NotificationStore::Message* msg = env.session.notifications().addMessage(edit->process().getProcessId(), "head", "body", game::Reference(game::Reference::Ship, 10));
        TS_ASSERT(!env.session.notifications().isMessageConfirmed(msg));

        afl::data::Segment seg;
        callMethod(env, *edit, game::interface::itmConfirmMessage, seg);

        TS_ASSERT(env.session.notifications().isMessageConfirmed(msg));
    }

    // Call without existing message is a no-op
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        TS_ASSERT_THROWS_NOTHING(callMethod(env, *edit, game::interface::itmConfirmMessage, seg));
    }

    // Error: arity
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmConfirmMessage, seg), interpreter::Error);
    }
}

/** Test callTaskEditorMethod(itmInsert). */
void
TestGameInterfaceTaskEditorContext::testCommandInsert()
{
    const String_t cmds[] = {"a", "b", "c", "d", "e"};

    // Add 'next'
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackString("next");
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 7U);
        TS_ASSERT_EQUALS(edit->getPC(), 1U);
        TS_ASSERT_EQUALS(edit->getCursor(), 3U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "x");
        TS_ASSERT_EQUALS((*edit)[2], "y");
        TS_ASSERT_EQUALS((*edit)[3], "b");
        TS_ASSERT_EQUALS((*edit)[4], "c");
        TS_ASSERT_EQUALS((*edit)[5], "d");
        TS_ASSERT_EQUALS((*edit)[6], "e");
    }

    // Add 'end'
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackString("end");
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 7U);
        TS_ASSERT_EQUALS(edit->getPC(), 1U);
        TS_ASSERT_EQUALS(edit->getCursor(), 7U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "b");
        TS_ASSERT_EQUALS((*edit)[2], "c");
        TS_ASSERT_EQUALS((*edit)[3], "d");
        TS_ASSERT_EQUALS((*edit)[4], "e");
        TS_ASSERT_EQUALS((*edit)[5], "x");
        TS_ASSERT_EQUALS((*edit)[6], "y");
    }

    // Add at beginning
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(0);
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 7U);
        TS_ASSERT_EQUALS(edit->getPC(), 3U);
        TS_ASSERT_EQUALS(edit->getCursor(), 4U);
        TS_ASSERT_EQUALS((*edit)[0], "x");
        TS_ASSERT_EQUALS((*edit)[1], "y");
        TS_ASSERT_EQUALS((*edit)[2], "a");
        TS_ASSERT_EQUALS((*edit)[3], "b");
        TS_ASSERT_EQUALS((*edit)[4], "c");
        TS_ASSERT_EQUALS((*edit)[5], "d");
        TS_ASSERT_EQUALS((*edit)[6], "e");
    }

    // Add at specific place (PC)
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 7U);
        TS_ASSERT_EQUALS(edit->getPC(), 3U);
        TS_ASSERT_EQUALS(edit->getCursor(), 4U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "x");
        TS_ASSERT_EQUALS((*edit)[2], "y");
        TS_ASSERT_EQUALS((*edit)[3], "b");
        TS_ASSERT_EQUALS((*edit)[4], "c");
        TS_ASSERT_EQUALS((*edit)[5], "d");
        TS_ASSERT_EQUALS((*edit)[6], "e");
    }

    // Add at specific place (end)
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 7U);
        TS_ASSERT_EQUALS(edit->getPC(), 1U);
        TS_ASSERT_EQUALS(edit->getCursor(), 2U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "b");
        TS_ASSERT_EQUALS((*edit)[2], "c");
        TS_ASSERT_EQUALS((*edit)[3], "d");
        TS_ASSERT_EQUALS((*edit)[4], "e");
        TS_ASSERT_EQUALS((*edit)[5], "x");
        TS_ASSERT_EQUALS((*edit)[6], "y");
    }

    // Null position
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        seg.pushBackString("x");
        seg.pushBackString("y");
        callMethod(env, *edit, game::interface::itmInsert, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Position range error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(6);
        seg.pushBackString("x");
        seg.pushBackString("y");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Position range error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(-1);
        seg.pushBackString("x");
        seg.pushBackString("y");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Position range error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackString("what");
        seg.pushBackString("x");
        seg.pushBackString("y");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Arity error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);
        edit->setCursor(2);
        edit->setPC(1);

        afl::data::Segment seg;
        seg.pushBackInteger(0);
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmInsert, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }
}

/** Test callTaskEditorMethod(itmDelete). */
void
TestGameInterfaceTaskEditorContext::testCommandDelete()
{
    const String_t cmds[] = {"a", "b", "c", "d", "e"};

    // Delete one
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackInteger(2);
        callMethod(env, *edit, game::interface::itmDelete, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 4U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "b");
        TS_ASSERT_EQUALS((*edit)[2], "d");
        TS_ASSERT_EQUALS((*edit)[3], "e");
    }

    // Delete multiple
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(3);
        callMethod(env, *edit, game::interface::itmDelete, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 2U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
        TS_ASSERT_EQUALS((*edit)[1], "e");
    }

    // Delete to end
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackInteger(1);
        seg.pushBackInteger(99999);
        callMethod(env, *edit, game::interface::itmDelete, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "a");
    }

    // Delete at end
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackInteger(5);
        callMethod(env, *edit, game::interface::itmDelete, seg);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Null position
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackNew(0);
        TS_ASSERT_THROWS_NOTHING(callMethod(env, *edit, game::interface::itmDelete, seg));
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Range error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackInteger(6);
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Type error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        seg.pushBackString("X");
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }

    // Arity error
    {
        Environment env;
        afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, 10);
        edit->addAsCurrent(cmds);

        afl::data::Segment seg;
        TS_ASSERT_THROWS(callMethod(env, *edit, game::interface::itmDelete, seg), interpreter::Error);
        TS_ASSERT_EQUALS(edit->getNumInstructions(), 5U);
    }
}

/** Test TaskEditorContext. */
void
TestGameInterfaceTaskEditorContext::testContext()
{
    const int SHIP_ID = 42;
    const String_t cmds[] = {"SetSpeed 5", "SetEnemy 3"};

    Environment env;
    afl::base::Ptr<interpreter::TaskEditor> edit = prepareShipTask(env, SHIP_ID);
    edit->addAsCurrent(cmds);
    game::interface::TaskEditorContext testee(edit, env.session);

    // Verify general properties
    interpreter::test::ContextVerifier verif(testee, "testContext");
    verif.verifyBasics();
    verif.verifyTypes();
    verif.verifyNotSerializable();
    TS_ASSERT(testee.getObject() == 0);
    TS_ASSERT(!testee.next());

    // Verify specific properties
    verif.verifyInteger("ID", SHIP_ID);
    verif.verifyString("TYPE", "ship");
    verif.verifyInteger("CURRENT", 0);

    // Assignment
    verif.setIntegerValue("CURRENT", 1);
    TS_ASSERT_EQUALS(edit->getPC(), 1U);

    TS_ASSERT_THROWS(verif.setIntegerValue("DELETE", 1), interpreter::Error);

    // Retrieve and call a command
    {
        std::auto_ptr<afl::data::Value> v(verif.getValue("DELETE"));
        interpreter::CallableValue* cv = dynamic_cast<interpreter::CallableValue*>(v.get());
        TS_ASSERT(cv != 0);

        interpreter::test::ValueVerifier vv(*cv, "testContext DELETE");
        vv.verifyBasics();
        vv.verifyNotSerializable();
        TS_ASSERT(cv->isProcedureCall());
        TS_ASSERT_EQUALS(cv->getDimension(0), 0);

        interpreter::Process proc(env.session.world(), "tester", 777);
        afl::data::Segment seg;
        seg.pushBackInteger(0);
        cv->call(proc, seg, false);

        TS_ASSERT_EQUALS(edit->getNumInstructions(), 1U);
        TS_ASSERT_EQUALS((*edit)[0], "SetEnemy 3");
    }
}

/** Test TaskEditorContext::create(). */
void
TestGameInterfaceTaskEditorContext::testCreate()
{
    // Success case
    {
        const int SHIP_ID = 30;
        Environment env;
        addRoot(env);
        addShipList(env);
        addGame(env);
        addShip(env, SHIP_ID, 7);

        std::auto_ptr<game::interface::TaskEditorContext> ctx(game::interface::TaskEditorContext::create(env.session, interpreter::Process::pkShipTask, SHIP_ID));
        TS_ASSERT(ctx.get() != 0);

        interpreter::test::ContextVerifier(*ctx, "testCreate").verifyInteger("ID", SHIP_ID);
    }

    // Failure case
    {
        Environment env;

        std::auto_ptr<game::interface::TaskEditorContext> ctx(game::interface::TaskEditorContext::create(env.session, interpreter::Process::pkShipTask, 99));
        TS_ASSERT(ctx.get() == 0);
    }
}

