/**
  *  \file game/playerbitmatrix.hpp
  *  \brief Class game::PlayerBitMatrix
  */
#ifndef C2NG_GAME_PLAYERBITMATRIX_HPP
#define C2NG_GAME_PLAYERBITMATRIX_HPP

#include "game/playerset.hpp"
#include "game/limits.hpp"

namespace game {

    /** Bit Matrix of Players
        This class provides a matrix of bits, such as is used for alliance relations.

        For ease of reference, parameters are called "subject" and "object" here.
        For example, "allies.get(2,9)" asks whether player 2 has offered an alliance to player 9.

        PlayerBitMatrix permits indexes from 1 to MAX_PLAYERS. */
    class PlayerBitMatrix {
     public:
        /** Constructor.
            Make empty matrix (get() returns false for all parameters). */
        PlayerBitMatrix();

        /** Get one bit.
            \param subj Subject
            \param obj  Object
            \return bit value; false if parameters are out of range. */
        bool get(int subj, int obj) const;

        /** Set one bit.
            \param subj  Subject
            \param obj   Object
            \param value New value */
        void set(int subj, int obj, bool value);

        /** Get one row.
            \param subj Subject
            \return set of all objects for this subject. */
        PlayerSet_t getRow(int subj) const;

        /** Clear this set. */
        void clear();

     private:
        PlayerSet_t m_data[MAX_PLAYERS];
    };

}

#endif
