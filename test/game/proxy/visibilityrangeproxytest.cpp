/**
  *  \file test/game/proxy/visibilityrangeproxytest.cpp
  *  \brief Test for game::proxy::VisibilityRangeProxy
  */

#include "game/proxy/visibilityrangeproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/config/userconfiguration.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

using game::test::SessionThread;
using game::test::WaitIndicator;

/** Test a simple sequence. */
AFL_TEST("game.proxy.VisibilityRangeProxy:sequence", a)
{
    const int PLAYER = 5;

    // Set up environment
    SessionThread t;

    // Add root
    afl::base::Ptr<game::Root> r = game::test::makeRoot(game::HostVersion()).asPtr();
    t.session().setRoot(r);
    r->userConfiguration().setOption("chart.range.distance", "87", game::config::ConfigurationOption::Game);

    // Add game with one object
    afl::base::Ptr<game::Game> g = new game::Game();
    t.session().setGame(g);
    g->teamSettings().setViewpointPlayer(PLAYER);
    game::map::Planet* p = g->currentTurn().universe().planets().create(33);
    p->setOwner(PLAYER);
    p->setPosition(game::map::Point(1000, 1000));
    p->internalCheck(game::map::Configuration(), game::PlayerSet_t(), 15, t.session().translator(), t.session().log());
    p->setPlayability(game::map::Object::ReadOnly);

    // Operate
    WaitIndicator ind;
    game::proxy::VisibilityRangeProxy proxy(t.gameSender());

    // getVisibilityRangeSettings - must not be empty
    game::map::VisSettings_t set = proxy.getVisibilityRangeSettings(ind);
    a.checkDifferent("01. size", set.size(), 0U);

    // loadVisibilityConfiguration - must produce correct value
    game::map::VisConfig cfg = proxy.loadVisibilityConfiguration(ind);
    a.checkEqual("11. range", cfg.range, 87);

    // buildVisibilityRange - must produce non-null, non-empty value
    std::auto_ptr<game::map::RangeSet> rs = proxy.buildVisibilityRange(ind, game::map::VisConfig(game::map::VisModeOwn, 100, false));
    a.checkNonNull("21. buildVisibilityRange", rs.get());
    a.check("22. isEmpty", !rs->isEmpty());
}

/** Test operation on empty session. */
AFL_TEST("game.proxy.VisibilityRangeProxy:empty", a)
{
    // Set up environment
    SessionThread t;
    WaitIndicator ind;
    game::proxy::VisibilityRangeProxy proxy(t.gameSender());

    // getVisibilityRangeSettings - cannot build on empty session
    game::map::VisSettings_t set = proxy.getVisibilityRangeSettings(ind);
    a.checkEqual("01. size", set.size(), 0U);

    // buildVisibilityRange - must produce non-null, empty value
    std::auto_ptr<game::map::RangeSet> rs = proxy.buildVisibilityRange(ind, game::map::VisConfig(game::map::VisModeOwn, 100, false));
    a.checkNonNull("11. buildVisibilityRange", rs.get());
    a.check("12. isEmpty", rs->isEmpty());
}
