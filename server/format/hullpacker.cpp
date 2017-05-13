/**
  *  \file server/format/hullpacker.cpp
  *  \brief Class server::format::HullPacker
  */

#include "server/format/hullpacker.hpp"
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
  Hull().BEAM.MAX                         hullX.BEAM.MAX
  Hull().CARGO.MAX                        hullX.CARGO.MAX
  Hull().CARGO.MAXFUEL                    hullX.CARGO.MAXFUEL
  Hull().COST.D                           hullX.COST->D
  Hull().COST.M                           hullX.COST->M
  Hull().COST.MC                          hullX.COST->MC
  Hull().COST.STR                         -
  Hull().COST.T                           hullX.COST->T
  Hull().CREW.NORMAL                      hullX.CREW.NORMAL
  Hull().ENGINE.COUNT                     hullX.ENGINE.COUNT
  Hull().FIGHTER.BAYS                     hullX.FIGHTER.BAYS
  Hull().ID                               -
  Hull().IMAGE                            hullX.IMAGE
  Hull().IMAGE$                           -
  Hull().MASS                             hullX.MASS
  Hull().NAME                             hullX.NAME
  Hull().NAME.SHORT                       -
  Hull().SPECIAL                          -
  Hull().TECH                             hullX.TECH
  Hull().TECH.HULL                        -
  Hull().TORP.LMAX                        hullX.TORP.LMAX
*/

String_t
server::format::HullPacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex HullPacker::pack
    afl::io::InternalStream out;
    afl::data::Access p(data);
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        afl::data::Access pp(p[i]);
        afl::data::Access pcost(pp("COST"));
        game::v3::structures::Hull hull;

        hull.name          = cs.encode(afl::string::toMemory(pp("NAME").toString()));
        hull.pictureNumber = int16_t(pp("IMAGE").toInteger());
        hull.zero          = 1;   // that's what c2format classic does
        hull.tritanium     = int16_t(pcost("T").toInteger());
        hull.duranium      = int16_t(pcost("D").toInteger());
        hull.molybdenum    = int16_t(pcost("M").toInteger());
        hull.maxFuel       = int16_t(pp("CARGO.MAXFUEL").toInteger());
        hull.maxCrew       = int16_t(pp("CREW.NORMAL").toInteger());
        hull.numEngines    = int16_t(pp("ENGINE.COUNT").toInteger());
        hull.mass          = int16_t(pp("MASS").toInteger());
        hull.techLevel     = int16_t(pp("TECH").toInteger());
        hull.maxCargo      = int16_t(pp("CARGO.MAX").toInteger());
        hull.numBays       = int16_t(pp("FIGHTER.BAYS").toInteger());
        hull.maxLaunchers  = int16_t(pp("TORP.LMAX").toInteger());
        hull.maxBeams      = int16_t(pp("BEAM.MAX").toInteger());
        hull.money         = int16_t(pcost("MC").toInteger());

        out.fullWrite(afl::base::fromObject(hull));
    }
    return afl::string::fromBytes(out.getContent());
}

afl::data::Value*
server::format::HullPacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex HullPacker::unpack
    afl::base::Ref<afl::data::Vector> vec(afl::data::Vector::create());
    afl::io::ConstMemoryStream in(afl::string::toBytes(data));
    game::v3::structures::Hull hull;
    int hullId = 0;
    while (in.read(afl::base::fromObject(hull)) == sizeof(hull)) {
        // Remap picture numbers.
        // It's ugly to do this here, but this makes it somehow consistent with c2server.
        // FIXME: do we need this?
        ++hullId;
        int picId = hullId == 104 ? 152 : hullId == 105 ? 153 : hull.pictureNumber;

        game::v3::structures::Cost c;
        c.money      = hull.money;
        c.tritanium  = hull.tritanium;
        c.duranium   = hull.duranium;
        c.molybdenum = hull.molybdenum;

        // Create the object
        afl::base::Ref<afl::data::Hash> h(afl::data::Hash::create());
        h->setNew("NAME",          makeStringValue(cs.decode(hull.name)));
        h->setNew("IMAGE",         makeIntegerValue(picId));
        h->setNew("COST",          unpackCost(c));
        h->setNew("CARGO.MAXFUEL", makeIntegerValue(hull.maxFuel));
        h->setNew("CREW.NORMAL",   makeIntegerValue(hull.maxCrew));
        h->setNew("ENGINE.COUNT",  makeIntegerValue(hull.numEngines));
        h->setNew("MASS",          makeIntegerValue(hull.mass));
        h->setNew("TECH",          makeIntegerValue(hull.techLevel));
        h->setNew("CARGO.MAX",     makeIntegerValue(hull.maxCargo));
        h->setNew("FIGHTER.BAYS",  makeIntegerValue(hull.numBays));
        h->setNew("TORP.LMAX",     makeIntegerValue(hull.maxLaunchers));
        h->setNew("BEAM.MAX",      makeIntegerValue(hull.maxBeams));

        vec->pushBackNew(new afl::data::HashValue(h));
    }
    return new afl::data::VectorValue(vec);
}

