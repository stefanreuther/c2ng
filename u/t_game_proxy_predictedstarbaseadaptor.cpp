/**
  *  \file u/t_game_proxy_predictedstarbaseadaptor.cpp
  *  \brief Test for game::proxy::PredictedStarbaseAdaptor
  */

#include "game/proxy/predictedstarbaseadaptor.hpp"

#include "t_game_proxy.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/exception.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/turn.hpp"

using afl::io::NullFileSystem;
using afl::string::NullTranslator;
using game::Session;
using interpreter::Process;
using interpreter::TaskEditor;

namespace {
    const int PLAYER_NR = 3;
    const int PLANET_ID = 55;
    const int HULL_NR = 12;
    const int HULL_SLOT = 3;
    const int ENGINE_NR = 4;

    void prepare(Session& session)
    {
        // Create root
        session.setRoot(new game::test::Root(game::HostVersion()));

        // Create ship list
        afl::base::Ptr<game::spec::ShipList> sl(new game::spec::ShipList());
        game::spec::Hull* h = sl->hulls().create(HULL_NR);
        h->setMaxBeams(10);
        h->setMaxLaunchers(10);
        h->setNumEngines(2);
        sl->hullAssignments().add(PLAYER_NR, HULL_SLOT, HULL_NR);
        sl->engines().create(ENGINE_NR);
        session.setShipList(sl);

        // Create game
        session.setGame(new game::Game());

        // Add a planet
        // - main
        game::map::Planet* pl = session.getGame()->currentTurn().universe().planets().create(PLANET_ID);
        game::map::PlanetData pd;
        pd.owner = PLAYER_NR;
        pl->addCurrentPlanetData(pd, game::PlayerSet_t(PLAYER_NR));

        // - base data
        game::map::BaseData bd;
        bd.engineStorage.set(9, 0);
        bd.beamStorage.set(0, 0);
        bd.launcherStorage.set(0, 0);
        bd.hullStorage.set(0, 0);
        bd.engineStorage.set(ENGINE_NR, 3);
        bd.hullStorage.set(HULL_SLOT, 4);
        pl->addCurrentBaseData(bd, game::PlayerSet_t(PLAYER_NR));

        // - meta info
        pl->internalCheck(session.getGame()->mapConfiguration(), session.translator(), session.log());
        pl->setPlayability(game::map::Object::Playable);
    }

    void prepareTask(TaskEditor& ed)
    {
        String_t cmds[] = {
            // starts at 3 engines, 4 hulls
            "enqueueship 12, 4",
            // now, 1 engine, 3 hulls
            "enqueueship 12, 4",
            // now, 0 engines, 2 hulls
        };
        ed.replace(0, 0, cmds, TaskEditor::PlaceCursorAfter, TaskEditor::PlacePCBefore);
    }
}


/** Test operation on empty session: construction throws. */
void
TestGameProxyPredictedStarbaseAdaptor::testEmpty()
{
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    TS_ASSERT_THROWS(game::proxy::PredictedStarbaseAdaptor(session, 77, false), game::Exception);
}

/** Test operation on existing base, waitClear=true case. */
void
TestGameProxyPredictedStarbaseAdaptor::testNormalTrue()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    prepare(session);

    // Add auto task
    // Intentionally leave the TaskEditor alive so nobody tries to run (and fail) the task
    afl::base::Ptr<TaskEditor> ed = session.getAutoTaskEditor(PLANET_ID, Process::pkBaseTask, true);
    prepareTask(*ed);

    // Verify 'true' case: storage consumed, no build order
    game::proxy::PredictedStarbaseAdaptor testee(session, PLANET_ID, true);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, ENGINE_NR).orElse(-1), 0);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 2);
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrderHullIndex().orElse(-1), 0);
}

/** Test operation on existing base, waitClear=false case. */
void
TestGameProxyPredictedStarbaseAdaptor::testNormalFalse()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    prepare(session);

    // Add auto task
    // Intentionally leave the TaskEditor alive so nobody tries to run (and fail) the task
    afl::base::Ptr<TaskEditor> ed = session.getAutoTaskEditor(PLANET_ID, Process::pkBaseTask, true);
    prepareTask(*ed);

    // Verify 'false' case: storage for build order not yet consumed, build order present
    game::proxy::PredictedStarbaseAdaptor testee(session, PLANET_ID, false);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, ENGINE_NR).orElse(-1), 2);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 3);
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrderHullIndex().orElse(-1), HULL_SLOT);
}

/** Test operation on existing base, with no auto-task present. */
void
TestGameProxyPredictedStarbaseAdaptor::testNoTask()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    prepare(session);

    // Verify
    game::proxy::PredictedStarbaseAdaptor testee(session, PLANET_ID, true);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::EngineTech, ENGINE_NR).orElse(-1), 3);
    TS_ASSERT_EQUALS(testee.planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 4);
    TS_ASSERT_EQUALS(testee.planet().getBaseBuildOrderHullIndex().orElse(-1), 0);
}

/** Test extra methods, for coverage. */
void
TestGameProxyPredictedStarbaseAdaptor::testExtra()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    prepare(session);

    // Verify
    game::proxy::PredictedStarbaseAdaptor testee(session, PLANET_ID, true);
    TS_ASSERT_EQUALS(&testee.session(), &session);
    TS_ASSERT_THROWS_NOTHING(testee.cancelAllCloneOrders());
    TS_ASSERT_THROWS_NOTHING(testee.notifyListeners());

    String_t name;
    game::Id_t id;
    TS_ASSERT_EQUALS(testee.findShipCloningHere(id, name), false);
}

/** Test creation using factory method. */
void
TestGameProxyPredictedStarbaseAdaptor::testFactory()
{
    // Environment
    NullTranslator tx;
    NullFileSystem fs;
    Session session(tx, fs);
    prepare(session);

    // Add auto task
    // Intentionally leave the TaskEditor alive so nobody tries to run (and fail) the task
    afl::base::Ptr<TaskEditor> ed = session.getAutoTaskEditor(PLANET_ID, Process::pkBaseTask, true);
    prepareTask(*ed);

    // Create using factory: same as 'true' case
    game::proxy::PredictedStarbaseAdaptorFromSession factory(PLANET_ID, true);
    std::auto_ptr<game::proxy::StarbaseAdaptor> ad(factory.call(session));
    TS_ASSERT(ad.get() != 0);
    TS_ASSERT_EQUALS(ad->planet().getBaseStorage(game::EngineTech, ENGINE_NR).orElse(-1), 0);
    TS_ASSERT_EQUALS(ad->planet().getBaseStorage(game::HullTech, HULL_SLOT).orElse(-1), 2);
    TS_ASSERT_EQUALS(ad->planet().getBaseBuildOrderHullIndex().orElse(-1), 0);
}

