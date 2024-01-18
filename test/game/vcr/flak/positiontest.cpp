/**
  *  \file test/game/vcr/flak/positiontest.cpp
  *  \brief Test for game::vcr::flak::Position
  */

#include "game/vcr/flak/position.hpp"
#include "afl/test/testrunner.hpp"

AFL_TEST("game.vcr.flak.Position", a)
{
    game::vcr::flak::Position pa;
    game::vcr::flak::Position pb(1000, 2000, 3000);
    game::vcr::flak::Position pc(1500, 2000, 9000);
    game::vcr::flak::Position pd(4000, 5000, 3000);

    a.checkEqual("01. x", pa.x, 0);
    a.checkEqual("02. x", pb.x, 1000);

    a.checkEqual("11. distanceTo", pa.distanceTo(pa), 0.0);
    a.checkEqual("12. distanceTo", pb.distanceTo(pb), 0.0);
    a.checkEqual("13. distanceTo", pb.distanceTo(pc), 500.0);

    a.checkEqual("21. isDistanceLERadius", pa.isDistanceLERadius(pa, 10), true);
    a.checkEqual("22. isDistanceLERadius", pb.isDistanceLERadius(pc, 500), true);
    a.checkEqual("23. isDistanceLERadius", pb.isDistanceLERadius(pc, 499), false);
    a.checkEqual("24. isDistanceLERadius", pb.isDistanceLERadius(pd, 4242), false);
    a.checkEqual("25. isDistanceLERadius", pb.isDistanceLERadius(pd, 4243), true);

    a.checkEqual("31. eq", pa == pa, true);
    a.checkEqual("32. ne", pa != pa, false);
    a.checkEqual("33. eq", pa == pb, false);
    a.checkEqual("34. ne", pa != pb, true);
}
