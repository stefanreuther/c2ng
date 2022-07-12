/**
  *  \file u/t_game_proxy_playerproxy.cpp
  *  \brief Test for game::proxy::PlayerProxy
  */

#include "game/proxy/playerproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

using afl::base::Ptr;
using game::Root;
using game::Player;
using game::PlayerList;

/** Test behaviour with empty list.
    A: create empty session.
    E: all functions must return expected empty values. */
void
TestGameProxyPlayerProxy::testEmpty()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    game::proxy::PlayerProxy testee(h.gameSender());
    TS_ASSERT(testee.getAllPlayers(ind).empty());

    TS_ASSERT_EQUALS(testee.getPlayerName(ind, 1, game::Player::LongName), "");

    game::PlayerArray<String_t> allNames = testee.getPlayerNames(ind, game::Player::LongName);
    TS_ASSERT_EQUALS(allNames.get(1), "");
    TS_ASSERT_EQUALS(allNames.get(10), "");
}

/** Test behaviour with nonempty lists.
    A: create empty session; add a root and populate player list.
    E: all functions must return expected empty values. */
void
TestGameProxyPlayerProxy::testNormal()
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Populate root
    Ptr<Root> root = new game::test::Root(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0)));

    PlayerList& playerList = root->playerList();
    Player& p1 = *playerList.create(1);
    p1.setName(Player::LongName, "The Ones");
    p1.setName(Player::AdjectiveName, "single");
    Player& p2 = *playerList.create(2);
    p2.setName(Player::LongName, "The Twos");
    p2.setName(Player::AdjectiveName, "double");

    h.session().setRoot(root);

    // Check
    game::proxy::PlayerProxy testee(h.gameSender());
    TS_ASSERT_EQUALS(testee.getAllPlayers(ind).toInteger(), 0x06U);     // 1<<1 + 1<<2

    TS_ASSERT_EQUALS(testee.getPlayerName(ind, 1, game::Player::LongName), "The Ones");
    TS_ASSERT_EQUALS(testee.getPlayerName(ind, 2, game::Player::LongName), "The Twos");
    TS_ASSERT_EQUALS(testee.getPlayerName(ind, 3, game::Player::LongName), "Player 3");

    game::PlayerArray<String_t> allNames = testee.getPlayerNames(ind, game::Player::AdjectiveName);
    TS_ASSERT_EQUALS(allNames.get(1), "single");
    TS_ASSERT_EQUALS(allNames.get(2), "double");
    TS_ASSERT_EQUALS(allNames.get(10), "");
}

