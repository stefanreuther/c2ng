/**
  *  \file u/t_game_proxy_teamproxy.cpp
  *  \brief Test for game::proxy::TeamProxy
  */

#include "game/proxy/teamproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/teamsettings.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "afl/string/nulltranslator.hpp"
#include "game/game.hpp"

/** Test behaviour on empty session.
    A: create empty session. Make a TeamProxy, call init, call commit.
    E: team object left at default; no exception thrown */
void
TestGameProxyTeamProxy::testEmpty()
{
    // Session
    game::test::SessionThread h;
    game::proxy::TeamProxy testee(h.gameSender());

    // Read teams through proxy
    game::TeamSettings set;
    game::test::WaitIndicator ind;
    TS_ASSERT_THROWS_NOTHING(testee.init(ind, set));
    TS_ASSERT(!set.hasAnyTeams());

    // Write teams through proxy
    TS_ASSERT_THROWS_NOTHING(testee.commit(set));
    TS_ASSERT_THROWS_NOTHING(h.sync());
}

/** Test behaviour on full session.
    A: create session with configured teams. Make a TeamProxy, call init, call commit.
    E: teams correctly transferred out; changes correctly transferred back in */
void
TestGameProxyTeamProxy::testNormal()
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
    TS_ASSERT_THROWS_NOTHING(testee.init(ind, set));
    TS_ASSERT_EQUALS(set.getTeamName(2, tx), "two");
    TS_ASSERT_EQUALS(set.getPlayerTeam(3), 5);

    // Modify and write back
    set.setTeamName(2, "double");
    set.setPlayerTeam(3, 7);
    TS_ASSERT_THROWS_NOTHING(testee.commit(set));
    TS_ASSERT_THROWS_NOTHING(h.sync());

    TS_ASSERT_EQUALS(g->teamSettings().getTeamName(2, tx), "double");
    TS_ASSERT_EQUALS(g->teamSettings().getPlayerTeam(3), 7);
}

