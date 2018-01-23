/**
  *  \file server/host/rank/rank.hpp
  */
#ifndef C2NG_SERVER_HOST_RANK_RANK_HPP
#define C2NG_SERVER_HOST_RANK_RANK_HPP

#include "server/host/game.hpp"
#include "afl/base/types.hpp"
#include "game/playerset.hpp"

namespace server { namespace host { namespace rank {

    // FIXME: convert to use PlayerArray?
    typedef int32_t Rank_t[Game::NUM_PLAYERS];

    void initRanks(Rank_t& ranks, int32_t value = 0x7FFFFFFF);
    void compactRanks(Rank_t& dest, const Rank_t& rank, const Rank_t& score, game::PlayerSet_t players);

} } }

#endif
