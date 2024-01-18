/**
  *  \file test/game/proxy/friendlycodeproxytest.cpp
  *  \brief Test for game::proxy::FriendlyCodeProxy
  */

#include "game/proxy/friendlycodeproxy.hpp"

#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

/** Simple test.
    A: prepare empty universe with Root (for host version) and ShipList (for friendly-code list)
    E: verify that we can properly generate random friendly codes. */
AFL_TEST("game.proxy.FriendlyCodeProxy:generateRandomCode", a)
{
    // Session
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0))).asPtr());
    h.session().setShipList(new game::spec::ShipList());

    // Test
    game::proxy::FriendlyCodeProxy testee(h.gameSender());
    game::test::WaitIndicator ind;
    String_t sa = testee.generateRandomCode(ind);
    String_t sb = testee.generateRandomCode(ind);

    // Friendly codes should be different (=random) and not empty
    a.checkDifferent("01", sa, "");
    a.checkDifferent("02", sb, "");
    a.checkDifferent("03", sa, sb);
}
