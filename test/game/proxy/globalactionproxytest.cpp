/**
  *  \file test/game/proxy/globalactionproxytest.cpp
  *  \brief Test for game::proxy::GlobalActionProxy
  */

#include "game/proxy/globalactionproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/interface/globalactioncontext.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "interpreter/variablereference.hpp"

/** Test behaviour on empty session/null reference. */
AFL_TEST("game.proxy.GlobalActionProxy:empty", a)
{
    game::test::SessionThread t;
    game::test::WaitIndicator ind;
    game::proxy::GlobalActionProxy testee(t.gameSender());
    util::TreeList result;
    testee.getActions(ind, result, interpreter::VariableReference());

    a.checkEqual("01. getFirstChild", result.getFirstChild(util::TreeList::root), util::TreeList::nil);
}

/** Test behaviour on completed session. */
AFL_TEST("game.proxy.GlobalActionProxy:normal", a)
{
    game::test::SessionThread t;

    // Add an item. This is a legitimate (but not public) way to add a separator/inner node.
    // (Normally, such nodes are only created on the way when a real node with underlying action is added.)
    game::interface::GlobalActionContext ctx;
    ctx.data()->actionNames.add(0, "test", util::TreeList::root);

    // Store this in a process
    interpreter::Process& p = t.session().processList().create(t.session().world(), "testNormal");
    interpreter::VariableReference ref = interpreter::VariableReference::Maker(p).make("WHATEVER", &ctx);

    // Call
    game::test::WaitIndicator ind;
    game::proxy::GlobalActionProxy testee(t.gameSender());
    util::TreeList result;
    testee.getActions(ind, result, ref);

    // Verify result
    size_t ax = result.getFirstChild(util::TreeList::root);
    a.checkDifferent("01. getFirstChild", ax, util::TreeList::nil);

    int32_t key;
    String_t name;
    a.checkEqual("11. result", result.get(ax, key, name), true);
    a.checkEqual("12. name", name, "test");
}
