/**
  *  \file test/game/proxy/teamproxytest.cpp
  *  \brief Test for game::proxy::TeamProxy
  */

#include "game/proxy/teamproxy.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/teamsettings.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

/** Test behaviour on empty session.
    A: create empty session. Make a TeamProxy, call init, call commit.
    E: team object left at default; no exception thrown */
AFL_TEST("game.proxy.TeamProxy:empty", a)
{
    // Session
    game::test::SessionThread h;
    game::proxy::TeamProxy testee(h.gameSender());

    // Read teams through proxy
    game::TeamSettings set;
    game::test::WaitIndicator ind;
    AFL_CHECK_SUCCEEDS(a("01. init"), testee.init(ind, set));
    a.check("02. hasAnyTeams", !set.hasAnyTeams());

    // Write teams through proxy
    AFL_CHECK_SUCCEEDS(a("11. commit"), testee.commit(set));
    AFL_CHECK_SUCCEEDS(a("12. sync"), h.sync());
}

/** Test behaviour on full session.
    A: create session with configured teams. Make a TeamProxy, call init, call commit.
    E: teams correctly transferred out; changes correctly transferred back in */
AFL_TEST("game.proxy.TeamProxy:normal", a)
{
    afl::string::NullTranslator tx;

    // Session
    game::test::SessionThread h;
    game::proxy::TeamProxy testee(h.gameSender());

    // Configure teams
    afl::base::Ref<game::Game> g(*new game::Game());
    g->teamSettings().setTeamName(2, "two");
    g->teamSettings().setPlayerTeam(3, 5);
    h.session().setGame(g.asPtr());

    // Read teams through proxy
    game::TeamSettings set;
    game::test::WaitIndicator ind;
    AFL_CHECK_SUCCEEDS(a("01. init"), testee.init(ind, set));
    a.checkEqual("02. getTeamName", set.getTeamName(2, tx), "two");
    a.checkEqual("03. getPlayerTeam", set.getPlayerTeam(3), 5);

    // Modify and write back
    set.setTeamName(2, "double");
    set.setPlayerTeam(3, 7);
    AFL_CHECK_SUCCEEDS(a("11. commit"), testee.commit(set));
    AFL_CHECK_SUCCEEDS(a("12. sync"), h.sync());

    a.checkEqual("21. getTeamName", g->teamSettings().getTeamName(2, tx), "double");
    a.checkEqual("22. getPlayerTeam", g->teamSettings().getPlayerTeam(3), 7);
}
