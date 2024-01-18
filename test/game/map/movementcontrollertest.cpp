/**
  *  \file test/game/map/movementcontrollertest.cpp
  *  \brief Test for game::map::MovementController
  */

#include "game/map/movementcontroller.hpp"

#include "afl/test/testrunner.hpp"
#include "game/map/configuration.hpp"

using game::map::MovementController;
using game::map::Point;
using game::map::Configuration;

/** Test initialisation.
    A: set initial position
    E: position immediately reported as current position */
AFL_TEST("game.map.MovementController:init", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);
    a.checkEqual("02. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4300));

    a.checkEqual("11. update", testee.update(Configuration(), 1), false); // no more change
}

/** Test slow movement.
    A: set initial position. Perform movement <= animation threshold.
    E: position immediately taken over */
AFL_TEST("game.map.MovementController:slow-movement", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);

    testee.setAnimationThreshold(20);
    testee.setTargetPosition(Point(1200, 4320));
    a.checkEqual("11. update", testee.update(Configuration(), 1), true);
    a.checkEqual("12. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4320));

    a.checkEqual("21. update", testee.update(Configuration(), 1), false); // no more change
}

/** Test fast movement.
    A: set initial position. Perform movement > animation threshold.
    E: position updated in individual steps */
AFL_TEST("game.map.MovementController:fast-movement", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);

    testee.setAnimationThreshold(20);
    testee.setTargetPosition(Point(1200, 4321));
    a.checkEqual("11. update", testee.update(Configuration(), 1), true);
    a.checkEqual("12. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4301));  // speed 1, 20 remaining
    a.checkEqual("13. update", testee.update(Configuration(), 1), true);
    a.checkEqual("14. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4303));  // speed 2, 18 remaining
    a.checkEqual("15. update", testee.update(Configuration(), 1), true);
    a.checkEqual("16. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4306));  // speed 3, 15 remaining
    a.checkEqual("17. update", testee.update(Configuration(), 1), true);
    a.checkEqual("18. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4310));  // speed 4, 11 remaining
    a.checkEqual("19. update", testee.update(Configuration(), 1), true);
    a.checkEqual("20. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4313));  // speed 3, 8 remaining
    a.checkEqual("21. update", testee.update(Configuration(), 1), true);
    a.checkEqual("22. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4315));  // speed 2, 5 remaining
    a.checkEqual("23. update", testee.update(Configuration(), 1), true);
    a.checkEqual("24. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4318));  // speed 3, 3 remaining
    a.checkEqual("25. update", testee.update(Configuration(), 1), true);
    a.checkEqual("26. getCurrentPosition", testee.getCurrentPosition(), Point(1200, 4321));  // finish

    a.checkEqual("31. update", testee.update(Configuration(), 1), false); // no more change
}

/** Test fast movement, diagonal.
    A: set initial position. Perform diagonal movement > animation threshold.
    E: position updated in individual steps; verify some steps */
AFL_TEST("game.map.MovementController:fast-movement:diagonal", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1300, 4200));
    a.checkEqual("11. update", testee.update(Configuration(), 1), true);
    a.checkEqual("12. getCurrentPosition", testee.getCurrentPosition(), Point(1201, 4299));  // speed 1 [0.7 -> 1.0]
    a.checkEqual("13. update", testee.update(Configuration(), 1), true);
    a.checkEqual("14. getCurrentPosition", testee.getCurrentPosition(), Point(1202, 4298));  // speed 2 [1.4 -> 1.0]
    a.checkEqual("15. update", testee.update(Configuration(), 1), true);
    a.checkEqual("16. getCurrentPosition", testee.getCurrentPosition(), Point(1204, 4296));  // speed 3 [2.1 -> 2.0]

    a.checkEqual("21. update", testee.update(Configuration(), 100), true);       // finish
    a.checkEqual("22. getCurrentPosition", testee.getCurrentPosition(), Point(1300, 4200));
}

/** Test fast movement, flat (slight slope).
    A: set initial position. Perform almost-horizontal movement > animation threshold.
    E: position updated in individual steps; verify some steps. Checks the "guarantee progress" part */
AFL_TEST("game.map.MovementController:fast-movement:sloped", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1300, 4302));
    a.checkEqual("11. update", testee.update(Configuration(), 1), true);
    a.checkEqual("12. getCurrentPosition", testee.getCurrentPosition(), Point(1201, 4301));  // speed 1, moves at least one in target direction
    a.checkEqual("13. update", testee.update(Configuration(), 1), true);
    a.checkEqual("14. getCurrentPosition", testee.getCurrentPosition(), Point(1202, 4302));  // speed 2; we're not rounding for now!
    a.checkEqual("15. update", testee.update(Configuration(), 1), true);
    a.checkEqual("16. getCurrentPosition", testee.getCurrentPosition(), Point(1205, 4302));  // speed 3

    a.checkEqual("21. update", testee.update(Configuration(), 100), true);       // finish
    a.checkEqual("22. getCurrentPosition", testee.getCurrentPosition(), Point(1300, 4302));
}

/** Test fast movement, steep slope.
    A: set initial position. Perform almost-vertical movement > animation threshold.
    E: position updated in individual steps; verify some steps. Checks the "guarantee progress" part */
AFL_TEST("game.map.MovementController:fast-movement:steep", a)
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    a.checkEqual("01. update", testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1198, 4200));
    a.checkEqual("11. update", testee.update(Configuration(), 1), true);
    a.checkEqual("12. getCurrentPosition", testee.getCurrentPosition(), Point(1199, 4299));  // speed 1, moves at least one in target direction
    a.checkEqual("13. update", testee.update(Configuration(), 1), true);
    a.checkEqual("14. getCurrentPosition", testee.getCurrentPosition(), Point(1198, 4298));  // speed 2; we're not rounding for now!
    a.checkEqual("15. update", testee.update(Configuration(), 1), true);
    a.checkEqual("16. getCurrentPosition", testee.getCurrentPosition(), Point(1198, 4295));  // speed 3

    a.checkEqual("21. update", testee.update(Configuration(), 100), true);       // finish
    a.checkEqual("22. getCurrentPosition", testee.getCurrentPosition(), Point(1198, 4200));
}
