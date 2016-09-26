/**
  *  \file game/playerbitmatrix.hpp
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
        PlayerBitMatrix();

        bool get(int subj, int obj) const;
        void set(int subj, int obj, bool value);

        void clear();

        PlayerSet_t getRow(int subj) const;

     private:
        PlayerSet_t m_data[MAX_PLAYERS];
    };

}

#endif
