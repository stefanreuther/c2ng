/**
  *  \file game/limits.hpp
  *  \brief Game engine limits
  */
#ifndef C2NG_GAME_LIMITS_HPP
#define C2NG_GAME_LIMITS_HPP

namespace game {

    /** Maximum number of experience levels. */
    const int MAX_EXPERIENCE_LEVELS = 10;

    /** Maximum number of players.
        This is finite because we use bitsets to represent set-of-players. */
    const int MAX_PLAYERS = 31;

    /** Maximum number of races.
        This is finite because we use bitsets to represent set-of-races. */
    const int MAX_RACES = 31;

    /** Maximum number in a normal attribute.
        Used at several places to detect unreasonable values (e.g. coordinates, amounts, etc.).
        This corresponds to v3's usual maximum value for a 16-bit field. */
    const int MAX_NUMBER = 10000;

}

#endif
