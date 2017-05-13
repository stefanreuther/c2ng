/**
  *  \file server/format/enginepacker.cpp
  *  \brief Class server::format::EnginePacker
  */

#include "server/format/enginepacker.hpp"
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
  Engine().COST.D                         engine[].COST->D
  Engine().COST.M                         engine[].COST->M
  Engine().COST.MC                        engine[].COST->MC
  Engine().COST.STR                       -
  Engine().COST.T                         engine[].COST->T
  Engine().FUELFACTOR                     engine[].FUELFACTOR[]
  Engine().ID                             -
  Engine().NAME                           engine[].NAME
  Engine().NAME.SHORT                     -
  Engine().SPEED$                         engine[].SPEED -- not in packer
  Engine().TECH                           engine[].TECH
  Engine().TECH.ENGINE                    -
*/

String_t
server::format::EnginePacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex EnginePacker::pack
    afl::io::InternalStream out;
    afl::data::Access p(data);
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access pp(p[i]);
        afl::data::Access pfactor(pp("FUELFACTOR"));
        game::v3::structures::Engine engine;

        engine.name = cs.encode(afl::string::toMemory(pp("NAME").toString()));
        packCost(engine.cost, pp("COST"));
        engine.techLevel = int16_t(pp("TECH").toInteger());
        for (int i = 0; i < game::v3::structures::NUM_WARP_FACTORS; ++i) {
            engine.fuelFactors[i] = pfactor[i+1].toInteger();
        }

        out.fullWrite(afl::base::fromObject(engine));
    }
    return afl::string::fromBytes(out.getContent());
}

afl::data::Value*
server::format::EnginePacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex EnginePacker::unpack
    afl::base::Ref<afl::data::Vector> vec(afl::data::Vector::create());
    afl::io::ConstMemoryStream in(afl::string::toBytes(data));
    game::v3::structures::Engine engine;
    while (in.read(afl::base::fromObject(engine)) == sizeof(engine)) {
        afl::base::Ref<afl::data::Hash> h(afl::data::Hash::create());
        h->setNew("NAME", makeStringValue(cs.decode(engine.name)));
        h->setNew("COST", unpackCost(engine.cost));
        h->setNew("TECH", makeIntegerValue(engine.techLevel));

        afl::base::Ref<afl::data::Vector> f(afl::data::Vector::create());
        f->pushBackInteger(0);
        for (int i = 0; i < game::v3::structures::NUM_WARP_FACTORS; ++i) {
            f->pushBackInteger(engine.fuelFactors[i]);
        }
        h->setNew("FUELFACTOR", new afl::data::VectorValue(f));

        vec->pushBackNew(new afl::data::HashValue(h));
    }
    return new afl::data::VectorValue(vec);
}

