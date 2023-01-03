/**
  *  \file server/format/simpacker.cpp
  *
  *  FIXME: This re-implements sim-io.cc somehow, because that has some ugly interdependencies we do not want.
  *  As of 20170122, directly referencing game::sim::Setup enlarges the binary by 200+ kByte,
  *  and that's without any I/O so far.
  */

#include "server/format/simpacker.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/data/access.hpp"
#include "afl/data/hash.hpp"
#include "afl/data/hashvalue.hpp"
#include "afl/data/vector.hpp"
#include "afl/data/vectorvalue.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalstream.hpp"
#include "game/sim/structures.hpp"
#include "game/v3/structures.hpp"
#include "server/types.hpp"
#include "server/errors.hpp"

using afl::data::Access;
using server::makeIntegerValue;
using server::makeStringValue;
namespace gs = game::sim::structures;

namespace {
    int checkVersion(Access obj, int existing)
    {
        int my_version;
        int32_t flags = obj("FLAGS").toInteger();
        if ((flags >> 16) != 0) {
            my_version = 5;
        } else if ((flags & 16 /*fl_RatingOverride*/) != 0) {
            my_version = 4;
        } else if (obj("MISSION.INTERCEPT").toInteger() != 0) {
            my_version = 4;
        } else {
            my_version = 3;
        }

        return my_version > existing ? my_version : existing;
    }

    void packShip(gs::SimShipData& sh, Access p, int version, afl::charset::Charset& cs)
    {
        // ex SimPacker::packShip
        (void) version;

        // Derived values
        int aux_type  = p("AUX").toInteger();
        int aux_count = p("AUX.COUNT").toInteger();
        bool fighters = aux_type == game::v3::structures::NUM_TORPEDO_TYPES+1;
        int32_t flags = p("FLAGS").toInteger();

        // Pack it
        sh.object.name               = cs.encode(afl::string::toMemory(p("NAME").toString()));
        sh.object.damage             = int16_t(p("DAMAGE").toInteger());
        sh.object.crew               = int16_t(p("CREW").toInteger());
        sh.object.id                 = int16_t(p("ID").toInteger());
        sh.object.owner              = uint8_t(p("OWNER").toInteger());
        sh.object.raceOrZero         = 0;
        sh.object.pictureNumber      = 0;
        sh.object.hullTypeOrZero     = 0;
        sh.object.beamType           = int16_t(p("BEAM").toInteger());
        sh.object.numBeams           = uint8_t(p("BEAM.COUNT").toInteger());
        sh.object.experienceLevel    = uint8_t(p("LEVEL").toInteger());
        sh.object.numBays            = int16_t(fighters ? aux_count : 0);
        sh.object.torpedoType        = int16_t(!fighters ? aux_type : 0);
        sh.object.ammo               = int16_t(p("AUX.AMMO").toInteger());
        sh.object.numLaunchersPacked = int16_t(!fighters ? aux_count : 0);
        sh.engineType                = int16_t(p("ENGINE").toInteger());
        sh.hullType                  = int16_t(p("HULL").toInteger());
        sh.shield                    = int16_t(p("SHIELD").toInteger());
        sh.friendlyCode              = cs.encode(afl::string::toMemory(p("FCODE").toString()));
        sh.aggressiveness            = int16_t(p("AGGRESSIVENESS").toInteger());
        sh.mass                      = int16_t(p("MASS").toInteger());
        sh.flags                     = int16_t(flags);
        sh.flakRating                = p("RATING.R").toInteger();
        sh.flakCompensation          = int16_t(p("RATING.C").toInteger());
        sh.interceptId               = int16_t(p("MISSION.INTERCEPT").toInteger());
        sh.flags2                    = int16_t(flags >> 16);
    }

    void packPlanet(gs::SimPlanetData& pl, Access p, int version, afl::charset::Charset& cs)
    {
        // ex SimPacker::packPlanet

        // The minimum version we ever write is 3, so no need
        // to pack for versions 1 or lower that have different storage for torps.
        (void) version;

        // Prepare
        int32_t flags = p("FLAGS").toInteger();
        Access base_ammo = p("STORAGE.AMMO");

        // Pack it
        for (int i = 0; i < gs::NUM_TORPEDO_TYPES; ++i) {
            pl.numTorpedoes[i] = int16_t(base_ammo[i].toInteger());
        }
        pl._pad0               = 0;
        pl.id                  = int16_t(p("ID").toInteger());
        pl.owner               = int16_t(p("OWNER").toInteger());
        pl._pad1               = 0;
        pl.beamTechLevel       = int16_t(p("TECH.BEAM").toInteger());
        pl._pad2               = 0;
        pl.experienceLevel     = uint8_t(p("LEVEL").toInteger());
        pl.numFighters         = int16_t(base_ammo[gs::NUM_TORPEDO_TYPES].toInteger());
        pl._pad3               = 0;
        pl.numTorpedoesOld     = 0;
        pl.torpedoTechLevel    = int16_t(p("TECH.TORPEDO").toInteger());
        pl.numBaseDefensePosts = int16_t(p("DEFENSE.BASE").toInteger());
        pl.numDefensePosts     = int16_t(p("DEFENSE").toInteger());
        pl.shield              = int16_t(p("SHIELD").toInteger());
        pl.friendlyCode        = cs.encode(afl::string::toMemory(p("FCODE").toString()));
        pl.aggressiveness      = int16_t(p("AGGRESSIVENESS").toInteger());
        pl._pad5               = 0;
        pl.flags               = int16_t(flags);
        pl.flakRating          = p("RATING.R").toInteger();
        pl.flakCompensation    = int16_t(p("RATING.C").toInteger());
        pl._pad6               = 0;
        pl.flags2              = int16_t(flags >> 16);
    }

    afl::data::Value* describeShip(const gs::SimShipData& sh, int version, afl::charset::Charset& cs)
    {
        // ex SimPacker::describeShip

        afl::base::Ref<afl::data::Hash> hd = afl::data::Hash::create();

        /* Version dependencies */
        int32_t agg   = version <= 0 ? -1 /*agg_Kill*/ : sh.aggressiveness;
        int32_t flags = (version < 3
                         ? 0
                         : version <= 3
                         ? uint16_t(sh.flags) & ~16 /*fl_RatingOverride*/
                         : version < 5
                         ? uint16_t(sh.flags)
                         : uint16_t(sh.flags) + 65536*sh.flags2);

        /* Map fighters/torps to one, as we do for ships */
        int32_t aux_type = (sh.object.numLaunchersPacked > 0 && sh.object.torpedoType > 0 && sh.object.torpedoType <= gs::NUM_TORPEDO_TYPES
                            ? sh.object.torpedoType
                            : sh.object.numBays > 0
                            ? gs::NUM_TORPEDO_TYPES+1
                            : 0);
        int32_t aux_count = (aux_type > 0 && aux_type <= gs::NUM_TORPEDO_TYPES
                             ? sh.object.numLaunchersPacked
                             : aux_type == gs::NUM_TORPEDO_TYPES+1
                             ? sh.object.numBays
                             : 0);

        /* Build result */
        hd->setNew("AGGRESSIVENESS",    makeIntegerValue(agg));
        hd->setNew("AUX",               makeIntegerValue(aux_type));
        hd->setNew("AUX.AMMO",          makeIntegerValue(sh.object.ammo));
        hd->setNew("AUX.COUNT",         makeIntegerValue(aux_count));
        hd->setNew("BEAM",              makeIntegerValue(sh.object.beamType));
        hd->setNew("BEAM.COUNT",        makeIntegerValue(sh.object.numBeams));
        hd->setNew("CREW",              makeIntegerValue(sh.object.crew));
        hd->setNew("DAMAGE",            makeIntegerValue(sh.object.damage));
        hd->setNew("ENGINE",            makeIntegerValue(sh.engineType));
        hd->setNew("FCODE",             makeStringValue(cs.decode(sh.friendlyCode)));
        hd->setNew("FLAGS",             makeIntegerValue(flags));
        hd->setNew("HULL",              makeIntegerValue(sh.hullType));
        hd->setNew("ID",                makeIntegerValue(sh.object.id));
        hd->setNew("LEVEL",             makeIntegerValue(sh.object.experienceLevel));
        hd->setNew("MASS",              makeIntegerValue(sh.mass));
        hd->setNew("MISSION.INTERCEPT", makeIntegerValue(sh.interceptId));
        hd->setNew("NAME",              makeStringValue(cs.decode(sh.object.name)));
        hd->setNew("OWNER",             makeIntegerValue(sh.object.owner));
        hd->setNew("RATING.C",          makeIntegerValue(sh.flakCompensation));
        hd->setNew("RATING.R",          makeIntegerValue(sh.flakRating));
        hd->setNew("SHIELD",            makeIntegerValue(sh.shield));

        return new afl::data::HashValue(hd);
    }

    afl::data::Value* describePlanet(const gs::SimPlanetData& pl, int version, afl::charset::Charset& cs)
    {
        // ex SimPacker::describePlanet

        /* Derived data */
        int beam_tech    = pl.beamTechLevel >= 0 && pl.beamTechLevel <= game::v3::structures::NUM_BEAM_TYPES ? pl.beamTechLevel : 0;
        int torp_tech    = pl.torpedoTechLevel >= 0 && pl.torpedoTechLevel <= gs::NUM_TORPEDO_TYPES ? pl.torpedoTechLevel : 0;
        int base_defense = pl.numBaseDefensePosts >= 0 && pl.numBaseDefensePosts <= 1000 ? pl.numBaseDefensePosts : 0;
        int defense      = pl.numDefensePosts >= 0 && pl.numDefensePosts < 1000 ? pl.numDefensePosts : 0;
        int32_t flags = (version < 3
                         ? 0
                         : version <= 3
                         ? uint16_t(pl.flags) & ~16 /*fl_RatingOverride*/
                         : version < 5
                         ? uint16_t(pl.flags)
                         : uint16_t(pl.flags) + 65536*pl.flags2);

        afl::base::Ref<afl::data::Vector> base_ammo = afl::data::Vector::create();
        for (int i = 0; i < gs::NUM_TORPEDO_TYPES; ++i) {
            int this_torps = (version > 1
                              ? pl.numTorpedoes[i]
                              : torp_tech == i+1
                              ? pl.numTorpedoesOld
                              : 0);
            base_ammo->pushBackInteger(this_torps);
        }
        base_ammo->pushBackInteger(pl.numFighters);

        /* Build result */
        afl::base::Ref<afl::data::Hash> hd = afl::data::Hash::create();
        hd->setNew("AGGRESSIVENESS",    makeIntegerValue(-1));           // Not editable! --- FIXME: not even in the data!
        hd->setNew("DAMAGE",            makeIntegerValue(0));            // Not editable!
        hd->setNew("DEFENSE",           makeIntegerValue(defense));
        hd->setNew("DEFENSE.BASE",      makeIntegerValue(base_defense));
        hd->setNew("FCODE",             makeStringValue(cs.decode(pl.friendlyCode)));
        hd->setNew("FLAGS",             makeIntegerValue(flags));
        hd->setNew("ID",                makeIntegerValue(pl.id));
        hd->setNew("LEVEL",             makeIntegerValue(pl.experienceLevel));
        hd->setNew("OWNER",             makeIntegerValue(pl.owner));
        hd->setNew("RATING.C",          makeIntegerValue(pl.flakCompensation));
        hd->setNew("RATING.R",          makeIntegerValue(pl.flakRating));
        hd->setNew("SHIELD",            makeIntegerValue(100));          // Not editable!
        hd->setNew("STORAGE.AMMO",      new afl::data::VectorValue(base_ammo));
        hd->setNew("TECH.BEAM",         makeIntegerValue(beam_tech));
        hd->setNew("TECH.TORPEDO",      makeIntegerValue(torp_tech));

        return new afl::data::HashValue(hd);
    }
}

String_t
server::format::SimPacker::pack(afl::data::Value* data, afl::charset::Charset& cs)
{
    // ex SimPacker::pack

    // Figure out key parameters
    Access root(data);
    Access shipArray = root("ships");
    Access planet    = root("planet");
    size_t numShips  = shipArray.getArraySize();

    // Figure out version to write
    int version = checkVersion(planet, 3);
    for (size_t i = 0; i < numShips; ++i) {
        version = checkVersion(shipArray[i], version);
    }

    // Store header
    afl::io::InternalStream result;
    uint8_t header[gs::MAGIC_LENGTH + 4];
    static_assert(sizeof(header) == 10, "sizeof header");
    std::memcpy(header, gs::MAGIC_V1, gs::MAGIC_LENGTH);
    header[6] = uint8_t('0' + version - 1);
    header[7] = gs::TERMINATOR;
    header[8] = uint8_t(numShips & 255);
    header[9] = uint8_t(numShips >> 8);
    if (!planet.isNull()) {
        header[9] |= 0x80;
    }
    result.fullWrite(header);

    // Store ships
    for (size_t i = 0; i < numShips; ++i) {
        gs::SimShipData sh;
        packShip(sh, shipArray[i], version, cs);
        result.fullWrite(afl::base::fromObject(sh).trim(gs::RECORD_SIZES[version]));
    }

    // Store planet
    if (!planet.isNull()) {
        gs::SimPlanetData pl;
        packPlanet(pl, planet, version, cs);
        result.fullWrite(afl::base::fromObject(pl).trim(gs::RECORD_SIZES[version]));
    }
    return afl::string::fromBytes(result.getContent());
}

afl::data::Value*
server::format::SimPacker::unpack(const String_t& data, afl::charset::Charset& cs)
{
    // ex SimPacker::unpack
    afl::base::ConstBytes_t bytes = afl::string::toBytes(data);

    // Figure out version and place initial cursor
    int version;
    if (bytes.subrange(0, gs::MAGIC_LENGTH).equalContent(gs::MAGIC_V0)) {
        version = 0;
        bytes.split(gs::MAGIC_LENGTH);
    } else if (bytes.size() >= gs::MAGIC_LENGTH+2
               && bytes.subrange(0, gs::MAGIC_LENGTH).equalContent(gs::MAGIC_V1)
               && *bytes.at(gs::MAGIC_LENGTH) >= '0'
               && *bytes.at(gs::MAGIC_LENGTH) < '0' + gs::MAX_VERSION)
    {
        version = *bytes.at(gs::MAGIC_LENGTH) - '0' + 1;
        bytes.split(gs::MAGIC_LENGTH+2);
    } else {
        throw std::runtime_error(INVALID_FILE_FORMAT);
    }

    // Make it a stream for convenience
    afl::io::ConstMemoryStream ms(bytes);

    // Check object count
    afl::bits::Value<afl::bits::UInt16LE> count;
    ms.fullRead(count.m_bytes);
    size_t num_ships = (count & 0x7FFF);
    bool has_planet = (count & 0x8000) != 0;

    // Make room for output
    afl::base::Ref<afl::data::Hash> hd = afl::data::Hash::create();

    // Read Ships
    afl::base::Ref<afl::data::Vector> sa = afl::data::Vector::create();
    for (size_t i = 0; i < num_ships; ++i) {
        // Fetch data
        gs::SimShipData data;
        data.aggressiveness = -1;
        data.mass = 100;
        data.flags = 0;
        data.flakRating = 0;
        data.flakCompensation = 0;
        data.interceptId = 0;
        ms.fullRead(afl::base::fromObject(data).trim(gs::RECORD_SIZES[version]));

        // Generate output
        sa->pushBackNew(describeShip(data, version, cs));
    }
    hd->setNew("ships", new afl::data::VectorValue(sa));

    if (has_planet) {
        // Fetch data
        gs::SimPlanetData data;
        data.aggressiveness   = -1;
        data._pad5            = 100;
        data.flags            = 0;
        data.flakRating       = 0;
        data.flakCompensation = 0;
        data._pad6            = 0;
        ms.fullRead(afl::base::fromObject(data).trim(gs::RECORD_SIZES[version]));

        hd->setNew("planet", describePlanet(data, version,cs));
    }

    return new afl::data::HashValue(hd);
}
