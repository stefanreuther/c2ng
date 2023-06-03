/**
  *  \file u/t_game_proxy_allianceproxy.cpp
  *  \brief Test for game::proxy::AllianceProxy
  */

#include "game/proxy/allianceproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/alliance/hosthandler.hpp"
#include "game/game.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"
#include "game/v3/command.hpp"
#include "game/v3/commandcontainer.hpp"
#include "game/v3/commandextra.hpp"

/** Test behaviour on empty session. Must not crash. */
void
TestGameProxyAllianceProxy::testEmpty()
{
    // Empty Session
    game::test::SessionThread h;
    game::proxy::AllianceProxy testee(h.gameSender());
    game::test::WaitIndicator ind;
    game::proxy::AllianceProxy::Status st = testee.getStatus(ind);

    // Verify
    TS_ASSERT_EQUALS(st.alliances.getLevels().size(), 0U);
    TS_ASSERT_EQUALS(st.playerNames.get(1), "");
    TS_ASSERT_EQUALS(st.playerNames.get(2), "");
    TS_ASSERT_EQUALS(st.playerNames.get(3), "");
    TS_ASSERT_EQUALS(st.players, game::PlayerSet_t());
    TS_ASSERT_EQUALS(st.viewpointPlayer, 0);
}

/** Test normal behaviour. Use a HostHandler to verify alliance transfer. */
void
TestGameProxyAllianceProxy::testIt()
{
    // Session
    game::test::SessionThread h;
    afl::base::Ref<game::Root> root(game::test::makeRoot(game::HostVersion(game::HostVersion::Host, MKVERSION(3,22,20))));
    h.session().setRoot(root.asPtr());
    for (int i = 1; i <= 5; ++i) {
        root->playerList().create(i);
    }

    afl::base::Ptr<game::Game> g = new game::Game();
    h.session().setGame(g);
    g->setViewpointPlayer(2);
    g->currentTurn().alliances().addNewHandler(new game::alliance::HostHandler(root->hostVersion().getVersion(), g->currentTurn(), g->getViewpointPlayer()),
                                               h.session().translator());

    // Test object
    game::proxy::AllianceProxy testee(h.gameSender());

    // Initialize
    game::test::WaitIndicator ind;
    game::proxy::AllianceProxy::Status st = testee.getStatus(ind);

    // Verify
    TS_ASSERT_EQUALS(st.alliances.getLevels().size(), 1U);
    TS_ASSERT_DIFFERS(st.alliances.getLevels()[0].getName(), "");
    TS_ASSERT_DIFFERS(st.alliances.getLevels()[0].getId(), "");
    TS_ASSERT_DIFFERS(st.playerNames.get(1), "");
    TS_ASSERT_DIFFERS(st.playerNames.get(2), "");
    TS_ASSERT_DIFFERS(st.playerNames.get(3), "");
    TS_ASSERT_EQUALS(st.players, game::PlayerSet_t() + 1 + 2 + 3 + 4 + 5);
    TS_ASSERT_EQUALS(st.viewpointPlayer, 2);

    // Offer alliance to 4, verify
    st.alliances.set(0, 4, game::alliance::Offer::Yes);
    testee.setAlliances(st.alliances);
    h.sync();

    // - command must have been created
    const game::v3::Command* cmd = game::v3::CommandExtra::create(g->currentTurn()).create(2).getCommand(game::v3::Command::TAlliance, 0);
    TS_ASSERT(cmd != 0);
    TS_ASSERT_EQUALS(cmd->getArg(), "ff4");

    // - team settings must have been updated
    TS_ASSERT_EQUALS(g->teamSettings().getPlayerTeam(2), g->teamSettings().getPlayerTeam(4));
}

