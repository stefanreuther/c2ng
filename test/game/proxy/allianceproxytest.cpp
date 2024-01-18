/**
  *  \file test/game/proxy/allianceproxytest.cpp
  *  \brief Test for game::proxy::AllianceProxy
  */

#include "game/proxy/allianceproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.AllianceProxy:empty", a)
{
    // Empty Session
    game::test::SessionThread h;
    game::proxy::AllianceProxy testee(h.gameSender());
    game::test::WaitIndicator ind;
    game::proxy::AllianceProxy::Status st = testee.getStatus(ind);

    // Verify
    a.checkEqual("01. getLevels", st.alliances.getLevels().size(), 0U);
    a.checkEqual("02. playerNames", st.playerNames.get(1), "");
    a.checkEqual("03. playerNames", st.playerNames.get(2), "");
    a.checkEqual("04. playerNames", st.playerNames.get(3), "");
    a.checkEqual("05. players", st.players, game::PlayerSet_t());
    a.checkEqual("06. viewpointPlayer", st.viewpointPlayer, 0);
}

/** Test normal behaviour. Use a HostHandler to verify alliance transfer. */
AFL_TEST("game.proxy.AllianceProxy:normal", a)
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
    a.checkEqual("01. getLevels", st.alliances.getLevels().size(), 1U);
    a.checkDifferent("02. getLevels", st.alliances.getLevels()[0].getName(), "");
    a.checkDifferent("03. getLevels", st.alliances.getLevels()[0].getId(), "");
    a.checkDifferent("04. playerNames", st.playerNames.get(1), "");
    a.checkDifferent("05. playerNames", st.playerNames.get(2), "");
    a.checkDifferent("06. playerNames", st.playerNames.get(3), "");
    a.checkEqual("07. players", st.players, game::PlayerSet_t() + 1 + 2 + 3 + 4 + 5);
    a.checkEqual("08. viewpointPlayer", st.viewpointPlayer, 2);

    // Offer alliance to 4, verify
    st.alliances.set(0, 4, game::alliance::Offer::Yes);
    testee.setAlliances(st.alliances);
    h.sync();

    // - command must have been created
    const game::v3::Command* cmd = game::v3::CommandExtra::create(g->currentTurn()).create(2).getCommand(game::v3::Command::TAlliance, 0);
    a.checkNonNull("11. cmd", cmd);
    a.checkEqual("12. getArg", cmd->getArg(), "ff4");

    // - team settings must have been updated
    a.checkEqual("21. teamSettings", g->teamSettings().getPlayerTeam(2), g->teamSettings().getPlayerTeam(4));
}
