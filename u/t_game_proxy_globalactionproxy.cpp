/**
  *  \file u/t_game_proxy_globalactionproxy.cpp
  *  \brief Test for game::proxy::GlobalActionProxy
  */

#include "game/proxy/globalactionproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/interface/globalactionextra.hpp"

/** Test behaviour on empty session. */
void
TestGameProxyGlobalActionProxy::testEmpty()
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::GlobalActionProxy testee(t.gameSender());
    util::TreeList result;
    testee.getActions(ind, result);

    TS_ASSERT_EQUALS(result.getFirstChild(util::TreeList::root), util::TreeList::nil);
}

/** Test behaviour on completed session. */
void
TestGameProxyGlobalActionProxy::testNormal()
{
    game::test::SessionThread t;

    // Add an item. This is a legitimate (but not public) way to add a separator/inner node.
    // (Normally, such nodes are only created on the way when a real node with underlying action is added.)
    game::interface::GlobalActionExtra::create(t.session())
        .actionNames().add(0, "test", util::TreeList::root);

    // Call
    game::test::WaitIndicator ind;
    game::proxy::GlobalActionProxy testee(t.gameSender());
    util::TreeList result;
    testee.getActions(ind, result);

    // Verify result
    size_t a = result.getFirstChild(util::TreeList::root);
    TS_ASSERT_DIFFERS(a, util::TreeList::nil);

    int32_t key;
    String_t name;
    TS_ASSERT_EQUALS(result.get(a, key, name), true);
    TS_ASSERT_EQUALS(name, "test");
}

