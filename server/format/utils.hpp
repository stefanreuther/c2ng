/**
  *  \file server/format/utils.hpp
  */
#ifndef C2NG_SERVER_FORMAT_UTILS_HPP
#define C2NG_SERVER_FORMAT_UTILS_HPP

#include "game/v3/structures.hpp"
#include "afl/data/access.hpp"

namespace server { namespace format {

    afl::data::Value* unpackCost(const game::v3::structures::Cost& c);

    void packCost(game::v3::structures::Cost& c, afl::data::Access a);

} }

#endif
