/**
  *  \file server/format/beampacker.cpp
  *  \brief Class server::format::BeamPacker
  */

#include "server/format/beampacker.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "game/v3/structures.hpp"
#include "server/format/utils.hpp"
#include "server/types.hpp"

/*
  Beam().COST.D                           beam[].COST->D
  Beam().COST.M                           beam[].COST->M
  Beam().COST.MC                          beam[].COST->MC
  Beam().COST.STR                         -
  Beam().COST.T                           beam[].COST->T
  Beam().DAMAGE                           beam[].DAMAGE
  Beam().ID                               -
  Beam().KILL                             beam[].KILL
  Beam().MASS                             beam[].MASS
  Beam().NAME                             beam[].NAME
  Beam().NAME.SHORT                       -
  Beam().TECH                             beam[].TECH
  Beam().TECH.BEAM                        -
*/

String_t
server::format::BeamPacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex BeamPacker::pack
    afl::io::InternalStream out;
    afl::data::Access p(data);
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access pp(p[i]);
        game::v3::structures::Beam beam;

        beam.name = cs.encode(afl::string::toMemory(pp("NAME").toString()));
        packCost(beam.cost, pp("COST"));
        beam.mass        = int16_t(pp("MASS").toInteger());
        beam.techLevel   = int16_t(pp("TECH").toInteger());
        beam.killPower   = int16_t(pp("KILL").toInteger());
        beam.damagePower = int16_t(pp("DAMAGE").toInteger());
        out.fullWrite(afl::base::fromObject(beam));
    }
    return afl::string::fromBytes(out.getContent());
}

afl::data::Value*
server::format::BeamPacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex BeamPacker::unpack
    afl::base::Ref<afl::data::Vector> vec(afl::data::Vector::create());
    afl::io::ConstMemoryStream in(afl::string::toBytes(data));
    game::v3::structures::Beam beam;
    while (in.read(afl::base::fromObject(beam)) == sizeof(beam)) {
        afl::base::Ref<afl::data::Hash> h(afl::data::Hash::create());
        h->setNew("NAME", makeStringValue(cs.decode(beam.name)));
        h->setNew("COST", unpackCost(beam.cost));
        h->setNew("MASS", makeIntegerValue(beam.mass));
        h->setNew("TECH", makeIntegerValue(beam.techLevel));
        h->setNew("KILL", makeIntegerValue(beam.killPower));
        h->setNew("DAMAGE", makeIntegerValue(beam.damagePower));
        vec->pushBackNew(new afl::data::HashValue(h));
    }
    return new afl::data::VectorValue(vec);
}

