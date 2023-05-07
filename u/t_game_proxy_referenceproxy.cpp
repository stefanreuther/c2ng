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
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(), game::LongName).isValid(), false);

    // Cannot retrieve any position
    TS_ASSERT_EQUALS(testee.getReferencePosition(ind, game::Reference(game::Reference::Planet, 10)).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferencePosition(ind, game::Reference()).isValid(), false);
}

/** Test behaviour with existing units. */
void
TestGameProxyReferenceProxy::testNormal()
{
    // Make game with a planet in it
    afl::base::Ptr<game::Game> g = new game::Game();
    game::map::Planet* p = g->currentTurn().universe().planets().create(10);
    p->setName("Melmac");
    p->setPosition(game::map::Point(1234, 2345));

    // Make session
    game::test::SessionThread t;
    t.session().setGame(g);
    game::proxy::ReferenceProxy testee(t.gameSender());

    // Retrieve different names
    game::test::WaitIndicator ind;
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName).orElse(""), "Planet #10: Melmac");
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName).orElse(""), "Melmac");

    TS_ASSERT_EQUALS(testee.getReferencePosition(ind, game::Reference(game::Reference::Planet, 10)).orElse(game::map::Point()), game::map::Point(1234, 2345));

    // Cannot retrieve name/position of null reference in any case
    TS_ASSERT_EQUALS(testee.getReferenceName(ind, game::Reference(), game::LongName).isValid(), false);
    TS_ASSERT_EQUALS(testee.getReferencePosition(ind, game::Reference()).isValid(), false);
}

