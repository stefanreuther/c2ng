/**
  *  \file test/game/interface/shippropertytest.cpp
  *  \brief Test for game::interface::ShipProperty
  */

#include "game/interface/shipproperty.hpp"

#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/exception.hpp"
#include "game/game.hpp"
#include "game/interface/referencecontext.hpp"
#include "game/session.hpp"
#include "game/spec/shiplist.hpp"
#include "game/test/root.hpp"
#include "game/test/shiplist.hpp"
#include "game/turn.hpp"
#include "interpreter/context.hpp"
#include "interpreter/error.hpp"
#include "interpreter/indexablevalue.hpp"
#include "interpreter/test/contextverifier.hpp"
#include "interpreter/test/valueverifier.hpp"

using game::config::HostConfiguration;
using interpreter::test::verifyNewBoolean;
using interpreter::test::verifyNewFloat;
using interpreter::test::verifyNewInteger;
using interpreter::test::verifyNewNull;
using interpreter::test::verifyNewString;

namespace {
    const int TURN_NR = 10;

    void addPlanetXY(game::Session& session, game::Game& g, game::Id_t id, int x, int y, String_t name)
    {
        game::map::Planet& pl = *g.currentTurn().universe().planets().create(id);
        pl.setPosition(game::map::Point(x, y));
        pl.setName(name);
        pl.internalCheck(g.mapConfiguration(), game::PlayerSet_t(), TURN_NR, session.translator(), session.log());
    }

    void addShipXY(game::Session& /*session*/, game::Game& g, game::Id_t id, int x, int y, int owner, int scanner, String_t name)
    {
        game::map::Ship& sh = *g.currentTurn().universe().ships().create(id);
        sh.addShipXYData(game::map::Point(x, y), owner, /* mass */ 400, game::PlayerSet_t(scanner));
        sh.setName(name);
        sh.internalCheck(game::PlayerSet_t(scanner), TURN_NR);
    }
}

/** Full test case for a torpedo ship.
    - ship has beams and torpedoes
    - other ships and planets exist to exercise access to those
    - message link exists
    - level score exists
    - verify all read/write properties */
AFL_TEST("game.interface.ShipProperty:basics", a)
{
    const int PLAYER = 3;
    const int SHIP_ID = 77;
    const int PLANET_ID = 99;
    const int FAR_SHIP_ID = 111;
    const int NEAR_SHIP_ID = 222;
    const int X = 1100;
    const int Y = 1300;
    const int DX = 100;
    const int DY = 200;
    const int BEAM_NR = 5;
    const int TORP_NR = 7;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    for (int i = 0; i <= 10; ++i) {
        root->playerList().create(i);       // This will enable setting PE to 0..10
    }
    root->hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);
    root->hostConfiguration()[HostConfiguration::EPShipAging].set(32);
    root->hostConfiguration()[HostConfiguration::ExperienceLevelNames].set("Noob,Nieswurz,Brotfahrer,Ladehugo,Erdwurm");

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // - standard data
    game::test::addAnnihilation(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);
    shipList->beams().get(BEAM_NR)->setShortName("beam short");
    shipList->launchers().get(TORP_NR)->setShortName("torp short");

    // - mission definition
    game::spec::Mission msn(5, ",Sensor Sweep");
    msn.setShortName("ssw");
    shipList->missions().addMission(msn);

    // - basic hull functions
    shipList->basicHullFunctions().addFunction(game::spec::BasicHullFunction::Cloak, "Cloaking");
    shipList->basicHullFunctions().addFunction(game::spec::BasicHullFunction::MerlinAlchemy, "Alchemy");

    // Game/Turn
    afl::base::Ref<game::Game> g(*new game::Game());
    afl::base::Ref<game::Turn> turn(g->currentTurn());
    g->setViewpointPlayer(PLAYER);

    // - related units
    addPlanetXY(session, *g, PLANET_ID,    X,    Y,                      "Marble");
    addShipXY  (session, *g, NEAR_SHIP_ID, X,    Y,    PLAYER+1, PLAYER, "USS Near");
    addShipXY  (session, *g, FAR_SHIP_ID,  X+DX, Y+DY, PLAYER+1, PLAYER, "USS Far");

    // - messages
    for (int i = 0; i < 10; ++i) {
        g->currentTurn().inbox().addMessage("msg...", TURN_NR);
    }

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
    game::map::Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.addShipSpecialFunction(shipList->modifiedHullFunctions().getFunctionIdFromHostId(game::spec::BasicHullFunction::Cloak));
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

    // Test reading all scalar properties
    verifyNewInteger(a("ispAuxId"),                   getShipProperty(sh, game::interface::ispAuxId,                   session, root, shipList, g, turn), TORP_NR);
    verifyNewInteger(a("ispAuxAmmo"),                 getShipProperty(sh, game::interface::ispAuxAmmo,                 session, root, shipList, g, turn), 200);
    verifyNewInteger(a("ispAuxCount"),                getShipProperty(sh, game::interface::ispAuxCount,                session, root, shipList, g, turn), 2);
    verifyNewString (a("ispAuxShort"),                getShipProperty(sh, game::interface::ispAuxShort,                session, root, shipList, g, turn), "torp short");
    verifyNewString (a("ispAuxName"),                 getShipProperty(sh, game::interface::ispAuxName,                 session, root, shipList, g, turn), "Mark 5 Photon");
    verifyNewInteger(a("ispBeamId"),                  getShipProperty(sh, game::interface::ispBeamId,                  session, root, shipList, g, turn), BEAM_NR);
    verifyNewInteger(a("ispBeamCount"),               getShipProperty(sh, game::interface::ispBeamCount,               session, root, shipList, g, turn), 3);
    verifyNewString (a("ispBeamShort"),               getShipProperty(sh, game::interface::ispBeamShort,               session, root, shipList, g, turn), "beam short");
    verifyNewString (a("ispBeamName"),                getShipProperty(sh, game::interface::ispBeamName,                session, root, shipList, g, turn), "Positron Beam");
    verifyNewInteger(a("ispCargoColonists"),          getShipProperty(sh, game::interface::ispCargoColonists,          session, root, shipList, g, turn), 30);
    verifyNewInteger(a("ispCargoD"),                  getShipProperty(sh, game::interface::ispCargoD,                  session, root, shipList, g, turn), 9);
    verifyNewInteger(a("ispCargoFree"),               getShipProperty(sh, game::interface::ispCargoFree,               session, root, shipList, g, turn), 56);
    verifyNewInteger(a("ispCargoM"),                  getShipProperty(sh, game::interface::ispCargoM,                  session, root, shipList, g, turn), 8);
    verifyNewInteger(a("ispCargoMoney"),              getShipProperty(sh, game::interface::ispCargoMoney,              session, root, shipList, g, turn), 2000);
    verifyNewInteger(a("ispCargoN"),                  getShipProperty(sh, game::interface::ispCargoN,                  session, root, shipList, g, turn), 50);
    verifyNewString (a("ispCargoStr"),                getShipProperty(sh, game::interface::ispCargoStr,                session, root, shipList, g, turn), "50N 10T 9D 8M 30C 7S 2000$ 200W");
    verifyNewInteger(a("ispCargoSupplies"),           getShipProperty(sh, game::interface::ispCargoSupplies,           session, root, shipList, g, turn), 7);
    verifyNewInteger(a("ispCargoT"),                  getShipProperty(sh, game::interface::ispCargoT,                  session, root, shipList, g, turn), 10);
    verifyNewInteger(a("ispCrew"),                    getShipProperty(sh, game::interface::ispCrew,                    session, root, shipList, g, turn), 200);
    verifyNewInteger(a("ispDamage"),                  getShipProperty(sh, game::interface::ispDamage,                  session, root, shipList, g, turn), 5);
    verifyNewInteger(a("ispEnemyId"),                 getShipProperty(sh, game::interface::ispEnemyId,                 session, root, shipList, g, turn), 1);
    verifyNewInteger(a("ispEngineId"),                getShipProperty(sh, game::interface::ispEngineId,                session, root, shipList, g, turn), 9);
    verifyNewString (a("ispEngineName"),              getShipProperty(sh, game::interface::ispEngineName,              session, root, shipList, g, turn), "Transwarp Drive");
    verifyNewString (a("ispFCode"),                   getShipProperty(sh, game::interface::ispFCode,                   session, root, shipList, g, turn), "fcd");
    verifyNewInteger(a("ispFighterBays"),             getShipProperty(sh, game::interface::ispFighterBays,             session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispFighterCount"),            getShipProperty(sh, game::interface::ispFighterCount,            session, root, shipList, g, turn));
    verifyNewInteger(a("ispFleetId"),                 getShipProperty(sh, game::interface::ispFleetId,                 session, root, shipList, g, turn), 0);
    verifyNewString (a("ispFleetName"),               getShipProperty(sh, game::interface::ispFleetName,               session, root, shipList, g, turn), "");
    verifyNewString (a("ispFleetStatus"),             getShipProperty(sh, game::interface::ispFleetStatus,             session, root, shipList, g, turn), "-");
    verifyNewNull   (a("ispFleet"),                   getShipProperty(sh, game::interface::ispFleet,                   session, root, shipList, g, turn));
    verifyNewInteger(a("ispHeadingAngle"),            getShipProperty(sh, game::interface::ispHeadingAngle,            session, root, shipList, g, turn), 26);
    verifyNewString (a("ispHeadingName"),             getShipProperty(sh, game::interface::ispHeadingName,             session, root, shipList, g, turn), "NNE");
    verifyNewString (a("ispHullSpecial"),             getShipProperty(sh, game::interface::ispHullSpecial,             session, root, shipList, g, turn), "C");
    verifyNewInteger(a("ispId"),                      getShipProperty(sh, game::interface::ispId,                      session, root, shipList, g, turn), 77);
    verifyNewInteger(a("ispLevel"),                   getShipProperty(sh, game::interface::ispLevel,                   session, root, shipList, g, turn), 3);
    verifyNewInteger(a("ispLevelGain"),               getShipProperty(sh, game::interface::ispLevelGain,               session, root, shipList, g, turn), 32);  // EPShipAging
    verifyNewString (a("ispLevelName"),               getShipProperty(sh, game::interface::ispLevelName,               session, root, shipList, g, turn), "Ladehugo");
    verifyNewNull   (a("ispLevelPoints"),             getShipProperty(sh, game::interface::ispLevelPoints,             session, root, shipList, g, turn));
    verifyNewInteger(a("ispLocX"),                    getShipProperty(sh, game::interface::ispLocX,                    session, root, shipList, g, turn), X);
    verifyNewInteger(a("ispLocY"),                    getShipProperty(sh, game::interface::ispLocY,                    session, root, shipList, g, turn), Y);
    verifyNewString (a("ispLoc"),                     getShipProperty(sh, game::interface::ispLoc,                     session, root, shipList, g, turn), "Marble (#99)");
    verifyNewBoolean(a("ispMarked"),                  getShipProperty(sh, game::interface::ispMarked,                  session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispMass"),                    getShipProperty(sh, game::interface::ispMass,                    session, root, shipList, g, turn), 1289);
    verifyNewInteger(a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn), 5);
    verifyNewInteger(a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn), 0);
    verifyNewString (a("ispMissionShort"),            getShipProperty(sh, game::interface::ispMissionShort,            session, root, shipList, g, turn), "ssw");
    verifyNewInteger(a("ispMissionTow"),              getShipProperty(sh, game::interface::ispMissionTow,              session, root, shipList, g, turn), 0);
    verifyNewString (a("ispMissionName"),             getShipProperty(sh, game::interface::ispMissionName,             session, root, shipList, g, turn), "Sensor Sweep");
    verifyNewInteger(a("ispMoveETA"),                 getShipProperty(sh, game::interface::ispMoveETA,                 session, root, shipList, g, turn), 5);
    verifyNewInteger(a("ispMoveFuel"),                getShipProperty(sh, game::interface::ispMoveFuel,                session, root, shipList, g, turn), 273);
    verifyNewString (a("ispName"),                    getShipProperty(sh, game::interface::ispName,                    session, root, shipList, g, turn), "USS Cube");
    verifyNewInteger(a("ispOrbitId"),                 getShipProperty(sh, game::interface::ispOrbitId,                 session, root, shipList, g, turn), PLANET_ID);
    verifyNewString (a("ispOrbitName"),               getShipProperty(sh, game::interface::ispOrbitName,               session, root, shipList, g, turn), "Marble");
    verifyNewBoolean(a("ispPlayed"),                  getShipProperty(sh, game::interface::ispPlayed,                  session, root, shipList, g, turn), true);
    verifyNewInteger(a("ispRealOwner"),               getShipProperty(sh, game::interface::ispRealOwner,               session, root, shipList, g, turn), PLAYER);
    verifyNewInteger(a("ispSpeedId"),                 getShipProperty(sh, game::interface::ispSpeedId,                 session, root, shipList, g, turn), 7);
    verifyNewString (a("ispSpeedName"),               getShipProperty(sh, game::interface::ispSpeedName,               session, root, shipList, g, turn), "Warp 7");
    verifyNewBoolean(a("ispTask"),                    getShipProperty(sh, game::interface::ispTask,                    session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispTorpId"),                  getShipProperty(sh, game::interface::ispTorpId,                  session, root, shipList, g, turn), TORP_NR);
    verifyNewInteger(a("ispTorpCount"),               getShipProperty(sh, game::interface::ispTorpCount,               session, root, shipList, g, turn), 200);
    verifyNewInteger(a("ispTorpLCount"),              getShipProperty(sh, game::interface::ispTorpLCount,              session, root, shipList, g, turn), 2);
    verifyNewString (a("ispTorpShort"),               getShipProperty(sh, game::interface::ispTorpShort,               session, root, shipList, g, turn), "torp short");
    verifyNewString (a("ispTorpName"),                getShipProperty(sh, game::interface::ispTorpName,                session, root, shipList, g, turn), "Mark 5 Photon");
    verifyNewInteger(a("ispTransferShipColonists"),   getShipProperty(sh, game::interface::ispTransferShipColonists,   session, root, shipList, g, turn), 7);
    verifyNewInteger(a("ispTransferShipD"),           getShipProperty(sh, game::interface::ispTransferShipD,           session, root, shipList, g, turn), 5);
    verifyNewInteger(a("ispTransferShipId"),          getShipProperty(sh, game::interface::ispTransferShipId,          session, root, shipList, g, turn), NEAR_SHIP_ID);
    verifyNewInteger(a("ispTransferShipM"),           getShipProperty(sh, game::interface::ispTransferShipM,           session, root, shipList, g, turn), 6);
    verifyNewInteger(a("ispTransferShipN"),           getShipProperty(sh, game::interface::ispTransferShipN,           session, root, shipList, g, turn), 3);
    verifyNewString (a("ispTransferShipName"),        getShipProperty(sh, game::interface::ispTransferShipName,        session, root, shipList, g, turn), "USS Near");
    verifyNewInteger(a("ispTransferShipSupplies"),    getShipProperty(sh, game::interface::ispTransferShipSupplies,    session, root, shipList, g, turn), 8);
    verifyNewInteger(a("ispTransferShipT"),           getShipProperty(sh, game::interface::ispTransferShipT,           session, root, shipList, g, turn), 4);
    verifyNewBoolean(a("ispTransferShip"),            getShipProperty(sh, game::interface::ispTransferShip,            session, root, shipList, g, turn), true);
    verifyNewInteger(a("ispTransferUnloadColonists"), getShipProperty(sh, game::interface::ispTransferUnloadColonists, session, root, shipList, g, turn), 24);
    verifyNewInteger(a("ispTransferUnloadD"),         getShipProperty(sh, game::interface::ispTransferUnloadD,         session, root, shipList, g, turn), 22);
    verifyNewInteger(a("ispTransferUnloadId"),        getShipProperty(sh, game::interface::ispTransferUnloadId,        session, root, shipList, g, turn), PLANET_ID);
    verifyNewInteger(a("ispTransferUnloadM"),         getShipProperty(sh, game::interface::ispTransferUnloadM,         session, root, shipList, g, turn), 23);
    verifyNewInteger(a("ispTransferUnloadN"),         getShipProperty(sh, game::interface::ispTransferUnloadN,         session, root, shipList, g, turn), 20);
    verifyNewString (a("ispTransferUnloadName"),      getShipProperty(sh, game::interface::ispTransferUnloadName,      session, root, shipList, g, turn), "Marble");
    verifyNewInteger(a("ispTransferUnloadSupplies"),  getShipProperty(sh, game::interface::ispTransferUnloadSupplies,  session, root, shipList, g, turn), 25);
    verifyNewInteger(a("ispTransferUnloadT"),         getShipProperty(sh, game::interface::ispTransferUnloadT,         session, root, shipList, g, turn), 21);
    verifyNewBoolean(a("ispTransferUnload"),          getShipProperty(sh, game::interface::ispTransferUnload,          session, root, shipList, g, turn), true);
    verifyNewString (a("ispTypeChar"),                getShipProperty(sh, game::interface::ispTypeChar,                session, root, shipList, g, turn), "T");
    verifyNewString (a("ispTypeStr"),                 getShipProperty(sh, game::interface::ispTypeStr,                 session, root, shipList, g, turn), "Torpedo Ship");
    verifyNewFloat  (a("ispWaypointDistance"),        getShipProperty(sh, game::interface::ispWaypointDistance,        session, root, shipList, g, turn), 223.6, 1.0);
    verifyNewInteger(a("ispWaypointDX"),              getShipProperty(sh, game::interface::ispWaypointDX,              session, root, shipList, g, turn), DX);
    verifyNewInteger(a("ispWaypointDY"),              getShipProperty(sh, game::interface::ispWaypointDY,              session, root, shipList, g, turn), DY);
    verifyNewInteger(a("ispWaypointPlanetId"),        getShipProperty(sh, game::interface::ispWaypointPlanetId,        session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointX"),               getShipProperty(sh, game::interface::ispWaypointX,               session, root, shipList, g, turn), X+DX);
    verifyNewInteger(a("ispWaypointY"),               getShipProperty(sh, game::interface::ispWaypointY,               session, root, shipList, g, turn), Y+DY);
    verifyNewString (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn), "(1200,1500)");

    // Complex values
    {
        // ispMessages - an iterable array
        std::auto_ptr<afl::data::Value> p(getShipProperty(sh, game::interface::ispMessages, session, root, shipList, g, turn));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ispMessages: IndexableValue", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ispMessages"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ispMessages dim 0", ix->getDimension(0), 1);
        a.checkEqual("ispMessages dim 1", ix->getDimension(1), 3);   // 2 messages

        // Quick test that messages can be retrieved
        std::auto_ptr<interpreter::Context> ctx(ix->makeFirstContext());
        a.checkNonNull("ispMessages makeFirstContext", ctx.get());
        interpreter::test::ContextVerifier cv(*ctx, a("ispMessages makeFirstContext"));
        cv.verifyBasics();
        cv.verifyNotSerializable();
        cv.verifyInteger("ID", 3);                  // 1-based, thus index 0 reported as 1 to user
        cv.verifyString("FULLTEXT", "msg...");
    }
    {
        // ispReference - reference
        std::auto_ptr<afl::data::Value> p(getShipProperty(sh, game::interface::ispReference, session, root, shipList, g, turn));
        game::Reference ref;
        a.check("ispReference", game::interface::checkReferenceArg(ref, p.get()));
        a.checkEqual("ispReference value", ref, game::Reference(game::Reference::Ship, SHIP_ID));
    }
    {
        // ispScore - function (not iterable)
        std::auto_ptr<afl::data::Value> p(getShipProperty(sh, game::interface::ispScore, session, root, shipList, g, turn));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ispScore", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ispScore"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ispScore dim", ix->getDimension(0), 0);
        AFL_CHECK_THROWS(a("ispScore makeFirstContext"), ix->makeFirstContext(), interpreter::Error);

        // Retrieve existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewInteger(a("ispScore(Level)"), ix->get(args), 3);
        }

        // Retrieve non-existing score value
        {
            afl::data::Segment seg;
            seg.pushBackInteger(999);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ispScore(999)"), ix->get(args));
        }

        // Null index
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ispScore(999)"), ix->get(args));
        }

        // Arity error
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 0);
            AFL_CHECK_THROWS(a("ispScore arity error"), ix->get(args), interpreter::Error);
        }

        // Type error
        {
            afl::data::Segment seg;
            seg.pushBackString("X");
            interpreter::Arguments args(seg, 0, 1);
            AFL_CHECK_THROWS(a("ispScore type error"), ix->get(args), interpreter::Error);
        }

        // Not assignable
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::ScoreId_ExpLevel);
            interpreter::Arguments args(seg, 0, 1);
            afl::data::IntegerValue iv(5);
            AFL_CHECK_THROWS(a("ispScore not assignable"), ix->set(args, &iv), interpreter::Error);
        }
    }
    {
        // ispHasFunction - function (not iterable)
        std::auto_ptr<afl::data::Value> p(getShipProperty(sh, game::interface::ispHasFunction, session, root, shipList, g, turn));
        interpreter::IndexableValue* ix = dynamic_cast<interpreter::IndexableValue*>(p.get());
        a.checkNonNull("ispHasFunction", ix);
        interpreter::test::ValueVerifier verif(*ix, a("ispHasFunction"));
        verif.verifyBasics();
        verif.verifyNotSerializable();
        a.checkEqual("ispHasFunction dim", ix->getDimension(0), 0);
        AFL_CHECK_THROWS(a("ispHasFunction makeFirstContext"), ix->makeFirstContext(), interpreter::Error);

        // Retrieve existing value - true
        {
            afl::data::Segment seg;
            seg.pushBackString("cloaking");
            interpreter::Arguments args(seg, 0, 1);
            verifyNewBoolean(a("ispHasFunction(Cloaking)"), ix->get(args), true);
        }

        // Retrieve existing value using integer index - true
        {
            afl::data::Segment seg;
            seg.pushBackInteger(game::spec::BasicHullFunction::Cloak);
            interpreter::Arguments args(seg, 0, 1);
            verifyNewBoolean(a("ispHasFunction(Cloak)"), ix->get(args), true);
        }

        // Retrieve existing value - false
        {
            afl::data::Segment seg;
            seg.pushBackString("alchemy");
            interpreter::Arguments args(seg, 0, 1);
            verifyNewBoolean(a("ispHasFunction(Alchemy)"), ix->get(args), false);
        }

        // Retrieve non-existing value
        {
            afl::data::Segment seg;
            seg.pushBackString("superperforator");
            interpreter::Arguments args(seg, 0, 1);
            AFL_CHECK_THROWS(a("ispHasFunction invalid"), ix->get(args), interpreter::Error);
        }

        // Null index
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 1);
            verifyNewNull(a("ispHasFunction(null)"), ix->get(args));
        }

        // Arity error
        {
            afl::data::Segment seg;
            interpreter::Arguments args(seg, 0, 0);
            AFL_CHECK_THROWS(a("ispHasFunction arity error"), ix->get(args), interpreter::Error);
        }

        // Not assignable
        {
            afl::data::Segment seg;
            seg.pushBackInteger(0);
            interpreter::Arguments args(seg, 0, 1);
            afl::data::IntegerValue iv(5);
            AFL_CHECK_THROWS(a("ispHasFunction not assignable"), ix->set(args, &iv), interpreter::Error);
        }
    }

    // Writable properties
    {
        afl::data::StringValue sv("qrs");
        setShipProperty(sh, game::interface::ispFCode, &sv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispFCode", sh.getFriendlyCode().orElse(""), "qrs");
    }
    {
        afl::data::IntegerValue iv(42);
        setShipProperty(sh, game::interface::ispMissionId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispMissionId", sh.getMission().orElse(-1), 42);
    }
    {
        afl::data::IntegerValue iv(42);
        setShipProperty(sh, game::interface::ispMissionIntercept, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispMissionIntercept", sh.getMissionParameter(game::InterceptParameter).orElse(-1), 42);
    }
    {
        afl::data::IntegerValue iv(42);
        setShipProperty(sh, game::interface::ispMissionTow, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispMissionTow", sh.getMissionParameter(game::TowParameter).orElse(-1), 42);
    }
    {
        afl::data::StringValue sv("USS Incognito");
        setShipProperty(sh, game::interface::ispName, &sv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispName", sh.getName(), "USS Incognito");
    }
    {
        afl::data::IntegerValue iv(3);
        setShipProperty(sh, game::interface::ispSpeedId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispSpeedId", sh.getWarpFactor().orElse(-1), 3);
    }
    {
        afl::data::IntegerValue iv(10);
        setShipProperty(sh, game::interface::ispEnemyId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispEnemyId", sh.getPrimaryEnemy().orElse(-1), 10);
    }

    // Error case: not assignable
    {
        afl::data::IntegerValue iv(10);
        AFL_CHECK_THROWS(a("set ispCrew"), setShipProperty(sh, game::interface::ispCrew, &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }

    // Error case: range error
    {
        afl::data::IntegerValue iv(160);
        AFL_CHECK_THROWS(a("set ispSpeedId: range"), setShipProperty(sh, game::interface::ispSpeedId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }
    {
        afl::data::IntegerValue iv(16);
        AFL_CHECK_THROWS(a("set ispEnemyId: range"), setShipProperty(sh, game::interface::ispEnemyId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }
}

/** Test case for a carrier.
    - ship has beams and fighters
    - no other units, messages, scores
    - verify all relevant read properties */
AFL_TEST("game.interface.ShipProperty:carrier", a)
{
    const int PLAYER = 3;
    const int SHIP_ID = 77;
    const int X = 1100;
    const int Y = 1300;
    const int BEAM_NR = 5;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));
    root->hostConfiguration()[HostConfiguration::NumExperienceLevels].set(4);
    root->hostConfiguration()[HostConfiguration::EPShipAging].set(0);

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // - standard data
    game::test::addGorbie(*shipList);
    game::test::addTranswarp(*shipList);
    game::test::initStandardBeams(*shipList);
    game::test::initStandardTorpedoes(*shipList);
    shipList->beams().get(BEAM_NR)->setShortName("beam short");

    // Game/Turn
    afl::base::Ref<game::Game> g(*new game::Game());
    afl::base::Ref<game::Turn> turn(g->currentTurn());
    g->setViewpointPlayer(PLAYER);

    // Ship under test
    game::map::ShipData sd;
    sd.owner                     = PLAYER;
    sd.friendlyCode              = "fgh";
    sd.warpFactor                = 0;
    sd.waypointDX                = 0;
    sd.waypointDY                = 0;
    sd.x                         = X;
    sd.y                         = Y;
    sd.engineType                = 9;
    sd.hullType                  = game::test::GORBIE_HULL_ID;
    sd.beamType                  = BEAM_NR;
    sd.numBeams                  = 3;
    sd.numBays                   = 10;
    sd.torpedoType               = 0;
    sd.ammo                      = 60;
    sd.numLaunchers              = 0;
    sd.mission                   = 25;
    sd.primaryEnemy              = 0;
    sd.missionTowParameter       = 10;
    sd.damage                    = 0;
    sd.crew                      = 200;
    sd.colonists                 = 30;
    sd.name                      = "Powerball";
    sd.neutronium                = 10;
    sd.tritanium                 = 20;
    sd.duranium                  = 30;
    sd.molybdenum                = 40;
    sd.supplies                  = 50;
    sd.unload.neutronium         = 0;
    sd.unload.tritanium          = 0;
    sd.unload.duranium           = 0;
    sd.unload.molybdenum         = 0;
    sd.unload.colonists          = 0;
    sd.unload.supplies           = 0;
    sd.unload.targetId           = 0;
    sd.transfer.neutronium       = 0;
    sd.transfer.tritanium        = 0;
    sd.transfer.duranium         = 0;
    sd.transfer.molybdenum       = 0;
    sd.transfer.colonists        = 0;
    sd.transfer.supplies         = 0;
    sd.transfer.targetId         = 0;
    sd.missionInterceptParameter = 55;
    sd.money                     = 1000;

    // Create ship. Must be part of the universe because MovementPredictor resolves it through it.
    game::map::Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.setPlayability(game::map::Object::Playable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);
    sh.setIsMarked(true);
    sh.setFleetNumber(SHIP_ID);
    sh.setFleetName("Invader");

    // Experience points
    game::UnitScoreDefinitionList::Definition levelDef;
    levelDef.name  = "Points";
    levelDef.id    = game::ScoreId_ExpPoints;
    levelDef.limit = -1;
    sh.unitScores().set(g->shipScores().add(levelDef), 300, TURN_NR);

    // Test reading all scalar properties
    verifyNewInteger(a("ispAuxId"),                   getShipProperty(sh, game::interface::ispAuxId,                   session, root, shipList, g, turn), 11);
    verifyNewInteger(a("ispAuxAmmo"),                 getShipProperty(sh, game::interface::ispAuxAmmo,                 session, root, shipList, g, turn), 60);
    verifyNewInteger(a("ispAuxCount"),                getShipProperty(sh, game::interface::ispAuxCount,                session, root, shipList, g, turn), 10);
    verifyNewString (a("ispAuxShort"),                getShipProperty(sh, game::interface::ispAuxShort,                session, root, shipList, g, turn), "Ftr");
    verifyNewString (a("ispAuxName"),                 getShipProperty(sh, game::interface::ispAuxName,                 session, root, shipList, g, turn), "Fighters");
    verifyNewInteger(a("ispBeamId"),                  getShipProperty(sh, game::interface::ispBeamId,                  session, root, shipList, g, turn), BEAM_NR);
    verifyNewInteger(a("ispBeamCount"),               getShipProperty(sh, game::interface::ispBeamCount,               session, root, shipList, g, turn), 3);
    verifyNewString (a("ispBeamShort"),               getShipProperty(sh, game::interface::ispBeamShort,               session, root, shipList, g, turn), "beam short");
    verifyNewString (a("ispBeamName"),                getShipProperty(sh, game::interface::ispBeamName,                session, root, shipList, g, turn), "Positron Beam");
    verifyNewInteger(a("ispCargoColonists"),          getShipProperty(sh, game::interface::ispCargoColonists,          session, root, shipList, g, turn), 30);
    verifyNewInteger(a("ispCargoD"),                  getShipProperty(sh, game::interface::ispCargoD,                  session, root, shipList, g, turn), 30);
    verifyNewInteger(a("ispCargoFree"),               getShipProperty(sh, game::interface::ispCargoFree,               session, root, shipList, g, turn), 20);
    verifyNewInteger(a("ispCargoM"),                  getShipProperty(sh, game::interface::ispCargoM,                  session, root, shipList, g, turn), 40);
    verifyNewInteger(a("ispCargoMoney"),              getShipProperty(sh, game::interface::ispCargoMoney,              session, root, shipList, g, turn), 1000);
    verifyNewInteger(a("ispCargoN"),                  getShipProperty(sh, game::interface::ispCargoN,                  session, root, shipList, g, turn), 10);
    verifyNewString (a("ispCargoStr"),                getShipProperty(sh, game::interface::ispCargoStr,                session, root, shipList, g, turn), "10N 20T 30D 40M 60F 30C 50S 1000$");
    verifyNewInteger(a("ispCargoSupplies"),           getShipProperty(sh, game::interface::ispCargoSupplies,           session, root, shipList, g, turn), 50);
    verifyNewInteger(a("ispCargoT"),                  getShipProperty(sh, game::interface::ispCargoT,                  session, root, shipList, g, turn), 20);
    verifyNewInteger(a("ispCrew"),                    getShipProperty(sh, game::interface::ispCrew,                    session, root, shipList, g, turn), 200);
    verifyNewInteger(a("ispDamage"),                  getShipProperty(sh, game::interface::ispDamage,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispEnemyId"),                 getShipProperty(sh, game::interface::ispEnemyId,                 session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispEngineId"),                getShipProperty(sh, game::interface::ispEngineId,                session, root, shipList, g, turn), 9);
    verifyNewString (a("ispEngineName"),              getShipProperty(sh, game::interface::ispEngineName,              session, root, shipList, g, turn), "Transwarp Drive");
    verifyNewString (a("ispFCode"),                   getShipProperty(sh, game::interface::ispFCode,                   session, root, shipList, g, turn), "fgh");
    verifyNewInteger(a("ispFighterBays"),             getShipProperty(sh, game::interface::ispFighterBays,             session, root, shipList, g, turn), 10);
    verifyNewInteger(a("ispFighterCount"),            getShipProperty(sh, game::interface::ispFighterCount,            session, root, shipList, g, turn), 60);
    verifyNewInteger(a("ispFleetId"),                 getShipProperty(sh, game::interface::ispFleetId,                 session, root, shipList, g, turn), SHIP_ID);
    verifyNewString (a("ispFleetName"),               getShipProperty(sh, game::interface::ispFleetName,               session, root, shipList, g, turn), "Invader");
    verifyNewString (a("ispFleetStatus"),             getShipProperty(sh, game::interface::ispFleetStatus,             session, root, shipList, g, turn), "leader");
    verifyNewString (a("ispFleet"),                   getShipProperty(sh, game::interface::ispFleet,                   session, root, shipList, g, turn), "Invader");
    verifyNewNull   (a("ispHeadingAngle"),            getShipProperty(sh, game::interface::ispHeadingAngle,            session, root, shipList, g, turn));
    verifyNewNull   (a("ispHeadingName"),             getShipProperty(sh, game::interface::ispHeadingName,             session, root, shipList, g, turn));
    verifyNewString (a("ispHullSpecial"),             getShipProperty(sh, game::interface::ispHullSpecial,             session, root, shipList, g, turn), "");
    verifyNewInteger(a("ispId"),                      getShipProperty(sh, game::interface::ispId,                      session, root, shipList, g, turn), 77);
    verifyNewNull   (a("ispLevel"),                   getShipProperty(sh, game::interface::ispLevel,                   session, root, shipList, g, turn));
    verifyNewInteger(a("ispLevelGain"),               getShipProperty(sh, game::interface::ispLevelGain,               session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispLevelName"),               getShipProperty(sh, game::interface::ispLevelName,               session, root, shipList, g, turn));
    verifyNewInteger(a("ispLevelPoints"),             getShipProperty(sh, game::interface::ispLevelPoints,             session, root, shipList, g, turn), 300);
    verifyNewInteger(a("ispLocX"),                    getShipProperty(sh, game::interface::ispLocX,                    session, root, shipList, g, turn), X);
    verifyNewInteger(a("ispLocY"),                    getShipProperty(sh, game::interface::ispLocY,                    session, root, shipList, g, turn), Y);
    verifyNewString (a("ispLoc"),                     getShipProperty(sh, game::interface::ispLoc,                     session, root, shipList, g, turn), "(1100,1300)");
    verifyNewBoolean(a("ispMarked"),                  getShipProperty(sh, game::interface::ispMarked,                  session, root, shipList, g, turn), true);
    verifyNewInteger(a("ispMass"),                    getShipProperty(sh, game::interface::ispMass,                    session, root, shipList, g, turn), 1229);
    verifyNewInteger(a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn), 25);
    verifyNewInteger(a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn), 55);
    verifyNewString (a("ispMissionShort"),            getShipProperty(sh, game::interface::ispMissionShort,            session, root, shipList, g, turn), "MIT 25");
    verifyNewInteger(a("ispMissionTow"),              getShipProperty(sh, game::interface::ispMissionTow,              session, root, shipList, g, turn), 10);
    verifyNewString (a("ispMissionName"),             getShipProperty(sh, game::interface::ispMissionName,             session, root, shipList, g, turn), "M.I.T. 25 (55,10)");
    verifyNewInteger(a("ispMoveETA"),                 getShipProperty(sh, game::interface::ispMoveETA,                 session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispMoveFuel"),                getShipProperty(sh, game::interface::ispMoveFuel,                session, root, shipList, g, turn), 0);
    verifyNewString (a("ispName"),                    getShipProperty(sh, game::interface::ispName,                    session, root, shipList, g, turn), "Powerball");
    verifyNewInteger(a("ispOrbitId"),                 getShipProperty(sh, game::interface::ispOrbitId,                 session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispOrbitName"),               getShipProperty(sh, game::interface::ispOrbitName,               session, root, shipList, g, turn));
    verifyNewBoolean(a("ispPlayed"),                  getShipProperty(sh, game::interface::ispPlayed,                  session, root, shipList, g, turn), true);
    verifyNewInteger(a("ispRealOwner"),               getShipProperty(sh, game::interface::ispRealOwner,               session, root, shipList, g, turn), PLAYER);
    verifyNewInteger(a("ispSpeedId"),                 getShipProperty(sh, game::interface::ispSpeedId,                 session, root, shipList, g, turn), 0);
    verifyNewString (a("ispSpeedName"),               getShipProperty(sh, game::interface::ispSpeedName,               session, root, shipList, g, turn), "Warp 0");
    verifyNewBoolean(a("ispTask"),                    getShipProperty(sh, game::interface::ispTask,                    session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispTorpId"),                  getShipProperty(sh, game::interface::ispTorpId,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTorpCount"),               getShipProperty(sh, game::interface::ispTorpCount,               session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTorpLCount"),              getShipProperty(sh, game::interface::ispTorpLCount,              session, root, shipList, g, turn), 0);
    verifyNewString (a("ispTorpShort"),               getShipProperty(sh, game::interface::ispTorpShort,               session, root, shipList, g, turn), ""); // Correct?
    verifyNewString (a("ispTorpName"),                getShipProperty(sh, game::interface::ispTorpName,                session, root, shipList, g, turn), "");
    verifyNewInteger(a("ispTransferShipColonists"),   getShipProperty(sh, game::interface::ispTransferShipColonists,   session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferShipD"),           getShipProperty(sh, game::interface::ispTransferShipD,           session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferShipId"),          getShipProperty(sh, game::interface::ispTransferShipId,          session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferShipM"),           getShipProperty(sh, game::interface::ispTransferShipM,           session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferShipN"),           getShipProperty(sh, game::interface::ispTransferShipN,           session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispTransferShipName"),        getShipProperty(sh, game::interface::ispTransferShipName,        session, root, shipList, g, turn));
    verifyNewInteger(a("ispTransferShipSupplies"),    getShipProperty(sh, game::interface::ispTransferShipSupplies,    session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferShipT"),           getShipProperty(sh, game::interface::ispTransferShipT,           session, root, shipList, g, turn), 0);
    verifyNewBoolean(a("ispTransferShip"),            getShipProperty(sh, game::interface::ispTransferShip,            session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispTransferUnloadColonists"), getShipProperty(sh, game::interface::ispTransferUnloadColonists, session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferUnloadD"),         getShipProperty(sh, game::interface::ispTransferUnloadD,         session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferUnloadId"),        getShipProperty(sh, game::interface::ispTransferUnloadId,        session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferUnloadM"),         getShipProperty(sh, game::interface::ispTransferUnloadM,         session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferUnloadN"),         getShipProperty(sh, game::interface::ispTransferUnloadN,         session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispTransferUnloadName"),      getShipProperty(sh, game::interface::ispTransferUnloadName,      session, root, shipList, g, turn));
    verifyNewInteger(a("ispTransferUnloadSupplies"),  getShipProperty(sh, game::interface::ispTransferUnloadSupplies,  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTransferUnloadT"),         getShipProperty(sh, game::interface::ispTransferUnloadT,         session, root, shipList, g, turn), 0);
    verifyNewBoolean(a("ispTransferUnload"),          getShipProperty(sh, game::interface::ispTransferUnload,          session, root, shipList, g, turn), false);
    verifyNewString (a("ispTypeChar"),                getShipProperty(sh, game::interface::ispTypeChar,                session, root, shipList, g, turn), "C");
    verifyNewString (a("ispTypeStr"),                 getShipProperty(sh, game::interface::ispTypeStr,                 session, root, shipList, g, turn), "Carrier");
    verifyNewFloat  (a("ispWaypointDistance"),        getShipProperty(sh, game::interface::ispWaypointDistance,        session, root, shipList, g, turn), 0.0, 0.001);
    verifyNewInteger(a("ispWaypointDX"),              getShipProperty(sh, game::interface::ispWaypointDX,              session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointDY"),              getShipProperty(sh, game::interface::ispWaypointDY,              session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointPlanetId"),        getShipProperty(sh, game::interface::ispWaypointPlanetId,        session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointX"),               getShipProperty(sh, game::interface::ispWaypointX,               session, root, shipList, g, turn), X);
    verifyNewInteger(a("ispWaypointY"),               getShipProperty(sh, game::interface::ispWaypointY,               session, root, shipList, g, turn), Y);
    verifyNewString (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn), "(Location)");

    // ispMessages is null if there are no messages
    verifyNewNull   (a("ispMessages"),                getShipProperty(sh, game::interface::ispMessages,                session, root, shipList, g, turn));

    // Writable properties: fleet stuff
    {
        afl::data::StringValue sv("peacekeeper");
        setShipProperty(sh, game::interface::ispFleetName, &sv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispFleetName", sh.getFleetName(), "peacekeeper");
    }
    {
        afl::data::IntegerValue iv(0);
        setShipProperty(sh, game::interface::ispFleetId, &iv, *root, *shipList, g->mapConfiguration(), turn->universe());
        a.checkEqual("set ispFleetId", sh.getFleetNumber(), 0);
    }
}

/** Test case for an empty/invisible ship.
    - ship has no data
    - no other units, messages, scores
    - verify all relevant read properties */
AFL_TEST("game.interface.ShipProperty:empty", a)
{
    const int PLAYER = 3;
    const int SHIP_ID = 123;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Game/Turn
    afl::base::Ref<game::Game> g(*new game::Game());
    afl::base::Ref<game::Turn> turn(g->currentTurn());

    // Create ship. Must be part of the universe because MovementPredictor resolves it through it.
    game::map::Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.setPlayability(game::map::Object::NotPlayable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    // Test reading all scalar properties
    verifyNewNull   (a("ispAuxId"),                   getShipProperty(sh, game::interface::ispAuxId,                   session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxAmmo"),                 getShipProperty(sh, game::interface::ispAuxAmmo,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxCount"),                getShipProperty(sh, game::interface::ispAuxCount,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxShort"),                getShipProperty(sh, game::interface::ispAuxShort,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxName"),                 getShipProperty(sh, game::interface::ispAuxName,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispBeamId"),                  getShipProperty(sh, game::interface::ispBeamId,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispBeamCount"),               getShipProperty(sh, game::interface::ispBeamCount,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispBeamShort"),               getShipProperty(sh, game::interface::ispBeamShort,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispBeamName"),                getShipProperty(sh, game::interface::ispBeamName,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoColonists"),          getShipProperty(sh, game::interface::ispCargoColonists,          session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoD"),                  getShipProperty(sh, game::interface::ispCargoD,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoFree"),               getShipProperty(sh, game::interface::ispCargoFree,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoM"),                  getShipProperty(sh, game::interface::ispCargoM,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoMoney"),              getShipProperty(sh, game::interface::ispCargoMoney,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoN"),                  getShipProperty(sh, game::interface::ispCargoN,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoStr"),                getShipProperty(sh, game::interface::ispCargoStr,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoSupplies"),           getShipProperty(sh, game::interface::ispCargoSupplies,           session, root, shipList, g, turn));
    verifyNewNull   (a("ispCargoT"),                  getShipProperty(sh, game::interface::ispCargoT,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispCrew"),                    getShipProperty(sh, game::interface::ispCrew,                    session, root, shipList, g, turn));
    verifyNewNull   (a("ispDamage"),                  getShipProperty(sh, game::interface::ispDamage,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispEnemyId"),                 getShipProperty(sh, game::interface::ispEnemyId,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispEngineId"),                getShipProperty(sh, game::interface::ispEngineId,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispEngineName"),              getShipProperty(sh, game::interface::ispEngineName,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispFCode"),                   getShipProperty(sh, game::interface::ispFCode,                   session, root, shipList, g, turn));
    verifyNewNull   (a("ispFighterBays"),             getShipProperty(sh, game::interface::ispFighterBays,             session, root, shipList, g, turn));
    verifyNewNull   (a("ispFighterCount"),            getShipProperty(sh, game::interface::ispFighterCount,            session, root, shipList, g, turn));
    verifyNewInteger(a("ispFleetId"),                 getShipProperty(sh, game::interface::ispFleetId,                 session, root, shipList, g, turn), 0);
    verifyNewString (a("ispFleetName"),               getShipProperty(sh, game::interface::ispFleetName,               session, root, shipList, g, turn), "");
    verifyNewString (a("ispFleetStatus"),             getShipProperty(sh, game::interface::ispFleetStatus,             session, root, shipList, g, turn), "-");
    verifyNewNull   (a("ispFleet"),                   getShipProperty(sh, game::interface::ispFleet,                   session, root, shipList, g, turn));
    verifyNewNull   (a("ispHeadingAngle"),            getShipProperty(sh, game::interface::ispHeadingAngle,            session, root, shipList, g, turn));
    verifyNewNull   (a("ispHeadingName"),             getShipProperty(sh, game::interface::ispHeadingName,             session, root, shipList, g, turn));
    verifyNewString (a("ispHullSpecial"),             getShipProperty(sh, game::interface::ispHullSpecial,             session, root, shipList, g, turn), "");
    verifyNewInteger(a("ispId"),                      getShipProperty(sh, game::interface::ispId,                      session, root, shipList, g, turn), 123);
    verifyNewNull   (a("ispLevel"),                   getShipProperty(sh, game::interface::ispLevel,                   session, root, shipList, g, turn));
    verifyNewNull   (a("ispLocX"),                    getShipProperty(sh, game::interface::ispLocX,                    session, root, shipList, g, turn));
    verifyNewNull   (a("ispLocY"),                    getShipProperty(sh, game::interface::ispLocY,                    session, root, shipList, g, turn));
    verifyNewNull   (a("ispLoc"),                     getShipProperty(sh, game::interface::ispLoc,                     session, root, shipList, g, turn));
    verifyNewBoolean(a("ispMarked"),                  getShipProperty(sh, game::interface::ispMarked,                  session, root, shipList, g, turn), false);
    verifyNewNull   (a("ispMass"),                    getShipProperty(sh, game::interface::ispMass,                    session, root, shipList, g, turn));
    verifyNewNull   (a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn));
    verifyNewNull   (a("ispMissionShort"),            getShipProperty(sh, game::interface::ispMissionShort,            session, root, shipList, g, turn));
    verifyNewNull   (a("ispMissionTow"),              getShipProperty(sh, game::interface::ispMissionTow,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispMissionName"),             getShipProperty(sh, game::interface::ispMissionName,             session, root, shipList, g, turn));
    verifyNewNull   (a("ispMoveETA"),                 getShipProperty(sh, game::interface::ispMoveETA,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispMoveFuel"),                getShipProperty(sh, game::interface::ispMoveFuel,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispName"),                    getShipProperty(sh, game::interface::ispName,                    session, root, shipList, g, turn));
    verifyNewNull   (a("ispOrbitId"),                 getShipProperty(sh, game::interface::ispOrbitId,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispOrbitName"),               getShipProperty(sh, game::interface::ispOrbitName,               session, root, shipList, g, turn));
    verifyNewBoolean(a("ispPlayed"),                  getShipProperty(sh, game::interface::ispPlayed,                  session, root, shipList, g, turn), false);
    verifyNewNull   (a("ispRealOwner"),               getShipProperty(sh, game::interface::ispRealOwner,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispSpeedId"),                 getShipProperty(sh, game::interface::ispSpeedId,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispSpeedName"),               getShipProperty(sh, game::interface::ispSpeedName,               session, root, shipList, g, turn));
    verifyNewBoolean(a("ispTask"),                    getShipProperty(sh, game::interface::ispTask,                    session, root, shipList, g, turn), false);
    verifyNewNull   (a("ispTorpId"),                  getShipProperty(sh, game::interface::ispTorpId,                  session, root, shipList, g, turn));
    verifyNewNull   (a("ispTorpCount"),               getShipProperty(sh, game::interface::ispTorpCount,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispTorpLCount"),              getShipProperty(sh, game::interface::ispTorpLCount,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispTorpShort"),               getShipProperty(sh, game::interface::ispTorpShort,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispTorpName"),                getShipProperty(sh, game::interface::ispTorpName,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipColonists"),   getShipProperty(sh, game::interface::ispTransferShipColonists,   session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipD"),           getShipProperty(sh, game::interface::ispTransferShipD,           session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipId"),          getShipProperty(sh, game::interface::ispTransferShipId,          session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipM"),           getShipProperty(sh, game::interface::ispTransferShipM,           session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipN"),           getShipProperty(sh, game::interface::ispTransferShipN,           session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipName"),        getShipProperty(sh, game::interface::ispTransferShipName,        session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipSupplies"),    getShipProperty(sh, game::interface::ispTransferShipSupplies,    session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShipT"),           getShipProperty(sh, game::interface::ispTransferShipT,           session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferShip"),            getShipProperty(sh, game::interface::ispTransferShip,            session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadColonists"), getShipProperty(sh, game::interface::ispTransferUnloadColonists, session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadD"),         getShipProperty(sh, game::interface::ispTransferUnloadD,         session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadId"),        getShipProperty(sh, game::interface::ispTransferUnloadId,        session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadM"),         getShipProperty(sh, game::interface::ispTransferUnloadM,         session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadN"),         getShipProperty(sh, game::interface::ispTransferUnloadN,         session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadName"),      getShipProperty(sh, game::interface::ispTransferUnloadName,      session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadSupplies"),  getShipProperty(sh, game::interface::ispTransferUnloadSupplies,  session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnloadT"),         getShipProperty(sh, game::interface::ispTransferUnloadT,         session, root, shipList, g, turn));
    verifyNewNull   (a("ispTransferUnload"),          getShipProperty(sh, game::interface::ispTransferUnload,          session, root, shipList, g, turn));
    verifyNewNull   (a("ispTypeChar"),                getShipProperty(sh, game::interface::ispTypeChar,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispTypeStr"),                 getShipProperty(sh, game::interface::ispTypeStr,                 session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointDistance"),        getShipProperty(sh, game::interface::ispWaypointDistance,        session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointDX"),              getShipProperty(sh, game::interface::ispWaypointDX,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointDY"),              getShipProperty(sh, game::interface::ispWaypointDY,              session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointPlanetId"),        getShipProperty(sh, game::interface::ispWaypointPlanetId,        session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointX"),               getShipProperty(sh, game::interface::ispWaypointX,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointY"),               getShipProperty(sh, game::interface::ispWaypointY,               session, root, shipList, g, turn));
    verifyNewNull   (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn));

    verifyNewNull   (a("ispMessages"),                getShipProperty(sh, game::interface::ispMessages,                session, root, shipList, g, turn));

    // Writable properties
    {
        // Cannot change fcode
        afl::data::StringValue sv("qrs");
        AFL_CHECK_THROWS(a("set ispFCode"), setShipProperty(sh, game::interface::ispFCode, &sv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }
}

/** Test case for a freighter.
    Ship is part of a fleet. */
AFL_TEST("game.interface.ShipProperty:freighter", a)
{
    const int PLAYER = 3;
    const int SHIP_ID = 77;
    const int LEADER_ID = 333;
    const int X = 1100;
    const int Y = 1300;
    const int TARGET_ID = 111;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // - standard data
    game::test::addOutrider(*shipList);
    game::test::addTranswarp(*shipList);

    // - mission definition
    game::spec::Mission msn(8, "!is*,Intercept");
    msn.setShortName("xcept");
    shipList->missions().addMission(msn);

    // Game/Turn
    afl::base::Ref<game::Game> g(*new game::Game());
    afl::base::Ref<game::Turn> turn(g->currentTurn());
    g->setViewpointPlayer(PLAYER);

    // Ship under test
    game::map::ShipData sd;
    sd.owner                     = PLAYER;
    sd.friendlyCode              = "xxy";
    sd.warpFactor                = 7;
    sd.waypointDX                = 0;
    sd.waypointDY                = 0;
    sd.x                         = X;
    sd.y                         = Y;
    sd.engineType                = 9;
    sd.hullType                  = game::test::OUTRIDER_HULL_ID;
    sd.beamType                  = 0;
    sd.numBeams                  = 0;
    sd.numBays                   = 0;
    sd.torpedoType               = 0;
    sd.ammo                      = 0;
    sd.numLaunchers              = 0;
    sd.mission                   = 8;
    sd.primaryEnemy              = 0;
    sd.missionTowParameter       = 0;
    sd.damage                    = 0;
    sd.crew                      = 10;
    sd.colonists                 = 0;
    sd.name                      = "Trolley";
    sd.neutronium                = 10;
    sd.tritanium                 = 0;
    sd.duranium                  = 0;
    sd.molybdenum                = 0;
    sd.supplies                  = 0;
    sd.missionInterceptParameter = TARGET_ID;
    sd.money                     = 0;

    // Create ship. Must be part of the universe because MovementPredictor resolves it through it.
    game::map::Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.setPlayability(game::map::Object::Playable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);
    sh.setFleetNumber(LEADER_ID);

    // Fleet leader
    game::map::Ship& leader = *turn->universe().ships().create(LEADER_ID);
    leader.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    leader.setPlayability(game::map::Object::Playable);
    leader.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);
    leader.setFleetNumber(LEADER_ID);
    leader.setName("Follow me");

    // A ship as target for intercept mission
    addShipXY(session, *g, TARGET_ID, X, Y, PLAYER+1, PLAYER, "USS Far");

    // Test reading all scalar properties
    verifyNewNull   (a("ispAuxId"),                   getShipProperty(sh, game::interface::ispAuxId,                   session, root, shipList, g, turn));
    verifyNewInteger(a("ispAuxAmmo"),                 getShipProperty(sh, game::interface::ispAuxAmmo,                 session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispAuxCount"),                getShipProperty(sh, game::interface::ispAuxCount,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxShort"),                getShipProperty(sh, game::interface::ispAuxShort,                session, root, shipList, g, turn));
    verifyNewNull   (a("ispAuxName"),                 getShipProperty(sh, game::interface::ispAuxName,                 session, root, shipList, g, turn));
    verifyNewInteger(a("ispBeamId"),                  getShipProperty(sh, game::interface::ispBeamId,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispBeamCount"),               getShipProperty(sh, game::interface::ispBeamCount,               session, root, shipList, g, turn), 0);
    verifyNewString (a("ispBeamShort"),               getShipProperty(sh, game::interface::ispBeamShort,               session, root, shipList, g, turn), "");
    verifyNewString (a("ispBeamName"),                getShipProperty(sh, game::interface::ispBeamName,                session, root, shipList, g, turn), "");
    verifyNewInteger(a("ispCargoColonists"),          getShipProperty(sh, game::interface::ispCargoColonists,          session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCargoD"),                  getShipProperty(sh, game::interface::ispCargoD,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCargoFree"),               getShipProperty(sh, game::interface::ispCargoFree,               session, root, shipList, g, turn), 40);
    verifyNewInteger(a("ispCargoM"),                  getShipProperty(sh, game::interface::ispCargoM,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCargoMoney"),              getShipProperty(sh, game::interface::ispCargoMoney,              session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCargoN"),                  getShipProperty(sh, game::interface::ispCargoN,                  session, root, shipList, g, turn), 10);
    verifyNewString (a("ispCargoStr"),                getShipProperty(sh, game::interface::ispCargoStr,                session, root, shipList, g, turn), "10N");
    verifyNewInteger(a("ispCargoSupplies"),           getShipProperty(sh, game::interface::ispCargoSupplies,           session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCargoT"),                  getShipProperty(sh, game::interface::ispCargoT,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispCrew"),                    getShipProperty(sh, game::interface::ispCrew,                    session, root, shipList, g, turn), 10);
    verifyNewInteger(a("ispDamage"),                  getShipProperty(sh, game::interface::ispDamage,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispEnemyId"),                 getShipProperty(sh, game::interface::ispEnemyId,                 session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispEngineId"),                getShipProperty(sh, game::interface::ispEngineId,                session, root, shipList, g, turn), 9);
    verifyNewString (a("ispEngineName"),              getShipProperty(sh, game::interface::ispEngineName,              session, root, shipList, g, turn), "Transwarp Drive");
    verifyNewString (a("ispFCode"),                   getShipProperty(sh, game::interface::ispFCode,                   session, root, shipList, g, turn), "xxy");
    verifyNewInteger(a("ispFighterBays"),             getShipProperty(sh, game::interface::ispFighterBays,             session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispFighterCount"),            getShipProperty(sh, game::interface::ispFighterCount,            session, root, shipList, g, turn));
    verifyNewInteger(a("ispFleetId"),                 getShipProperty(sh, game::interface::ispFleetId,                 session, root, shipList, g, turn), LEADER_ID);
    verifyNewString (a("ispFleetName"),               getShipProperty(sh, game::interface::ispFleetName,               session, root, shipList, g, turn), "");
    verifyNewString (a("ispFleetStatus"),             getShipProperty(sh, game::interface::ispFleetStatus,             session, root, shipList, g, turn), "member");
    verifyNewString (a("ispFleet"),                   getShipProperty(sh, game::interface::ispFleet,                   session, root, shipList, g, turn), "Ship #333: Follow me");
    verifyNewNull   (a("ispHeadingAngle"),            getShipProperty(sh, game::interface::ispHeadingAngle,            session, root, shipList, g, turn));
    verifyNewNull   (a("ispHeadingName"),             getShipProperty(sh, game::interface::ispHeadingName,             session, root, shipList, g, turn));
    verifyNewString (a("ispHullSpecial"),             getShipProperty(sh, game::interface::ispHullSpecial,             session, root, shipList, g, turn), "");
    verifyNewInteger(a("ispId"),                      getShipProperty(sh, game::interface::ispId,                      session, root, shipList, g, turn), SHIP_ID);
    verifyNewNull   (a("ispLevel"),                   getShipProperty(sh, game::interface::ispLevel,                   session, root, shipList, g, turn));
    verifyNewInteger(a("ispLocX"),                    getShipProperty(sh, game::interface::ispLocX,                    session, root, shipList, g, turn), X);
    verifyNewInteger(a("ispLocY"),                    getShipProperty(sh, game::interface::ispLocY,                    session, root, shipList, g, turn), Y);
    verifyNewString (a("ispLoc"),                     getShipProperty(sh, game::interface::ispLoc,                     session, root, shipList, g, turn), "(1100,1300)");
    verifyNewBoolean(a("ispMarked"),                  getShipProperty(sh, game::interface::ispMarked,                  session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispMass"),                    getShipProperty(sh, game::interface::ispMass,                    session, root, shipList, g, turn), 85);
    verifyNewInteger(a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn), 8);
    verifyNewInteger(a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn), TARGET_ID);
    verifyNewString (a("ispMissionShort"),            getShipProperty(sh, game::interface::ispMissionShort,            session, root, shipList, g, turn), "xcept");
    verifyNewInteger(a("ispMissionTow"),              getShipProperty(sh, game::interface::ispMissionTow,              session, root, shipList, g, turn), 0);
    verifyNewString (a("ispMissionName"),             getShipProperty(sh, game::interface::ispMissionName,             session, root, shipList, g, turn), "Intercept");
    verifyNewInteger(a("ispMoveETA"),                 getShipProperty(sh, game::interface::ispMoveETA,                 session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispMoveFuel"),                getShipProperty(sh, game::interface::ispMoveFuel,                session, root, shipList, g, turn), 0);
    verifyNewString (a("ispName"),                    getShipProperty(sh, game::interface::ispName,                    session, root, shipList, g, turn), "Trolley");
    verifyNewInteger(a("ispOrbitId"),                 getShipProperty(sh, game::interface::ispOrbitId,                 session, root, shipList, g, turn), 0);
    verifyNewNull   (a("ispOrbitName"),               getShipProperty(sh, game::interface::ispOrbitName,               session, root, shipList, g, turn));
    verifyNewBoolean(a("ispPlayed"),                  getShipProperty(sh, game::interface::ispPlayed,                  session, root, shipList, g, turn), true);
    verifyNewInteger(a("ispRealOwner"),               getShipProperty(sh, game::interface::ispRealOwner,               session, root, shipList, g, turn), PLAYER);
    verifyNewInteger(a("ispSpeedId"),                 getShipProperty(sh, game::interface::ispSpeedId,                 session, root, shipList, g, turn), 7);
    verifyNewString (a("ispSpeedName"),               getShipProperty(sh, game::interface::ispSpeedName,               session, root, shipList, g, turn), "Warp 7");
    verifyNewBoolean(a("ispTask"),                    getShipProperty(sh, game::interface::ispTask,                    session, root, shipList, g, turn), false);
    verifyNewInteger(a("ispTorpId"),                  getShipProperty(sh, game::interface::ispTorpId,                  session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTorpCount"),               getShipProperty(sh, game::interface::ispTorpCount,               session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispTorpLCount"),              getShipProperty(sh, game::interface::ispTorpLCount,              session, root, shipList, g, turn), 0);
    verifyNewString (a("ispTorpShort"),               getShipProperty(sh, game::interface::ispTorpShort,               session, root, shipList, g, turn), "");
    verifyNewString (a("ispTorpName"),                getShipProperty(sh, game::interface::ispTorpName,                session, root, shipList, g, turn), "");
    // Skip checking the ispTransferXxx, we have not initialized those
    verifyNewString (a("ispTypeChar"),                getShipProperty(sh, game::interface::ispTypeChar,                session, root, shipList, g, turn), "F");
    verifyNewString (a("ispTypeStr"),                 getShipProperty(sh, game::interface::ispTypeStr,                 session, root, shipList, g, turn), "Freighter");
    verifyNewFloat  (a("ispWaypointDistance"),        getShipProperty(sh, game::interface::ispWaypointDistance,        session, root, shipList, g, turn), 0.0, 0.001);
    verifyNewInteger(a("ispWaypointDX"),              getShipProperty(sh, game::interface::ispWaypointDX,              session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointDY"),              getShipProperty(sh, game::interface::ispWaypointDY,              session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointPlanetId"),        getShipProperty(sh, game::interface::ispWaypointPlanetId,        session, root, shipList, g, turn), 0);
    verifyNewInteger(a("ispWaypointX"),               getShipProperty(sh, game::interface::ispWaypointX,               session, root, shipList, g, turn), X);
    verifyNewInteger(a("ispWaypointY"),               getShipProperty(sh, game::interface::ispWaypointY,               session, root, shipList, g, turn), Y);
    verifyNewString (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn), "USS Far (#111)");

    // Writing properties
    {
        // Cannot change speed or mission, is controlled by fleet leader
        afl::data::IntegerValue iv(3);
        AFL_CHECK_THROWS(a("set ispSpeedId"),          setShipProperty(sh, game::interface::ispSpeedId,          &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), game::Exception);
        AFL_CHECK_THROWS(a("set ispMissionId"),        setShipProperty(sh, game::interface::ispMissionId,        &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), game::Exception);
        AFL_CHECK_THROWS(a("set ispMissionIntercept"), setShipProperty(sh, game::interface::ispMissionIntercept, &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), game::Exception);
        AFL_CHECK_THROWS(a("set ispMissionTow"),       setShipProperty(sh, game::interface::ispMissionTow,       &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), game::Exception);
    }
    {
        // Cannot change fleet number to unrelated ship
        afl::data::IntegerValue iv(TARGET_ID);
        AFL_CHECK_THROWS(a("set ispFleetId"),          setShipProperty(sh, game::interface::ispFleetId,          &iv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }
    {
        // Cannot change fleet name
        afl::data::StringValue sv("name");
        AFL_CHECK_THROWS(a("set ispFleetName"),        setShipProperty(sh, game::interface::ispFleetName,        &sv, *root, *shipList, g->mapConfiguration(), turn->universe()), interpreter::Error);
    }
}

/** Test intercept usecases. */
AFL_TEST("game.interface.ShipProperty:intercept", a)
{
    const int PLAYER = 3;
    const int SHIP_ID = 77;
    const int NAMED_ID = 20;
    const int UNNAMED_ID = 30;
    const int X = 1100;
    const int Y = 1300;

    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    game::Session session(tx, fs);

    // Root
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,1,0))));

    // Ship List
    afl::base::Ref<game::spec::ShipList> shipList(*new game::spec::ShipList());

    // Game/Turn
    afl::base::Ref<game::Game> g(*new game::Game());
    afl::base::Ref<game::Turn> turn(g->currentTurn());
    g->setViewpointPlayer(PLAYER);

    // Ship under test
    game::map::ShipData sd;
    sd.owner                     = PLAYER;
    sd.waypointDX                = 10;
    sd.waypointDY                = 10;
    sd.x                         = X;
    sd.y                         = Y;
    sd.hullType                  = game::test::GORBIE_HULL_ID;
    sd.mission                   = 8;
    sd.missionTowParameter       = 10;
    sd.missionInterceptParameter = NAMED_ID;

    // Create ship. Must be part of the universe because MovementPredictor resolves it through it.
    game::map::Ship& sh = *turn->universe().ships().create(SHIP_ID);
    sh.addCurrentShipData(sd, game::PlayerSet_t(PLAYER));
    sh.setPlayability(game::map::Object::Playable);
    sh.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    // Target ships
    game::map::Ship& named = *turn->universe().ships().create(NAMED_ID);
    named.addShipXYData(game::map::Point(X+100, Y), PLAYER+1, 100, game::PlayerSet_t(PLAYER));
    named.setName("Named");
    named.setPlayability(game::map::Object::NotPlayable);
    named.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    game::map::Ship& unnamed = *turn->universe().ships().create(UNNAMED_ID);
    unnamed.addShipXYData(game::map::Point(X, Y+100), PLAYER+2, 100, game::PlayerSet_t(PLAYER));
    unnamed.setPlayability(game::map::Object::NotPlayable);
    unnamed.internalCheck(game::PlayerSet_t(PLAYER), TURN_NR);

    // Initial state: intercepting NAMED_ID
    verifyNewInteger(a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn), 8);
    verifyNewInteger(a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn), NAMED_ID);
    verifyNewString (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn), "Named (#20)");

    // Modify target
    afl::data::IntegerValue iv(UNNAMED_ID);
    AFL_CHECK_SUCCEEDS(a("set ispMissionIntercept"), setShipProperty(sh, game::interface::ispMissionIntercept, &iv, *root, *shipList, g->mapConfiguration(), turn->universe()));

    // Initial state: intercepting UNNAMED_ID
    verifyNewInteger(a("ispMissionId"),               getShipProperty(sh, game::interface::ispMissionId,               session, root, shipList, g, turn), 8);
    verifyNewInteger(a("ispMissionIntercept"),        getShipProperty(sh, game::interface::ispMissionIntercept,        session, root, shipList, g, turn), UNNAMED_ID);
    verifyNewString (a("ispWaypointName"),            getShipProperty(sh, game::interface::ispWaypointName,            session, root, shipList, g, turn), "Ship #30");
}
