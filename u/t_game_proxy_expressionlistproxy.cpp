/**
  *  \file u/t_game_proxy_expressionlistproxy.cpp
  *  \brief Test for game::proxy::ExpressionListProxy
  */

#include "game/proxy/expressionlistproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"

/** Test behaviour on empty session.
    A: create empty session. Create ExpressionListProxy.
    E: calls are ignored, empty result returned. */
void
TestGameProxyExpressionListProxy::testEmpty()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    game::proxy::ExpressionListProxy testee(h.gameSender(), game::config::ExpressionLists::PlanetLabels);

    testee.pushRecent("[x]", "p");
    testee.pushRecent("[y]", "q");

    game::config::ExpressionLists::Items_t result;
    testee.getList(ind, result);
    TS_ASSERT_EQUALS(result.size(), 0U);
}

/** Test behaviour on non-empty session.
    A: create session with Game. Create ExpressionListProxy.
    E: the result of pushRecent() can be read back with getList(). */
void
TestGameProxyExpressionListProxy::testNormal()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    h.session().setGame(new game::Game());
    game::proxy::ExpressionListProxy testee(h.gameSender(), game::config::ExpressionLists::PlanetLabels);

    testee.pushRecent("[x]", "p");
    testee.pushRecent("[y]", "q");

    game::config::ExpressionLists::Items_t result;
    testee.getList(ind, result);
    TS_ASSERT_EQUALS(result.size(), 2U);

    TS_ASSERT_EQUALS(result[0].name, "q");
    TS_ASSERT_EQUALS(result[0].flags, "[y]");
    TS_ASSERT_EQUALS(result[0].value, "q");
    TS_ASSERT_EQUALS(result[0].isHeading, false);

    TS_ASSERT_EQUALS(result[1].name, "p");
    TS_ASSERT_EQUALS(result[1].flags, "[x]");
    TS_ASSERT_EQUALS(result[1].value, "p");
    TS_ASSERT_EQUALS(result[1].isHeading, false);
}

