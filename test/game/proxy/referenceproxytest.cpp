/**
  *  \file test/game/proxy/referenceproxytest.cpp
  *  \brief Test for game::proxy::ReferenceProxy
  */

#include "game/proxy/referenceproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/game.hpp"
#include "game/map/planet.hpp"
#include "game/map/universe.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"
#include "game/turn.hpp"

/** Test behaviour on empty session. */
AFL_TEST("game.proxy.ReferenceProxy:empty", a)
{
    // Make empty session
    game::test::SessionThread t;
    game::proxy::ReferenceProxy testee(t.gameSender());

    // Cannot retrieve any name
    game::test::WaitIndicator ind;
    a.checkEqual("01. getReferenceName", testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName).isValid(), false);
    a.checkEqual("02. getReferenceName", testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName).isValid(), false);
    a.checkEqual("03. getReferenceName", testee.getReferenceName(ind, game::Reference(), game::LongName).isValid(), false);

    // Cannot retrieve any position
    a.checkEqual("11. getReferencePosition", testee.getReferencePosition(ind, game::Reference(game::Reference::Planet, 10)).isValid(), false);
    a.checkEqual("12. getReferencePosition", testee.getReferencePosition(ind, game::Reference()).isValid(), false);
}

/** Test behaviour with existing units. */
AFL_TEST("game.proxy.ReferenceProxy:normal", a)
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
    a.checkEqual("01. getReferenceName", testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::LongName).orElse(""), "Planet #10: Melmac");
    a.checkEqual("02. getReferenceName", testee.getReferenceName(ind, game::Reference(game::Reference::Planet, 10), game::PlainName).orElse(""), "Melmac");

    a.checkEqual("11. getReferencePosition", testee.getReferencePosition(ind, game::Reference(game::Reference::Planet, 10)).orElse(game::map::Point()), game::map::Point(1234, 2345));

    // Cannot retrieve name/position of null reference in any case
    a.checkEqual("21. getReferenceName", testee.getReferenceName(ind, game::Reference(), game::LongName).isValid(), false);
    a.checkEqual("22. getReferencePosition", testee.getReferencePosition(ind, game::Reference()).isValid(), false);
}
