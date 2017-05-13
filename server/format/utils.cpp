/**
  *  \file server/format/utils.cpp
  */

#include "server/format/utils.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/hash.hpp"
#include "server/types.hpp"

// /** Unpack a TCost.
//     \param cost [in] Data
//     \return Hash containing the cost */
afl::data::Value*
server::format::unpackCost(const game::v3::structures::Cost& c)
{
    // ex planetscentral/format/specpacker.cc:unpackCost
    afl::base::Ref<afl::data::Hash> hash(afl::data::Hash::create());
    hash->setNew("MC", makeIntegerValue(c.money));
    hash->setNew("T",  makeIntegerValue(c.tritanium));
    hash->setNew("D",  makeIntegerValue(c.duranium));
    hash->setNew("M",  makeIntegerValue(c.molybdenum));
    return new afl::data::HashValue(hash);
}

// /** Pack a TCost.
//     \param cost [out] Data
//     \param pc [in] Hash provided by user */
void
server::format::packCost(game::v3::structures::Cost& c, afl::data::Access a)
{
    // ex planetscentral/format/specpacker.cc:packCost
    c.money      = int16_t(a("MC").toInteger());
    c.tritanium  = int16_t(a("T").toInteger());
    c.duranium   = int16_t(a("D").toInteger());
    c.molybdenum = int16_t(a("M").toInteger());
}
