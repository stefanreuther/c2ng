/**
  *  \file u/t_server_play_truehullpacker.cpp
  *  \brief Test for server::play::TruehullPacker
  */

#include <memory>
#include "server/play/truehullpacker.hpp"

#include "t_server_play.hpp"
#include "afl/data/access.hpp"
#include "game/test/root.hpp"

/** Simple functionality test.
    A: create ship list; create TruehullPacker
    E: correct values for all properties */
void
TestServerPlayTruehullPacker::testIt()
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    sl->hullAssignments().add(/*player:*/ 2, /*slot:*/ 5, /*hull:*/ 7);

    // Testee with offset 0
    {
        server::play::TruehullPacker testee(*sl, *r, 0);
        TS_ASSERT_EQUALS(testee.getName(), "truehull");

        std::auto_ptr<server::Value_t> value(testee.buildValue());
        afl::data::Access a(value.get());
        TS_ASSERT_EQUALS(a[2][4].toInteger(), 7);
    }

    // Same thing with offset 1
    {
        server::play::TruehullPacker testee(*sl, *r, 1);
        std::auto_ptr<server::Value_t> value(testee.buildValue());
        afl::data::Access a(value.get());
        TS_ASSERT_EQUALS(a[1][4].toInteger(), 7);
    }
}
