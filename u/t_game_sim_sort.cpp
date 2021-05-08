/**
  *  \file u/t_game_sim_sort.cpp
  *  \brief Test for game::sim::Sort
  */

#include "game/sim/sort.hpp"

#include "t_game_sim.hpp"
#include "game/sim/ship.hpp"

void
TestGameSimSort::testIt()
{
    game::sim::Ship a, b;
    a.setId(100);             b.setId(200);
    a.setOwner(3);            b.setOwner(2);
    a.setHullTypeOnly(88);    b.setHullTypeOnly(66);
    a.setFriendlyCode("123"); b.setFriendlyCode("-20");
    a.setName("ho");          b.setName("hi");

    TS_ASSERT(game::sim::compareId(a, a) == 0);
    TS_ASSERT(game::sim::compareId(a, b) < 0);
    TS_ASSERT(game::sim::compareId(b, a) > 0);

    TS_ASSERT(game::sim::compareOwner(a, a) == 0);
    TS_ASSERT(game::sim::compareOwner(a, b) > 0);
    TS_ASSERT(game::sim::compareOwner(b, a) < 0);

    TS_ASSERT(game::sim::compareHull(a, a) == 0);
    TS_ASSERT(game::sim::compareHull(a, b) > 0);
    TS_ASSERT(game::sim::compareHull(b, a) < 0);

    TS_ASSERT(game::sim::compareBattleOrderHost(a, a) == 0);
    TS_ASSERT(game::sim::compareBattleOrderHost(a, b) < 0);
    TS_ASSERT(game::sim::compareBattleOrderHost(b, a) > 0);

    TS_ASSERT(game::sim::compareBattleOrderPHost(a, a) == 0);
    TS_ASSERT(game::sim::compareBattleOrderPHost(a, b) > 0);
    TS_ASSERT(game::sim::compareBattleOrderPHost(b, a) < 0);

    TS_ASSERT(game::sim::compareName(a, a) == 0);
    TS_ASSERT(game::sim::compareName(a, b) > 0);
    TS_ASSERT(game::sim::compareName(b, a) < 0);
}

