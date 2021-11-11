/**
  *  \file game/vcr/flak/definitions.hpp
  *  \brief FLAK definitions
  */
#ifndef C2NG_GAME_VCR_FLAK_DEFINITIONS_HPP
#define C2NG_GAME_VCR_FLAK_DEFINITIONS_HPP

namespace game { namespace vcr { namespace flak {

    /* Manifest constants for game rules */
    const int FLAK_MAX_BEAMS                        = 20;     ///< Maximum number of beams per object.
    const int FLAK_MAX_TORPS                        = 20;     ///< Maximum number of torpedo launchers per object.
    const int FLAK_MAX_BAYS                         = 50;     ///< Maximum number of fighter bays per object.
    const int FLAK_NUM_OWNERS                       = 12;     ///< Maximum value for TFlakShip::player.

    /* We choose a new enemy every this many ticks. */
    const int FLAK_CHOOSE_ENEMY_TIME                = 30;     ///< Interval between choosing enemies.

    /* Targeting */
    const int FLAK_DIFF_OFFSET                      = 100;    ///< Offset for targeting rating difference.
    const int FLAK_DIVISOR_IF_SMALLER               = 100;    ///< Targeting divisor if we're smaller than the target.
    const int FLAK_DIVISOR_IF_BIGGER                = 250;    ///< Targeting divisor if we're bigger than the target.
    const int FLAK_DIVISOR_SAME_ENEMY_BONUS         = 150;    ///< Targeting divisor bonus for keeping same target.
    const int FLAK_MULTIPLIER_MIN                   = 50;     ///< Minimum targeting multiplier.

    /* Torpedo firepower limitation */
    const int FLAK_TORP_LIMIT_FACTOR                = 120;    ///< Safety factor for torpedo limit.

    /* Compensation */
    const int FLAK_COMPENSATION_LIMIT               = 2;      ///< Maximum compensation bonus.
    const int FLAK_COMPENSATION_DIVISOR             = 1000;   ///< Compensation formula offset.

    /* Combat */
    const int FLAK_TORP_MOVEMENT_SPEED              = 1000;   ///< Torpedo movement speed; meters per tick.
    const int FLAK_FIGHTER_INTERCEPT_RANGE          = 128;    ///< Fighter intercept range; meters.

    /* MaxFightersLaunched */
    const int FLAK_MFL_SCALE                        = 2;      ///< MaxFightersLaunched per bay.
    const int FLAK_MAXIMUM_MFL                      = 50;     ///< Maximum fighters launched.
    const int FLAK_MINIMUM_MFL                      = 1;      ///< Minimum number of fighters allowed for a ship.

} } }

#endif
