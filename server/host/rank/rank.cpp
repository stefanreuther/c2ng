/**
  *  \file server/host/rank/rank.cpp
  */

#include "server/host/rank/rank.hpp"

// /** Initialize ranks.
//     \param ranks [out] Output
//     \param value [in] Value to initialize with */
void
server::host::rank::initRanks(Rank_t& ranks, int32_t value)
{
    // ex planetscentral/host/rank.h:initRanks
    for (int i = 0; i < Game::NUM_PLAYERS; ++i) {
        ranks[i] = value;
    }
}

// /** Compact ranks.
//     \param dest    [out] Output ranks
//     \param rank    [in]  Input ranks, smaller is better
//     \param score   [in]  Input scores, used as tie-breaker for equal input ranks, bigger is better
//     \param players [in]  Mask for slots used in either Rank_t */
void
server::host::rank::compactRanks(Rank_t& dest, const Rank_t& rank, const Rank_t& score, game::PlayerSet_t players)
{
    // ex planetscentral/host/rank.h:compactRanks
    int place = 1;
    while (1) {
        /* Locate best-possible entry */
        game::PlayerSet_t thisSlots;   // players on this rank
        int32_t    thisRank = 0;       // current value from rank
        int32_t    thisScore = 0;      // current value from score
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (players.contains(i)) {
                if (thisSlots.empty()
                    || rank[i-1] < thisRank
                    || (rank[i-1] == thisRank && score[i-1] > thisScore))
                {
                    thisSlots = game::PlayerSet_t(i);
                    thisRank  = rank[i-1];
                    thisScore = score[i-1];
                } else if (rank[i-1] == thisRank && score[i-1] == thisScore) {
                    thisSlots += i;
                }
            }
        }

        /* Termination condition */
        if (thisSlots.empty()) {
            break;
        }

        /* Mark it */
        players -= thisSlots;
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (thisSlots.contains(i)) {
                dest[i-1] = place;
            }
        }
        ++place;
    }
}

