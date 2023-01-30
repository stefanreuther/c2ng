/**
  *  \file u/t_game_proxy_friendlycodeproxy.cpp
  *  \brief Test for game::proxy::FriendlyCodeProxy
  */

#include "game/proxy/friendlycodeproxy.hpp"

#include "t_game_proxy.hpp"
#include "game/test/root.hpp"
#include "game/test/sessionthread.hpp"
#include "game/test/waitindicator.hpp"

/** Simple test.
    A: prepare empty universe with Root (for host version) and ShipList (for friendly-code list)
    E: verify that we can properly generate random friendly codes. */
void
TestGameProxyFriendlyCodeProxy::testIt()
{
    // Session
    game::test::SessionThread h;
    h.session().setRoot(game::test::makeRoot(game::HostVersion(game::HostVersion::PHost, MKVERSION(3,0,0))).asPtr());
    h.session().setShipList(new game::spec::ShipList());

    // Test
    game::proxy::FriendlyCodeProxy testee(h.gameSender());
    game::test::WaitIndicator ind;
    String_t a = testee.generateRandomCode(ind);
    String_t b = testee.generateRandomCode(ind);

    // Friendly codes should be different (=random) and not empty
    TS_ASSERT_DIFFERS(a, "");
    TS_ASSERT_DIFFERS(b, "");
    TS_ASSERT_DIFFERS(a, b);
}

