/**
  *  \file test/game/actions/changebuildqueuetest.cpp
  *  \brief Test for game::actions::ChangeBuildQueue
  */

#include "game/actions/changebuildqueue.hpp"

#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/configuration.hpp"
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
        game::map::Configuration mapConfig;
        game::spec::ShipList shipList;
        game::config::HostConfiguration config;
        game::HostVersion host;
        util::RandomNumberGenerator rng;

        Environment()
            : univ(), mapConfig(), shipList(), config(), host(HostVersion::PHost, MKVERSION(3,4,0)), rng(32)
            { }
    };

    void init(afl::test::Assert a, game::spec::ShipList& shipList)
    {
        // 10 hulls
        for (int i = 1; i <= 10; ++i) {
            game::spec::Hull* pHull = shipList.hulls().create(i);
            a.checkNonNull("hulls().create", pHull);
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

    void init(afl::test::Assert a, Environment& env)
    {
        init(a, env.shipList);
    }

    Planet& addPlanet(afl::test::Assert a, game::map::Universe& univ, game::Id_t planetId, int player, String_t fc)
    {
        Planet* p = univ.planets().create(planetId);
        a.checkNonNull("planets().create", p);

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

    Planet& addPlanet(afl::test::Assert a, Environment& env, game::Id_t planetId, int player, String_t fc)
    {
        return addPlanet(a, env.univ, planetId, player, fc);
    }

    void addDefaultPlanets(afl::test::Assert a, Environment& env)
    {
        addPlanet(a("p1"), env, 1, PLAYER, "xyz");
        addPlanet(a("p2"), env, 2, PLAYER, "PB2");
        addPlanet(a("p3"), env, 3, PLAYER, "PB1");
        addPlanet(a("p4"), env, 4, PLAYER, "xyz");
        addPlanet(a("p5"), env, 5, PLAYER, "PB3");
        addPlanet(a("p6"), env, 6, PLAYER, "PB3");
    }

    Ship& addShip(afl::test::Assert a, Environment& env, game::Id_t shipId, int player, game::Id_t planetId, String_t fc)
    {
        Planet* p = env.univ.planets().get(planetId);
        a.checkNonNull("planets().get", p);
        game::map::Point pt;
        a.check("planet getPosition", p->getPosition().get(pt));

        Ship* sh = env.univ.ships().create(shipId);
        a.checkNonNull("ships().create", sh);

        game::map::ShipData sd;
        sd.owner = player;
        sd.friendlyCode = fc;
        sd.x = pt.getX();
        sd.y = pt.getY();
        sd.waypointDX = 0;
        sd.waypointDY = 0;
        sd.engineType = 9;
        sd.hullType = 7;
        sh->addCurrentShipData(sd, game::PlayerSet_t(player));

        return *sh;
    }

    void finish(Environment& env)
    {
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        game::PlayerSet_t p(PLAYER);
        env.univ.postprocess(p, p, game::map::Object::Playable, env.mapConfig, env.host, env.config, 77, env.shipList, tx, log);
    }
}


/** Basic test.
    Set up a standard situation and verify that it is parsed correctly. */
AFL_TEST("game.actions.ChangeBuildQueue:init", a)
{
    // Prepare
    Environment env;
    init(a, env);
    addDefaultPlanets(a, env);
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    // - order
    a.checkEqual("01. size", infos.size(), 6U);
    a.checkEqual("02. planetId", infos[0].planetId, 3);  // PB1
    a.checkEqual("03. planetId", infos[1].planetId, 2);  // PB2
    a.checkEqual("04. planetId", infos[2].planetId, 5);  // PB3
    a.checkEqual("05. planetId", infos[3].planetId, 6);  // PB3
    a.checkEqual("06. planetId", infos[4].planetId, 1);  // xyz
    a.checkEqual("07. planetId", infos[5].planetId, 4);  // xyz

    // - warning status
    a.checkEqual("11. conflict", infos[0].conflict, false);
    a.checkEqual("12. conflict", infos[1].conflict, false);
    a.checkEqual("13. conflict", infos[2].conflict, false);
    a.checkEqual("14. conflict", infos[3].conflict, true);     // slot 3 = planet 6 clashes with slot 2 = planet 5
    a.checkEqual("15. conflict", infos[4].conflict, false);
    a.checkEqual("16. conflict", infos[5].conflict, false);

    // - change status
    for (size_t i = 0; i < 6; ++i) {
        a.checkEqual("21. isChange", infos[i].isChange, false);
    }

    // - build points
    a.checkEqual("31. pointsRequired", infos[0].pointsRequired.isValid(), false);   // not set because not PBP queue
    a.checkEqual("32. pointsAvailable", infos[0].pointsAvailable.isValid(), false);  // not set because not known
}

/** Test increasePriority().
    Set up a standard situation and call increasePriority().
    Verify that correct order/codes are generated. */
AFL_TEST("game.actions.ChangeBuildQueue:increasePriority", a)
{
    // Prepare
    Environment env;
    init(a, env);
    addDefaultPlanets(a, env);
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
    a.checkEqual("01. size", infos.size(), 6U);
    a.checkEqual("02. planetId", infos[0].planetId, 3);  // PB1
    a.checkEqual("03. planetId", infos[1].planetId, 5);  // PB2 (moved up)
    a.checkEqual("04. planetId", infos[2].planetId, 2);  // PB2 (moved down)
    a.checkEqual("05. planetId", infos[3].planetId, 6);  // PB3
    a.checkEqual("06. planetId", infos[4].planetId, 4);  // PB4 (moved up)
    a.checkEqual("07. planetId", infos[5].planetId, 1);  // xyz

    // - change status
    a.checkEqual("11. isChange", infos[0].isChange, false);
    a.checkEqual("12. isChange", infos[1].isChange, true);
    a.checkEqual("13. isChange", infos[2].isChange, true);
    a.checkEqual("14. isChange", infos[3].isChange, false);
    a.checkEqual("15. isChange", infos[4].isChange, true);
    a.checkEqual("16. isChange", infos[5].isChange, false);

    // Commit and verify
    testee.commit();
    a.checkEqual("21. getFriendlyCode", env.univ.planets().get(5)->getFriendlyCode().orElse(""), "PB2");
    a.checkEqual("22. getFriendlyCode", env.univ.planets().get(4)->getFriendlyCode().orElse(""), "PB4");
}

/** Test decreasePriority().
    Set up a standard situation and call decreasePriority().
    Verify that correct order/codes are generated. */
AFL_TEST("game.actions.ChangeBuildQueue:decreasePriority", a)
{
    // Prepare
    Environment env;
    init(a, env);
    addDefaultPlanets(a, env);
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    testee.decreasePriority(2);

    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    // - order
    a.checkEqual("01. size", infos.size(), 6U);
    a.checkEqual("02. planetId", infos[0].planetId, 3);  // PB1
    a.checkEqual("03. planetId", infos[1].planetId, 2);  // PB2
    a.checkEqual("04. planetId", infos[2].planetId, 6);  // PB3
    a.checkEqual("05. planetId", infos[3].planetId, 5);  // PB4 (moved down)
    a.checkEqual("06. planetId", infos[4].planetId, 1);  // xyz
    a.checkEqual("07. planetId", infos[5].planetId, 4);  // xyz

    // - change status
    a.checkEqual("11. isChange", infos[0].isChange, false);
    a.checkEqual("12. isChange", infos[1].isChange, false);
    a.checkEqual("13. isChange", infos[2].isChange, false);
    a.checkEqual("14. isChange", infos[3].isChange, true);
    a.checkEqual("15. isChange", infos[4].isChange, false);
    a.checkEqual("16. isChange", infos[5].isChange, false);

    // Commit and verify
    testee.commit();
    a.checkEqual("21. getFriendlyCode", env.univ.planets().get(5)->getFriendlyCode().orElse(""), "PB4");
}

/** Test PBP computations.
    Set up a standard situation, for a THost game (= with PBPs).
    Verify that correct point counts are generated. */
AFL_TEST("game.actions.ChangeBuildQueue:points", a)
{
    // Prepare
    Environment env;
    init(a, env);
    addDefaultPlanets(a, env);
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
        a.checkEqual("01. pointsRequired", infos[0].pointsRequired.isValid(), true);
        a.checkEqual("02. pointsRequired", infos[0].pointsRequired.orElse(0), 2);

        // pointsAvailable not known
        a.checkEqual("11. pointsAvailable", infos[0].pointsAvailable.isValid(), false);
    }

    // Set build points and retry
    testee.setAvailableBuildPoints(5);
    {
        game::actions::ChangeBuildQueue::Infos_t infos;
        testee.describe(infos, tx);

        a.checkEqual("21. pointsRequired", infos[0].pointsRequired.orElse(0), 2);
        a.checkEqual("22. pointsRequired", infos[1].pointsRequired.orElse(0), 2);
        a.checkEqual("23. pointsRequired", infos[2].pointsRequired.orElse(0), 2);
        a.checkEqual("24. pointsRequired", infos[3].pointsRequired.orElse(0), 2);
        a.checkEqual("25. pointsRequired", infos[0].pointsAvailable.orElse(-1), 5);
        a.checkEqual("26. pointsRequired", infos[1].pointsAvailable.orElse(-1), 3);
        a.checkEqual("27. pointsRequired", infos[2].pointsAvailable.orElse(-1), 1);
        a.checkEqual("28. pointsRequired", infos[3].pointsAvailable.orElse(-1), 0);
    }
}

/** Test cloning.
    Set up a situation including a cloning ship.
    Verify that correct labels are generated. */
AFL_TEST("game.actions.ChangeBuildQueue:clone", a)
{
    // Prepare
    Environment env;
    init(a, env);
    addDefaultPlanets(a, env);
    addShip(a("s34"), env, 34, PLAYER, 2, "cln")
        .setName("NSEA Protector");
    finish(env);

    // Test
    game::actions::ChangeBuildQueue testee(env.univ, env.shipList, env.host, env.config, env.rng, PLAYER);
    afl::string::NullTranslator tx;
    game::actions::ChangeBuildQueue::Infos_t infos;
    testee.describe(infos, tx);

    // Verify
    a.checkEqual("01. size", infos.size(), 6U);
    a.checkEqual("02. actionName", infos[0].actionName, "Build Hull 1");
    a.checkEqual("03. actionName", infos[1].actionName, "Clone NSEA Protector");
    a.checkEqual("04. actionName", infos[2].actionName, "Build Hull 1");
    a.checkEqual("05. action", infos[0].action, game::actions::ChangeBuildQueue::BuildShip);
    a.checkEqual("06. action", infos[1].action, game::actions::ChangeBuildQueue::CloneShip);
    a.checkEqual("07. action", infos[2].action, game::actions::ChangeBuildQueue::BuildShip);
    a.checkEqual("08. hullName", infos[0].hullName, "Hull 1");
    a.checkEqual("09. hullName", infos[1].hullName, "Hull 7");
    a.checkEqual("10. hullName", infos[2].hullName, "Hull 1");
    a.checkEqual("11. hullNr", infos[0].hullNr, 1);
    a.checkEqual("12. hullNr", infos[1].hullNr, 7);
    a.checkEqual("13. hullNr", infos[2].hullNr, 1);
}

/** Test planned build.
    Set up a situation with normal and planned builds (auto tasks).
    Verify correct result. */
AFL_TEST("game.actions.ChangeBuildQueue:planned-build", a)
{
    // This needs a Session to be able to set up an auto-task!
    // Therefore, set up by hand.
    afl::io::NullFileSystem fs;
    afl::string::NullTranslator tx;
    game::Session session(tx, fs);

    session.setRoot(game::test::makeRoot(game::HostVersion()).asPtr());
    session.setShipList(new game::spec::ShipList());
    init(a, *session.getShipList());

    session.setGame(new game::Game());
    game::map::Universe& univ = session.getGame()->currentTurn().universe();
    addPlanet(a("p6"),  univ,  6, PLAYER, "xyz");
    addPlanet(a("p10"), univ, 10, PLAYER, "abc");
    addPlanet(a("p20"), univ, 20, PLAYER, "xyz");
    univ.postprocess(game::PlayerSet_t(PLAYER), game::PlayerSet_t(PLAYER), game::map::Object::Playable,
                     session.getGame()->mapConfiguration(),
                     session.getRoot()->hostVersion(), session.getRoot()->hostConfiguration(),
                     77, *session.getShipList(), session.translator(), session.log());

    // Cancel planet 10's build order and give him an auto-task instead
    univ.planets().get(10)->setBaseBuildOrder(game::ShipBuildOrder());
    afl::base::Ptr<interpreter::TaskEditor> ed(session.getAutoTaskEditor(10, interpreter::Process::pkBaseTask, true));
    a.checkNonNull("01. getAutoTaskEditor", ed.get());
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
    a.checkEqual("11. size", infos.size(), 3U);
    a.checkEqual("12. actionName", infos[0].actionName, "Build Hull 1");
    a.checkEqual("13. actionName", infos[1].actionName, "Build Hull 1");
    a.checkEqual("14. actionName", infos[2].actionName, "Plan Hull 4");
    a.checkEqual("15. planetId", infos[0].planetId, 6);
    a.checkEqual("16. planetId", infos[1].planetId, 20);
    a.checkEqual("17. planetId", infos[2].planetId, 10);
    a.checkEqual("18. action", infos[0].action, game::actions::ChangeBuildQueue::BuildShip);
    a.checkEqual("19. action", infos[1].action, game::actions::ChangeBuildQueue::BuildShip);
    a.checkEqual("20. action", infos[2].action, game::actions::ChangeBuildQueue::PlanShip);
    a.checkEqual("21. hullName", infos[0].hullName, "Hull 1");
    a.checkEqual("22. hullName", infos[1].hullName, "Hull 1");
    a.checkEqual("23. hullName", infos[2].hullName, "Hull 4");
    a.checkEqual("24. hullNr", infos[0].hullNr, 1);
    a.checkEqual("25. hullNr", infos[1].hullNr, 1);
    a.checkEqual("26. hullNr", infos[2].hullNr, 4);
}
