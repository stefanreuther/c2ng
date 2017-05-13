/**
  *  \file server/format/truehullpacker.cpp
  *  \brief Class server::format::TruehullPacker
  */

#include "server/format/truehullpacker.hpp"
#include "game/v3/structures.hpp"
#include "afl/data/access.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "server/types.hpp"

String_t
server::format::TruehullPacker::pack(afl::data::Value* data, afl::charset::Charset& /*cs*/)
{
    // ex TruehullPacker::pack
    afl::data::Access p(data);

    game::v3::structures::Truehull th;
    for (int pl = 0; pl < game::v3::structures::NUM_PLAYERS; ++pl) {
        for (int i = 0; i < game::v3::structures::NUM_HULLS_PER_PLAYER; ++i) {
            th.hulls[pl][i] = int16_t(p[pl][i].toInteger());
        }
    }

    return afl::string::fromBytes(afl::base::fromObject(th));
}

afl::data::Value*
server::format::TruehullPacker::unpack(const String_t& data, afl::charset::Charset& /*cs*/)
{
    // ex TruehullPacker::unpack
    afl::data::Vector::Ref_t vec(afl::data::Vector::create());

    game::v3::structures::Truehull th;
    if (data.size() >= sizeof(th)) {
        afl::base::fromObject(th).copyFrom(afl::string::toBytes(data));
        for (int pl = 0; pl < game::v3::structures::NUM_PLAYERS; ++pl) {
            afl::data::Vector::Ref_t innerVector(afl::data::Vector::create());
            for (int i = 0; i < game::v3::structures::NUM_HULLS_PER_PLAYER; ++i) {
                innerVector->pushBackInteger(th.hulls[pl][i]);
            }
            vec->pushBackNew(new afl::data::VectorValue(innerVector));
        }
    }

    return new afl::data::VectorValue(vec);
}
