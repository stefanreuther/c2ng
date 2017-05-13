/**
  *  \file server/format/torpedopacker.cpp
  *  \brief Class server::format::TorpedoPacker
  */

#include "server/format/torpedopacker.hpp"
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
  Torpedo().COST.D                        torp[].TORPCOST->D
  Torpedo().COST.M                        torp[].TORPCOST->M
  Torpedo().COST.MC                       torp[].TORPCOST->MC
  Torpedo().COST.STR                      -
  Torpedo().COST.T                        torp[].TORPCOST->T
  Launcher().COST.D                       torp[].TUBECOST->D
  Launcher().COST.M                       torp[].TUBECOST->M
  Launcher().COST.MC                      torp[].TUBECOST->MC
  Launcher().COST.STR                     -
  Launcher().COST.T                       torp[].TUBECOST->T
  Torpedo().DAMAGE                        torp[].DAMAGE1            -- not doubled like in c2server, hence renamed!
  Torpedo().ID                            -
  Torpedo().KILL                          torp[].KILL1              -- not doubled like in c2server, hence renamed!
  Launcher().MASS                         torp[].MASS     of launcher
  Torpedo().NAME                          torp[].NAME
  Torpedo().NAME.SHORT                    -
  Torpedo().TECH                          torp[].TECH
  Torpedo().TECH.TORPEDO                  -
*/

String_t
server::format::TorpedoPacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex TorpedoPacker::pack
    afl::io::InternalStream out;
    afl::data::Access p(data);
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access pp(p[i]);
        game::v3::structures::Torpedo torpedo;

        torpedo.name         = cs.encode(afl::string::toMemory(pp("NAME").toString()));
        packCost(torpedo.launcherCost, pp("TUBECOST"));
        torpedo.torpedoCost  = int16_t(pp("TORPCOST")("MC").toInteger());
        torpedo.launcherMass = int16_t(pp("MASS").toInteger());
        torpedo.techLevel    = int16_t(pp("TECH").toInteger());
        torpedo.killPower    = int16_t(pp("KILL1").toInteger());
        torpedo.damagePower  = int16_t(pp("DAMAGE1").toInteger());
        out.fullWrite(afl::base::fromObject(torpedo));
    }
    return afl::string::fromBytes(out.getContent());
}

afl::data::Value*
server::format::TorpedoPacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex TorpedoPacker::unpack
    afl::base::Ref<afl::data::Vector> vec(afl::data::Vector::create());
    afl::io::ConstMemoryStream in(afl::string::toBytes(data));
    game::v3::structures::Torpedo torpedo;

    /* A typical torpspec file has a few bytes at its end that do not correspond
       to a real torpedo. Thus, limit unpacking to ten (NUM_TORPS) elements. */
    int count = 0;
    while (count < game::v3::structures::NUM_TORPEDO_TYPES
           && in.read(afl::base::fromObject(torpedo)) == sizeof(torpedo))
    {
        ++count;

        game::v3::structures::Cost torpCost;
        torpCost.money = torpedo.torpedoCost;
        torpCost.tritanium = torpCost.duranium = torpCost.molybdenum = 1;

        afl::base::Ref<afl::data::Hash> h(afl::data::Hash::create());
        h->setNew("NAME",     makeStringValue(cs.decode(torpedo.name)));
        h->setNew("TORPCOST", unpackCost(torpCost));
        h->setNew("TUBECOST", unpackCost(torpedo.launcherCost));
        h->setNew("MASS",     makeIntegerValue(torpedo.launcherMass));
        h->setNew("TECH",     makeIntegerValue(torpedo.techLevel));
        h->setNew("KILL1",    makeIntegerValue(torpedo.killPower));
        h->setNew("DAMAGE1",  makeIntegerValue(torpedo.damagePower));
        vec->pushBackNew(new afl::data::HashValue(h));
    }
    return new afl::data::VectorValue(vec);
}
