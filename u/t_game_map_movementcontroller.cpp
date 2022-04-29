/**
  *  \file u/t_game_map_movementcontroller.cpp
  *  \brief Test for game::map::MovementController
  */

#include "game/map/movementcontroller.hpp"

#include "t_game_map.hpp"
#include "game/map/configuration.hpp"

using game::map::MovementController;
using game::map::Point;
using game::map::Configuration;

/** Test initialisation.
    A: set initial position
    E: position immediately reported as current position */
void
TestGameMapMovementController::testInit()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4300));

    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), false); // no more change
}

/** Test slow movement.
    A: set initial position. Perform movement <= animation threshold.
    E: position immediately taken over */
void
TestGameMapMovementController::testSlowMovement()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);

    testee.setAnimationThreshold(20);
    testee.setTargetPosition(Point(1200, 4320));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4320));

    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), false); // no more change
}

/** Test fast movement.
    A: set initial position. Perform movement > animation threshold.
    E: position updated in individual steps */
void
TestGameMapMovementController::testFastMovement()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);

    testee.setAnimationThreshold(20);
    testee.setTargetPosition(Point(1200, 4321));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4301));  // speed 1, 20 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4303));  // speed 2, 18 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4306));  // speed 3, 15 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4310));  // speed 4, 11 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4313));  // speed 3, 8 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4315));  // speed 2, 5 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4318));  // speed 3, 3 remaining
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1200, 4321));  // finish

    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), false); // no more change
}

/** Test fast movement, diagonal.
    A: set initial position. Perform diagonal movement > animation threshold.
    E: position updated in individual steps; verify some steps */
void
TestGameMapMovementController::testFastMovementDiagonal()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1300, 4200));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1201, 4299));  // speed 1 [0.7 -> 1.0]
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1202, 4298));  // speed 2 [1.4 -> 1.0]
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1204, 4296));  // speed 3 [2.1 -> 2.0]

    TS_ASSERT_EQUALS(testee.update(Configuration(), 100), true);       // finish
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1300, 4200));
}

/** Test fast movement, flat (slight slope).
    A: set initial position. Perform almost-horizontal movement > animation threshold.
    E: position updated in individual steps; verify some steps. Checks the "guarantee progress" part */
void
TestGameMapMovementController::testFastMovementFlat()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1300, 4302));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1201, 4301));  // speed 1, moves at least one in target direction
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1202, 4302));  // speed 2; we're not rounding for now!
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1205, 4302));  // speed 3

    TS_ASSERT_EQUALS(testee.update(Configuration(), 100), true);       // finish
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1300, 4302));
}

/** Test fast movement, steep slope.
    A: set initial position. Perform almost-vertical movement > animation threshold.
    E: position updated in individual steps; verify some steps. Checks the "guarantee progress" part */
void
TestGameMapMovementController::testFastMovementSteep()
{
    MovementController testee;
    testee.setTargetPosition(Point(1200, 4300));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);

    testee.setTargetPosition(Point(1198, 4200));
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1199, 4299));  // speed 1, moves at least one in target direction
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1198, 4298));  // speed 2; we're not rounding for now!
    TS_ASSERT_EQUALS(testee.update(Configuration(), 1), true);
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1198, 4295));  // speed 3

    TS_ASSERT_EQUALS(testee.update(Configuration(), 100), true);       // finish
    TS_ASSERT_EQUALS(testee.getCurrentPosition(), Point(1198, 4200));
}

