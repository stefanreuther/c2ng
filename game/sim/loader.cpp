/**
  *  \file game/sim/loader.cpp
  *  \brief Class game::sim::Loader
  *
  *  PCC2 Comment:
  *
  *    There are multiple versions of the .ccb file format. Here, we
  *    adopt the numbering used in the file format list, i.e. 0 =
  *    "CCsim", 1 = "CCbsim0", 2 = "CCbsim1" etc. Files differ in record
  *    sizes (more data for recent features added to the end), and in
  *    content (version <= 1 has only one starbase torp type).
  *
  *    We read all file formats, but only save a selection of the more
  *    recent ones. In this case, we generally use version 3, and use 4
  *    if its features (FLAK rating overrides) are required.
  */

#include <cstring>
#include "game/sim/loader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/structures.hpp"

namespace st = game::sim::structures;

namespace {
    struct Header {
        uint8_t     signature[st::MAGIC_LENGTH];
        char        version;
        uint8_t     terminator;
        st::Int16_t count;
    };

    int getMinimumRequiredVersion(const game::sim::Object& obj)
    {
        int result;
        if ((obj.getFlags() & ~0xFFFF) != 0) {
            result = 5;
        } else if ((obj.getFlags() & game::sim::Object::fl_RatingOverride) != 0) {
            result = 4;
        } else {
            const game::sim::Ship* sh = dynamic_cast<const game::sim::Ship*>(&obj);
            if (sh != 0 && sh->getInterceptId() != 0) {
                result = 4;
            } else {
                result = 3;
            }
        }
        return result;
    }
}


// Constructor.
game::sim::Loader::Loader(afl::charset::Charset& cs, afl::string::Translator& tx)
    : m_charset(cs),
      m_translator(tx)
{ }

// Load a setup.
void
game::sim::Loader::load(afl::io::Stream& in, Setup& setup)
{
    // ex GSimState::load, ccsim.pas:LoadBattleSetupFromFile

    // Check signature
    size_t version;
    uint8_t signatureBuffer[structures::MAGIC_LENGTH];
    in.fullRead(signatureBuffer);
    if (std::memcmp(signatureBuffer, structures::MAGIC_V0, structures::MAGIC_LENGTH) == 0) {
        version = 0;
    } else if (std::memcmp(signatureBuffer, structures::MAGIC_V1, structures::MAGIC_LENGTH) == 0) {
        uint8_t magic[2];
        in.fullRead(magic);
        version = magic[0] - '0' + 1;
        if (magic[0] < '0' || magic[1] != structures::TERMINATOR || version > structures::MAX_VERSION) {
            throw afl::except::FileFormatException(in, m_translator("Unsupported file format version"));
        }
    } else {
        throw afl::except::FileFormatException(in, m_translator("File is missing required signature"));
    }

    // Check object count
    structures::Int16_t count;
    in.fullRead(count.m_bytes);

    const size_t numShips = count & 0x7FFF;
    const bool hasPlanet = (count & 0x8000) != 0;

    const size_t recordSize = structures::RECORD_SIZES[version];

    // Read objects
    for (size_t i = 0; i < numShips; ++i) {
        structures::SimShipData data;
        in.fullRead(afl::base::fromObject(data).trim(recordSize));

        // FIXME: report null as error
        if (Ship* sh = setup.addShip()) {
            sh->setId(data.object.id);
            sh->setName(m_charset.decode(data.object.name));
            sh->setDamage(data.object.damage);
            sh->setCrew(data.object.crew);
            sh->setOwner(data.object.owner);
            sh->setBeamType(data.object.beamType);
            sh->setNumBeams(data.object.numBeams);
            sh->setTorpedoType(data.object.torpedoType);
            sh->setNumLaunchers(data.object.numLaunchersPacked);
            sh->setNumBays(data.object.numBays);
            sh->setAmmo(data.object.ammo);
            sh->setExperienceLevel(data.object.experienceLevel);
            sh->setEngineType(data.engineType);
            sh->setHullTypeOnly(data.hullType);
            sh->setShield(data.shield);
            sh->setFriendlyCode(m_charset.decode(data.friendlyCode));

            if (version > 0) {
                sh->setAggressiveness(data.aggressiveness);
            } else {
                sh->setAggressiveness(Ship::agg_Kill);
            }

            if (version < 3) {
                sh->setFlags(0);
            } else if (version < 5) {
                sh->setFlags(data.flags);
            } else {
                sh->setFlags(uint16_t(data.flags) + 65536*data.flags2);
            }

            if (version < 3) {
                sh->setMass(100);
            } else {
                sh->setMass(data.mass);
            }

            if (version < 4) {
                sh->setFlakRatingOverride(0);
                sh->setFlakCompensationOverride(0);
                sh->setInterceptId(0);
                sh->setFlags(sh->getFlags() & ~Ship::fl_RatingOverride);
            } else {
                sh->setFlakRatingOverride(data.flakRating);
                sh->setFlakCompensationOverride(data.flakCompensation);
                sh->setInterceptId(data.interceptId);
            }
        }
    }

    if (hasPlanet) {
        structures::SimPlanetData data;
        in.fullRead(afl::base::fromObject(data).trim(recordSize));

        // FIXME: report null as error
        if (Planet* pl = setup.addPlanet()) {
            pl->setId(data.id);
            pl->setOwner(data.owner);
            pl->setBaseBeamTech(data.beamTechLevel);
            pl->setExperienceLevel(data.experienceLevel);
            pl->setNumBaseFighters(data.numFighters);
            pl->setBaseTorpedoTech(data.torpedoTechLevel);
            pl->setBaseDefense(data.numBaseDefensePosts);
            pl->setDefense(data.numDefensePosts);

            // FIXME: .shield field is ignored?
            pl->setShield(100);
            pl->setDamage(0);

            pl->setFriendlyCode(m_charset.decode(data.friendlyCode));

            for (int i = 1; i <= structures::NUM_TORPEDO_TYPES; ++i) {
                pl->setNumBaseTorpedoes(i, (version > 1
                                            ? data.numTorpedoes[i-1]
                                            : pl->getBaseTorpedoTech() == i
                                            ? data.numTorpedoesOld
                                            : 0));
            }

            // FIXME: .aggressiveness is ignored, planets don't have it. But why is it in the structure?

            if (version < 3) {
                pl->setFlags(0);
            } else if (version < 5) {
                pl->setFlags(data.flags);
            } else {
                pl->setFlags(uint16_t(data.flags) + 65536*data.flags2);
            }

            if (version < 4) {
                pl->setFlakRatingOverride(0);
                pl->setFlakCompensationOverride(0);
                pl->setFlags(pl->getFlags() & ~Ship::fl_RatingOverride);
            } else {
                pl->setFlakRatingOverride(data.flakRating);
                pl->setFlakCompensationOverride(data.flakCompensation);
            }
        }
    }
}

void
game::sim::Loader::save(afl::io::Stream& out, const Setup& setup)
{
    // ex GSimState::save, ccsim.pas:SaveBattleSetupToFile

    // Figure out what version to save in
    int version = 3;
    for (Setup::Slot_t i = 0, n = setup.getNumObjects(); i < n; ++i) {
        if (const Object* p = setup.getObject(i)) {
            version = std::max(version, getMinimumRequiredVersion(*p));
        }
    }

    // Write header
    Header h;
    std::memcpy(h.signature, st::MAGIC_V1, sizeof(st::MAGIC_V1));
    h.version = static_cast<char>('0' + version - 1);
    h.terminator = st::TERMINATOR;
    if (setup.hasPlanet()) {
        h.count = static_cast<int16_t>(setup.getNumShips() | 0x8000);
    } else {
        h.count = static_cast<int16_t>(setup.getNumShips());
    }
    out.fullWrite(afl::base::fromObject(h));

    // Write ships
    for (Setup::Slot_t i = 0, n = setup.getNumShips(); i < n; ++i) {
        if (const Ship* sh = setup.getShip(i)) {
            // ex GSimShip::storeData
            st::SimShipData data;
            data.object.name               = m_charset.encode(afl::string::toMemory(sh->getName()));
            data.object.damage             = static_cast<int16_t>(sh->getDamage());
            data.object.crew               = static_cast<int16_t>(sh->getCrew());
            data.object.id                 = static_cast<int16_t>(sh->getId());
            data.object.owner              = static_cast<uint8_t>(sh->getOwner());
            data.object.raceOrZero         = 0;      // unused field
            data.object.pictureNumber      = 0;      // unused field
            data.object.hullTypeOrZero     = 0;      // unused field, hull is stored separately
            data.object.beamType           = static_cast<int16_t>(sh->getBeamType());
            data.object.numBeams           = static_cast<uint8_t>(sh->getNumBeams());
            data.object.experienceLevel    = static_cast<uint8_t>(sh->getExperienceLevel());
            data.object.numBays            = static_cast<int16_t>(sh->getNumBays());
            data.object.torpedoType        = static_cast<int16_t>(sh->getTorpedoType());
            data.object.ammo               = static_cast<int16_t>(sh->getAmmo());
            data.object.numLaunchersPacked = static_cast<int16_t>(sh->getNumLaunchers());
            data.engineType                = static_cast<int16_t>(sh->getEngineType());
            data.hullType                  = static_cast<int16_t>(sh->getHullType());
            data.shield                    = static_cast<int16_t>(sh->getShield());
            data.friendlyCode              = m_charset.encode(afl::string::toMemory(sh->getFriendlyCode()));
            data.aggressiveness            = static_cast<int16_t>(sh->getAggressiveness());
            data.mass                      = static_cast<int16_t>(sh->getMass());
            data.flags                     = static_cast<int16_t>(sh->getFlags() & 0xFFFF);
            data.flakRating                = sh->getFlakRatingOverride();
            data.flakCompensation          = static_cast<int16_t>(sh->getFlakCompensationOverride());
            data.interceptId               = static_cast<int16_t>(sh->getInterceptId());
            data.flags2                    = static_cast<int16_t>(sh->getFlags() >> 16);

            out.fullWrite(afl::base::fromObject(data).trim(st::RECORD_SIZES[version]));
        }
    }

    // Write planet
    if (const Planet* p = setup.getPlanet()) {
        st::SimPlanetData data;

        // ex GSimPlanet::storeData
        // @change Removed support for version <= 1
        for (int i = 0; i < st::NUM_TORPEDO_TYPES; ++i) {
            data.numTorpedoes[i] = static_cast<int16_t>(p->getNumBaseTorpedoes(i+1));
        }
        data._pad0               = 0;
        data.id                  = static_cast<int16_t>(p->getId());
        data.owner               = static_cast<int16_t>(p->getOwner());
        data._pad1               = 0;
        data.beamTechLevel       = static_cast<int16_t>(p->getBaseBeamTech());
        data._pad2               = 0;
        data.experienceLevel     = static_cast<uint8_t>(p->getExperienceLevel());
        data.numFighters         = static_cast<int16_t>(p->getNumBaseFighters());
        data._pad3               = 0;
        data.numTorpedoesOld     = 0;
        data.torpedoTechLevel    = static_cast<int16_t>(p->getBaseTorpedoTech());
        data.numBaseDefensePosts = static_cast<int16_t>(p->getBaseDefense());
        data.numDefensePosts     = static_cast<int16_t>(p->getDefense());
        data.shield              = static_cast<int16_t>(p->getShield());
        data.friendlyCode        = m_charset.encode(afl::string::toMemory(p->getFriendlyCode()));
        data.aggressiveness      = -1;
        data._pad5               = 0;
        data.flags               = static_cast<int16_t>(p->getFlags() & 0xFFFF);
        data.flakRating          = p->getFlakRatingOverride();
        data.flakCompensation    = static_cast<int16_t>(p->getFlakCompensationOverride());
        data._pad6               = 0;
        data.flags2              = static_cast<int16_t>(p->getFlags() >> 16);

        out.fullWrite(afl::base::fromObject(data).trim(st::RECORD_SIZES[version]));
    }
}
