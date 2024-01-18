/**
  *  \file test/game/sim/sorttest.cpp
  *  \brief Test for game::sim::Sort
  */

#include "game/sim/sort.hpp"

#include "afl/test/testrunner.hpp"
#include "game/sim/ship.hpp"

AFL_TEST("game.sim.Sort", a)
{
    game::sim::Ship sa, sb;
    sa.setId(100);             sb.setId(200);
    sa.setOwner(3);            sb.setOwner(2);
    sa.setHullTypeOnly(88);    sb.setHullTypeOnly(66);
    sa.setFriendlyCode("123"); sb.setFriendlyCode("-20");
    sa.setName("ho");          sb.setName("hi");

    a.check("01. compareId", game::sim::compareId(sa, sa) == 0);
    a.check("02. compareId", game::sim::compareId(sa, sb) < 0);
    a.check("03. compareId", game::sim::compareId(sb, sa) > 0);

    a.check("11. compareOwner", game::sim::compareOwner(sa, sa) == 0);
    a.check("12. compareOwner", game::sim::compareOwner(sa, sb) > 0);
    a.check("13. compareOwner", game::sim::compareOwner(sb, sa) < 0);

    a.check("21. compareHull", game::sim::compareHull(sa, sa) == 0);
    a.check("22. compareHull", game::sim::compareHull(sa, sb) > 0);
    a.check("23. compareHull", game::sim::compareHull(sb, sa) < 0);

    a.check("31. compareBattleOrderHost", game::sim::compareBattleOrderHost(sa, sa) == 0);
    a.check("32. compareBattleOrderHost", game::sim::compareBattleOrderHost(sa, sb) < 0);
    a.check("33. compareBattleOrderHost", game::sim::compareBattleOrderHost(sb, sa) > 0);

    a.check("41. compareBattleOrderPHost", game::sim::compareBattleOrderPHost(sa, sa) == 0);
    a.check("42. compareBattleOrderPHost", game::sim::compareBattleOrderPHost(sa, sb) > 0);
    a.check("43. compareBattleOrderPHost", game::sim::compareBattleOrderPHost(sb, sa) < 0);

    a.check("51. compareName", game::sim::compareName(sa, sa) == 0);
    a.check("52. compareName", game::sim::compareName(sa, sb) > 0);
    a.check("53. compareName", game::sim::compareName(sb, sa) < 0);
}
