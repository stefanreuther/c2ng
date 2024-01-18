/**
  *  \file test/game/proxy/expressionlistproxytest.cpp
  *  \brief Test for game::proxy::ExpressionListProxy
  */

#include "game/proxy/expressionlistproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

/** Test behaviour on empty session.
    A: create empty session. Create ExpressionListProxy.
    E: calls are ignored, empty result returned. */
AFL_TEST("game.proxy.ExpressionListProxy:empty", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    game::proxy::ExpressionListProxy testee(h.gameSender(), game::config::ExpressionLists::PlanetLabels);

    testee.pushRecent("[x]", "p");
    testee.pushRecent("[y]", "q");

    game::config::ExpressionLists::Items_t result;
    testee.getList(ind, result);
    a.checkEqual("01. size", result.size(), 0U);
}

/** Test behaviour on non-empty session.
    A: create session with Game. Create ExpressionListProxy.
    E: the result of pushRecent() can be read back with getList(). */
AFL_TEST("game.proxy.ExpressionListProxy:normal", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;
    h.session().setGame(new game::Game());
    game::proxy::ExpressionListProxy testee(h.gameSender(), game::config::ExpressionLists::PlanetLabels);

    testee.pushRecent("[x]", "p");
    testee.pushRecent("[y]", "q");

    game::config::ExpressionLists::Items_t result;
    testee.getList(ind, result);
    a.checkEqual("01. size", result.size(), 2U);

    a.checkEqual("11. name",      result[0].name, "q");
    a.checkEqual("12. flags",     result[0].flags, "[y]");
    a.checkEqual("13. value",     result[0].value, "q");
    a.checkEqual("14. isHeading", result[0].isHeading, false);

    a.checkEqual("21. name",      result[1].name, "p");
    a.checkEqual("22. flags",     result[1].flags, "[x]");
    a.checkEqual("23. value",     result[1].value, "p");
    a.checkEqual("24. isHeading", result[1].isHeading, false);
}
