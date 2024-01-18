/**
  *  \file test/server/play/truehullpackertest.cpp
  *  \brief Test for server::play::TruehullPacker
  */

#include "server/play/truehullpacker.hpp"

#include "afl/data/access.hpp"
#include "afl/test/testrunner.hpp"
#include "game/test/root.hpp"
#include <memory>

/** Simple functionality test.
    A: create ship list; create TruehullPacker
    E: correct values for all properties */
AFL_TEST("server.play.TruehullPacker", a)
{
    // Input data
    afl::base::Ref<game::Root> r = game::test::makeRoot(game::HostVersion());
    afl::base::Ref<game::spec::ShipList> sl = *new game::spec::ShipList();
    sl->hullAssignments().add(/*player:*/ 2, /*slot:*/ 5, /*hull:*/ 7);

    // Testee with offset 0
    {
        server::play::TruehullPacker testee(*sl, *r, 0);
        a.checkEqual("01. getName", testee.getName(), "truehull");

        std::auto_ptr<server::Value_t> value(testee.buildValue());
        afl::data::Access ap(value.get());
        a.checkEqual("11. offset0", ap[2][4].toInteger(), 7);
    }

    // Same thing with offset 1
    {
        server::play::TruehullPacker testee(*sl, *r, 1);
        std::auto_ptr<server::Value_t> value(testee.buildValue());
        afl::data::Access ap(value.get());
        a.checkEqual("21. offset1", ap[1][4].toInteger(), 7);
    }
}
