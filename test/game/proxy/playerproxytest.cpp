/**
  *  \file test/game/proxy/playerproxytest.cpp
  *  \brief Test for game::proxy::PlayerProxy
  */

#include "game/proxy/playerproxy.hpp"

#include "afl/test/testrunner.hpp"
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
AFL_TEST("game.proxy.PlayerProxy:empty", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    game::proxy::PlayerProxy testee(h.gameSender());
    a.check("01. getAllPlayers", testee.getAllPlayers(ind).empty());

    a.checkEqual("11. getPlayerName", testee.getPlayerName(ind, 1, game::Player::LongName), "");

    game::PlayerArray<String_t> allNames = testee.getPlayerNames(ind, game::Player::LongName);
    a.checkEqual("21. getPlayerNames", allNames.get(1), "");
    a.checkEqual("22. getPlayerNames", allNames.get(10), "");
}

/** Test behaviour with nonempty lists.
    A: create empty session; add a root and populate player list.
    E: all functions must return expected empty values. */
AFL_TEST("game.proxy.PlayerProxy:normal", a)
{
    game::test::SessionThread h;
    game::test::WaitIndicator ind;

    // Populate root
    Ptr<Root> root = game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(4,0,0))).asPtr();

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
    a.checkEqual("01. getAllPlayers", testee.getAllPlayers(ind).toInteger(), 0x06U);     // 1<<1 + 1<<2

    a.checkEqual("11. getPlayerName", testee.getPlayerName(ind, 1, game::Player::LongName), "The Ones");
    a.checkEqual("12. getPlayerName", testee.getPlayerName(ind, 2, game::Player::LongName), "The Twos");
    a.checkEqual("13. getPlayerName", testee.getPlayerName(ind, 3, game::Player::LongName), "Player 3");

    game::PlayerArray<String_t> allNames = testee.getPlayerNames(ind, game::Player::AdjectiveName);
    a.checkEqual("21. getPlayerNames", allNames.get(1), "single");
    a.checkEqual("22. getPlayerNames", allNames.get(2), "double");
    a.checkEqual("23. getPlayerNames", allNames.get(10), "");
}
