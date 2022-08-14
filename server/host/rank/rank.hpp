/**
  *  \file server/host/rank/rank.hpp
  *  \brief Utilities for Player Ranking
  */
#ifndef C2NG_SERVER_HOST_RANK_RANK_HPP
#define C2NG_SERVER_HOST_RANK_RANK_HPP

#include "afl/base/types.hpp"
#include "game/playerset.hpp"
#include "server/host/game.hpp"

namespace server { namespace host { namespace rank {

    /** List of ranks or scores. */
    // FIXME: convert to use PlayerArray?
    typedef int32_t Rank_t[Game::NUM_PLAYERS];

    /** Initialize ranks.
        Set all entries to the given value.
        @param [out] ranks  Ranks
        @param [in]  value  Value */
    void initRanks(Rank_t& ranks, int32_t value);

    /** Compact ranks.
        Given a list of ranks ("slot 3 is 4th place"),
        builds a new list where places are assigned continuously starting from 1, and ties are broken using scores.
        As special cases,
        - to build ranks according to a single score,
          pass all-the-same as @c rank, and the score as @c score
        - to build ranks according to one score, using another as tie-breaker,
          pass the negated first score as @c rank, and the tie-breaker as @c score.

        @param [out] dest    Output ranks
        @param [in]  rank    Input ranks, smaller is better
        @param [in]  score   Input scores, used as tie-breaker for equal input ranks, bigger is better
        @param [in]  players Only consider these slots of the @c Rank_t objects; do not touch the others */
    void compactRanks(Rank_t& dest, const Rank_t& rank, const Rank_t& score, game::PlayerSet_t players);

} } }

#endif
