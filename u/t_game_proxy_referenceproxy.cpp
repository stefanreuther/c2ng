/**
  *  \file u/t_game_proxy_referenceproxy.cpp
  *  \brief Test for game::proxy::ReferenceProxy
  */

#include "game/proxy/referenceproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/game.hpp"
#include "game/turn.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"

/** Test behaviour on empty session. */
void
TestGameProxyReferenceProxy::testEmpty()
{
    // Make empty session
    game::test::SessionThread t;
    game::proxy::ReferenceProxy testee(t.gameSender());

    // Cannot retrieve any name
    game::test::WaitIndicator ind;
    String_t out;
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName, out), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName, out), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(), game::LongName, out), false);
}

/** Test behaviour with existing units. */
void
TestGameProxyReferenceProxy::testNormal()
{
    // Make game with a planet in it
    afl::base::Ptr<game::Game> g = new game::Game();
    game::map::Planet* p = g->currentTurn().universe().planets().create(10);
    p->setName("Melmac");

    // Make session
    game::test::SessionThread t;
    t.session().setGame(g);
    game::proxy::ReferenceProxy testee(t.gameSender());

    // Retrieve different names
    game::test::WaitIndicator ind;
    String_t out;
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName, out), true);
    TS_ASSERT_EQUALS(out, "Planet #10: Melmac");

    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName, out), true);
    TS_ASSERT_EQUALS(out, "Melmac");

    // Cannot retrieve name of null reference in any case
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(), game::LongName, out), false);
}

