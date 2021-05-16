/**
  *  \file u/t_game_actions_changebuildqueue.cpp
  *  \brief Test for game::actions::ChangeBuildQueue
  */

#include "game/actions/changebuildqueue.hpp"

#include "t_game_actions.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"
#include "game/session.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

namespace {
    using afl::string::Format;
    using game::Element;
    using game::HostVersion;
    using game::map::Planet;
    using game::map::Ship;

    const int PLAYER = 9;

    struct Environment {
        game::map::Universe univ;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion host;
        util::RandomNumberGenerator rng;

        Environment()
            : univ(), shipList(), config(), host(HostVersion::PHost, MKVERSION(3,4,0)), rng(32)
            { }
    };

    void init(game::spec::ShipList& shipList)
    {
        // 10 hulls
        for (int i = 1; i <= 10; ++i) {
            game::spec::Hull* pHull = shipList.hulls().create(i);
            TS_ASSERT(pHull != 0);
            pHull->setName(Format("Hull %d", i));
            pHull->setMass(100);
            pHull->setNumEngines(1);

            shipList.hullAssignments().add(PLAYER, i, i);
        }

        // 9 engines
        for (int i = 1; i <= 9; ++i) {
            shipList.engines().create(i);
        }
    }

    void init(Environment& env)
    {
        init(env.shipList);
    }

    Planet& addPlanet(game::map::Universe& univ, game::Id_t planetId, int player, String_t fc)
    {
        Planet* p = univ.planets().create(planetId);
        TS_ASSERT(p != 0);

        game::map::PlanetData pd;
        pd.owner = player;
        pd.colonistClans = 100;
        pd.friendlyCode = fc;
        p->addCurrentPlanetData(pd, game::PlayerSet_t(player));
        p->setPosition(game::map::Point(1000 + planetId, 2000));


        game::map::BaseData bd;
        bd.shipBuildOrder.setHullIndex(1);
        bd.shipBuildOrder.setEngineType(1);
        bd.hullStorage.set(1, 100);
        bd.engineStorage.set(1, 100);
        p->addCurrentBaseData(bd, game::PlayerSet_t(player));

        return *p;
    }

    Planet& addPlanet(Environment& env, game::Id_t planetId, int player, String_t fc)
    {
        return addPlanet(env.univ, planetId, player, fc);
    }

    void addDefaultPlanets(Environment& env)
    {
        addPlanet(env, 1, PLAYER, "xyz");
        addPlanet(env, 2, PLAYER, "PB2");
        addPlanet(env, 3, PLAYER, "PB1");
        addPlanet(env, 4, PLAYER, "xyz");
        addPlanet(env, 5, PLAYER, "PB3");
        addPlanet(env, 6, PLAYER, "PB3");
    }

    Ship& addShip(Environment& env, game::Id_t shipId, int player, game::Id_t planetId, String_t fc)
    {
        Planet* p = env.univ.planets().get(planetId);
        TS_ASSERT(p != 0);
        game::map::Point pt;
        TS_ASSERT(p->getPosition(pt));

        Ship* sh = env.univ.ships().create(shipId);
        TS_ASSERT(sh != 0);

        game::map::ShipData sd;
        sd.owner = player;
        sd.friendlyCode = fc;
        sd.x = pt.getX();
        sd.y = pt.getY();
        sd.waypointDX = 0;
        sd.waypointDY = 0;
        sd.engineType = 9;
        sd.hullType = 1;
        sh->addCurrentShipData(sd, game::PlayerSet_t(player));

        return *sh;
    }

    void finish(Environment& env)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::PlayerSet_t p(PLAYER);
        env.univ.postprocess(p, p, game::map::Object::Playable, env.host, env.config, 77, env.shipList, tx, log);
    }
}


/** Basic test.
    Set up a standard situation and verify that it is parsed correctly. */
void
TestGameActionsChangeBuildQueue::testBasic()
{
    // Prepare
    Environment env;
    init(env);
    addDefaultPlanets(env);
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    // - order
    TS_ASSERT_EQUALS(infos.size(), 6U);
    TS_ASSERT_EQUALS(infos[0].planetId, 3);  // PB1
    TS_ASSERT_EQUALS(infos[1].planetId, 2);  // PB2
    TS_ASSERT_EQUALS(infos[2].planetId, 5);  // PB3
    TS_ASSERT_EQUALS(infos[3].planetId, 6);  // PB3
    TS_ASSERT_EQUALS(infos[4].planetId, 1);  // xyz
    TS_ASSERT_EQUALS(infos[5].planetId, 4);  // xyz

    // - warning status
    TS_ASSERT_EQUALS(infos[0].conflict, false);
    TS_ASSERT_EQUALS(infos[1].conflict, false);
    TS_ASSERT_EQUALS(infos[2].conflict, false);
    TS_ASSERT_EQUALS(infos[3].conflict, true);     // slot 3 = planet 6 clashes with slot 2 = planet 5
    TS_ASSERT_EQUALS(infos[4].conflict, false);
    TS_ASSERT_EQUALS(infos[5].conflict, false);

    // - build points
    TS_ASSERT_EQUALS(infos[0].pointsRequired.isValid(), false);   // not set because not PBP queue
    TS_ASSERT_EQUALS(infos[0].pointsAvailable.isValid(), false);  // not set because not known
}

/** Test increasePriority().
    Set up a standard situation and call increasePriority().
    Verify that correct order/codes are generated. */
void
TestGameActionsChangeBuildQueue::testIncrease()
{
    // Prepare
    Environment env;
    init(env);
    addDefaultPlanets(env);
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    testee.increasePriority(2);
    testee.increasePriority(5);

    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    // - order
    TS_ASSERT_EQUALS(infos.size(), 6U);
    TS_ASSERT_EQUALS(infos[0].planetId, 3);  // PB1
    TS_ASSERT_EQUALS(infos[1].planetId, 5);  // PB2 (moved up)
    TS_ASSERT_EQUALS(infos[2].planetId, 2);  // PB2 (moved down)
    TS_ASSERT_EQUALS(infos[3].planetId, 6);  // PB3
    TS_ASSERT_EQUALS(infos[4].planetId, 4);  // PB4 (moved up)
    TS_ASSERT_EQUALS(infos[5].planetId, 1);  // xyz

    // Commit and verify
    testee.commit();
    TS_ASSERT_EQUALS(env.univ.planets().get(5)->getFriendlyCode().orElse(""), "PB2");
    TS_ASSERT_EQUALS(env.univ.planets().get(4)->getFriendlyCode().orElse(""), "PB4");
}

/** Test decreasePriority().
    Set up a standard situation and call decreasePriority().
    Verify that correct order/codes are generated. */
void
TestGameActionsChangeBuildQueue::testDecrease()
{
    // Prepare
    Environment env;
    init(env);
    addDefaultPlanets(env);
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    testee.decreasePriority(2);

    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    // - order
    TS_ASSERT_EQUALS(infos.size(), 6U);
    TS_ASSERT_EQUALS(infos[0].planetId, 3);  // PB1
    TS_ASSERT_EQUALS(infos[1].planetId, 2);  // PB2
    TS_ASSERT_EQUALS(infos[2].planetId, 6);  // PB3
    TS_ASSERT_EQUALS(infos[3].planetId, 5);  // PB4 (moved down)
    TS_ASSERT_EQUALS(infos[4].planetId, 1);  // xyz
    TS_ASSERT_EQUALS(infos[5].planetId, 4);  // xyz

    // Commit and verify
    testee.commit();
    TS_ASSERT_EQUALS(env.univ.planets().get(5)->getFriendlyCode().orElse(""), "PB4");
}

/** Test PBP computations.
    Set up a standard situation, for a THost game (= with PBPs).
    Verify that correct point counts are generated. */
void
TestGameActionsChangeBuildQueue::testPBPs()
{
    // Prepare
    Environment env;
    init(env);
    addDefaultPlanets(env);
    env.host = HostVersion(game::HostVersion::Host, MKVERSION(3,22,40));
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    afl::string::NullTranslator tx;

    // Verify initial build points
    {
        game::actions::ChangeBuildQueue::Infos_t infos;
        testee.describe(infos, tx);

        // pointsRequired is known, 100 kt costs 2 points to build
        TS_ASSERT_EQUALS(infos[0].pointsRequired.isValid(), true);
        TS_ASSERT_EQUALS(infos[0].pointsRequired.orElse(0), 2);

        // pointsAvailable not known
        TS_ASSERT_EQUALS(infos[0].pointsAvailable.isValid(), false);
    }

    // Set build points and retry
    testee.setAvailableBuildPoints(5);
    {
        game::actions::ChangeBuildQueue::Infos_t infos;
        testee.describe(infos, tx);

        TS_ASSERT_EQUALS(infos[0].pointsRequired.orElse(0), 2);
        TS_ASSERT_EQUALS(infos[1].pointsRequired.orElse(0), 2);
        TS_ASSERT_EQUALS(infos[2].pointsRequired.orElse(0), 2);
        TS_ASSERT_EQUALS(infos[3].pointsRequired.orElse(0), 2);
        TS_ASSERT_EQUALS(infos[0].pointsAvailable.orElse(-1), 5);
        TS_ASSERT_EQUALS(infos[1].pointsAvailable.orElse(-1), 3);
        TS_ASSERT_EQUALS(infos[2].pointsAvailable.orElse(-1), 1);
        TS_ASSERT_EQUALS(infos[3].pointsAvailable.orElse(-1), 0);
    }
}

/** Test cloning.
    Set up a situation including a cloning ship.
    Verify that correct labels are generated. */
void
TestGameActionsChangeBuildQueue::testClone()
{
    // Prepare
    Environment env;
    init(env);
    addDefaultPlanets(env);
    addShip(env, 34, PLAYER, 2, "cln")
        .setName("NSEA Protector");
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    TS_ASSERT_EQUALS(infos.size(), 6U);
    TS_ASSERT_EQUALS(infos[0].actionName, "Build Hull 1");
    TS_ASSERT_EQUALS(infos[1].actionName, "Clone NSEA Protector");
    TS_ASSERT_EQUALS(infos[2].actionName, "Build Hull 1");
}

/** Test planned build.
    Set up a situation with normal and planned builds (auto tasks).
    Verify correct result. */
void
TestGameActionsChangeBuildQueue::testPlannedBuild()
{
    // This needs a Session to be able to set up an auto-task!
    // Therefore, set up by hand.
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    session.setRoot(new game::test::Root(game::HostVersion()));
    session.setShipList(new game::spec::ShipList());
    init(*session.getShipList());

    session.setGame(new game::Game());
    game::map::Universe& univ = session.getGame()->currentTurn().universe();
    addPlanet(univ,  6, PLAYER, "xyz");
    addPlanet(univ, 10, PLAYER, "abc");
    addPlanet(univ, 20, PLAYER, "xyz");
    univ.postprocess(game::PlayerSet_t(PLAYER), game::PlayerSet_t(PLAYER), game::map::Object::Playable,
                     session.getRoot()->hostVersion(), session.getRoot()->hostConfiguration(),
                     77, *session.getShipList(), session.translator(), session.log());

    // Cancel planet 10's build order and give him an auto-task instead
    univ.planets().get(10)->setBaseBuildOrder(game::ShipBuildOrder());
    afl::base::Ptr<interpreter::TaskEditor> ed(session.getAutoTaskEditor(10, interpreter::Process::pkBaseTask, true));
    TS_ASSERT(ed.get() != 0);
    String_t commands[] = {
        "enqueueship 3,8",
        "enqueueship 4,7",
        "enqueueship 5,6",
    };
    ed->addAtEnd(commands);
    ed->setPC(1);

    // Test
    game::actions::ChangeBuildQueue testee(univ, *session.getShipList(), session.getRoot()->hostVersion(), session.getRoot()->hostConfiguration(), session.rng(), PLAYER);
    testee.addPlannedBuilds(session.processList());
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    TS_ASSERT_EQUALS(infos.size(), 3U);
    TS_ASSERT_EQUALS(infos[0].actionName, "Build Hull 1");
    TS_ASSERT_EQUALS(infos[1].actionName, "Build Hull 1");
    TS_ASSERT_EQUALS(infos[2].actionName, "Plan Hull 4");
    TS_ASSERT_EQUALS(infos[0].planetId, 6);
    TS_ASSERT_EQUALS(infos[1].planetId, 20);
    TS_ASSERT_EQUALS(infos[2].planetId, 10);
}

