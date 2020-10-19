/**
  *  \file game/historyturnlist.hpp
  *  \brief Class game::HistoryTurnList
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

    /** List of history turns. */
    class HistoryTurnList {
     public:
        /** Constructor.
            Makes an empty list. */
        HistoryTurnList();

        /** Destructor. */
        ~HistoryTurnList();

        /** Get HistoryTurn object by turn number.
            \param nr Turn number
            \return HistoryTurn object; can be null */
        HistoryTurn* get(int nr) const;

        /** Create HistoryTurn object by turn number.
            \param nr Turn number
            \return HistoryTurn object; can be null if turn number is invalid */
        HistoryTurn* create(int nr);

        /** Find newest unknown turn number.
            \param currentTurn Current turn (not in HistoryTurnList; assumed to be known)
            \return Greatest turn number that is not known, i.e. marked as HistoryTurn::Unknown or not contained in the HistoryTurnList,
                    and precedes a known turn (currentTurn or known HistoryTurn); can be 0. */
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
