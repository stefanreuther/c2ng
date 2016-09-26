/**
  *  \file game/historyturnlist.hpp
  */
#ifndef C2NG_GAME_HISTORYTURNLIST_HPP
#define C2NG_GAME_HISTORYTURNLIST_HPP

#include "afl/container/ptrmap.hpp"
#include "game/score/turnscorelist.hpp"
#include "game/historyturn.hpp"

namespace game {

    class TurnLoader;
    class Root;
    class Timestamp;

    class HistoryTurnList {
     public:
        HistoryTurnList();
        ~HistoryTurnList();

        HistoryTurn* get(int nr) const;
        HistoryTurn* create(int nr);

        int findNewestUnknownTurnNumber(int currentTurn) const;

        /** Initialize from turn scores.
            This will initialize the timestamps of all turns for which we have one.

            \param scores Turn score list
            \param turn First turn number to initialize
            \param count Number of turns */
        void initFromTurnScores(const game::score::TurnScoreList& scores, int turn, int count);

        /** Initialize from turn loader.
            This will query the turn loader for all turns that are not yet known.

            \param loader Turn loader
            \param root Root
            \param player Player number to check
            \param turn First turn number to initialize
            \param count Number of turns

            FIXME: the TurnLoader is contained in the root; do we need the distinction? */
        void initFromTurnLoader(TurnLoader& loader, Root& root, int player, int turn, int count);

        /** Get status for one turn.
            This will report the stored status.
            Call initFromTurnLoader(), initFromTurnScores() before calling this if you can to get current data.

            \param turn Turn number
            \return Turn status */
        HistoryTurn::Status getTurnStatus(int turn) const;

        /** Get timestamp for one turn.
            This will report the stored status.
            Call initFromTurnLoader(), initFromTurnScores() before calling this if you can to get current data.

            \param turn Turn number
            \return Turn timestamp */
        Timestamp getTurnTimestamp(int turn) const;

     private:
        afl::container::PtrMap<int, HistoryTurn> m_turns;
    };

}

#endif
