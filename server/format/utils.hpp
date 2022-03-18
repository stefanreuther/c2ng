/**
  *  \file server/format/utils.hpp
  *  \brief Format Server Utilities
  */
#ifndef C2NG_SERVER_FORMAT_UTILS_HPP
#define C2NG_SERVER_FORMAT_UTILS_HPP

#include "game/v3/structures.hpp"
#include "afl/data/access.hpp"

namespace server { namespace format {

    /** Unpack a Cost.
        \param c [in] Data
        \return Hash containing the cost */
    afl::data::Value* unpackCost(const game::v3::structures::Cost& c);

    /** Pack a TCost.
        \param c [out] Data
        \param a [in] Hash provided by user */
    void packCost(game::v3::structures::Cost& c, afl::data::Access a);

} }

#endif
