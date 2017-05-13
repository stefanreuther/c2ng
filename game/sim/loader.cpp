/**
  *  \file game/sim/loader.cpp
  *  \brief Class game::sim::Loader
  */

#include <cstring>
#include "game/sim/loader.hpp"
#include "afl/except/fileformatexception.hpp"
#include "game/sim/planet.hpp"
#include "game/sim/setup.hpp"
#include "game/sim/ship.hpp"
#include "game/sim/structures.hpp"
#include "util/translation.hpp"

// Constructor.
game::sim::Loader::Loader(afl::charset::Charset& cs)
    : m_charset(cs)
{ }

// Load a setup.
void
game::sim::Loader::load(afl::io::Stream& in, Setup& setup)
{
    // ex GSimState::load

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
            throw afl::except::FileFormatException(in, _("Unsupported file format version"));
        }
    } else {
        throw afl::except::FileFormatException(in, _("File is missing required signature"));
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

        if (Ship* sh = setup.addShip()) {
            sh->setId(data.object.id);
            sh->setName(m_charset.decode(data.object.name));
            sh->setDamage(data.object.damage);
            sh->setCrew(data.object.crew);
            sh->setOwner(data.object.owner);
            sh->setBeamType(data.object.beamType);
            sh->setNumBeams(data.object.numBeams);
            sh->setTorpedoType(data.object.launcherType);
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
