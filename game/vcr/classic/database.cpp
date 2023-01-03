/**
  *  \file game/vcr/classic/database.cpp
  *  \brief Class game::vcr::classic::Database
  */

#include "game/vcr/classic/database.hpp"
#include "game/v3/structures.hpp"
#include "game/vcr/classic/types.hpp"

namespace {
    /** Check whether battle record bears the PHost magic number. */
    bool hasPHostMagic(const game::v3::structures::Vcr& vcr)
    {
        // some docs claim that there is another magic, 65261, but I have never seen that one in the wild.
        // Actually, all docs from PHost 1.1 up to 2.13 as well as 3.x and 4.x use 48879.
        return ((vcr.randomSeed + vcr.signature) & 0xFFFFU) == 48879;
    }


    /** Unpack VCR.
        This unpacks a VCR from a classic VCR file.
        \param raw Data from file
        \param side Side to unpack, 0 or 1 */
    game::vcr::Object unpack(game::v3::structures::Vcr& in,
                             game::vcr::classic::Side side,
                             const game::config::HostConfiguration& config,
                             afl::charset::Charset& charset)
    {
        // ex GVcrObject::unpack
        // Copy everything
        game::vcr::Object out;
        const game::v3::structures::VcrObject& obj = in.objects[side];
        out.setMass(in.mass[side]);
        out.setShield(in.shield[side]);
        out.setDamage(obj.damage);
        out.setCrew(obj.crew);
        out.setId(obj.id);
        out.setOwner(obj.owner);
        out.setRace(obj.raceOrZero);
        out.setPicture(obj.pictureNumber);
        out.setHull(obj.hullTypeOrZero);
        out.setBeamType(obj.beamType);
        out.setNumBeams(obj.numBeams);
        out.setTorpedoType(obj.torpedoType);
        out.setNumBays(obj.numBays);
        out.setExperienceLevel(obj.experienceLevel);
        out.setIsPlanet(side == game::vcr::classic::RightSide && in.battleType != 0);
        out.setName(charset.decode(obj.name));

        // Handle torps/fighters
        if (obj.numLaunchersPacked != 0) {
            if (out.isPlanet() && config[config.PlanetsHaveTubes]()) {
                // It's a planet with tubes
                out.setNumLaunchers(obj.numLaunchersPacked & 0xFF);
                out.setNumTorpedoes((obj.numLaunchersPacked >> 8) & 0xFF);
                out.setNumFighters(obj.ammo);
            } else {
                // It has just torps
                out.setNumLaunchers(obj.numLaunchersPacked);
                out.setNumTorpedoes(obj.ammo);
                out.setNumFighters(0);
            }
        } else {
            // No torps, but may have regular fighters
            out.setNumLaunchers(0);
            out.setNumTorpedoes(0);
            out.setNumFighters(out.getNumBays() != 0 ? obj.ammo : 0);
        }

        // Silent fixes, avoid confusing display
        if (out.getBeamType() == 0) {
            out.setNumBeams(0);
        }
        if (out.getTorpedoType() == 0) {
            out.setNumLaunchers(0);
            out.setNumTorpedoes(0);
        }

        // Set Nu extensions to defaults; these are not transferred in VCR.DAT files
        out.setBeamKillRate(config[config.PlayerRace](out.getOwner()) == 5 ? 3 : 1);
        out.setBeamChargeRate(1);
        out.setTorpMissRate(35);
        out.setTorpChargeRate(1);
        out.setCrewDefenseRate(0);
        return out;
    }
}


// /** Construct blank VCR database. */
game::vcr::classic::Database::Database()
    : m_battles()
{
    // ex GClassicVcrDatabase::GClassicVcrDatabase
}

// /** Destructor. */
game::vcr::classic::Database::~Database()
{ }

// /** Load a file. */
void
game::vcr::classic::Database::load(afl::io::Stream& file,
                                   const game::config::HostConfiguration& config,
                                   afl::charset::Charset& charset)
{
    // ex GClassicVcrDatabase::load, ccmain.pas:LoadVCRs, ccmain.pas:CheckCurrentVCRs
    game::v3::structures::Vcr rawVcr;
    game::v3::structures::Int16_t rawCount;

    // read count
    file.fullRead(afl::base::fromObject(rawCount));

    int16_t count = rawCount;
    bool mayBePHost = false;
    uint16_t firstFlags = 0;
    uint16_t capabilities = 0;
    uint16_t firstSignature = 0;
    Type type = Host;

    // read entries
    while (count > 0) {
        --count;
        file.fullRead(afl::base::fromObject(rawVcr));

        if (m_battles.empty()) {
            // this is the first VCR
            mayBePHost = hasPHostMagic(rawVcr);
            firstFlags  = rawVcr.flags;
            firstSignature = rawVcr.signature;
        } else if (mayBePHost && count == 0) {
            // this is the last VCR. Some things to note:
            // - it may be PHost 2's config battle. Discard it but memorize type.
            // - it may be the bogus battle from CORR (all but name zeroed)
            if (hasPHostMagic(rawVcr) && rawVcr.battleType >= 2) {
                // PHost config battle. We assume that the PConfig is correct
                // and do not extract it from the file. Just discard the config
                // battle. Check the version field, though.
                if ((rawVcr.flags & 255) == 2) {
                    // PHost 2
                    type = PHost2;
                } else {
                    // PHost 1 or something entirely different
                    type = UnknownPHost;
                }
                break;
            }
            if (rawVcr.objects[0].owner == 0 || rawVcr.objects[1].owner == 0) {
                // CORR
                break;
            }
        } else {
            // normal
        }

        // Remember it
        addNewBattle(new Battle(unpack(rawVcr, LeftSide, config, charset),
                                unpack(rawVcr, RightSide, config, charset),
                                rawVcr.randomSeed, rawVcr.signature, rawVcr.flags));
    }

    // If it hasn't been detected as PHost 2, it might be 3 or newer
    if (mayBePHost && type == Host) {
        type = PHost3;
        if ((firstFlags & game::v3::structures::ValidCapabilities) != 0) {
            capabilities = uint16_t(firstFlags & ~game::v3::structures::ValidCapabilities);
        }
        if (capabilities != 0) {
            type = PHost4;
        }
    }

    // If it still looks like Host, it might be NuHost VCRs unpacked by c2nu
    if (type == Host && firstSignature == 0x554E) {
        type = NuHost;
    }

    // OK, now store type in all VCRs.
    for (afl::container::PtrVector<Battle>::iterator i = m_battles.begin(); i != m_battles.end(); ++i) {
        (*i)->setType(type, capabilities);
        if (type == Host) {
            (*i)->applyClassicLimits();
        }
    }

    // FIXME: PCC1 checks VCR type against availability of pconfig.src, content against PlayerRace settings
}

game::vcr::classic::Battle*
game::vcr::classic::Database::addNewBattle(Battle* battle)
{
    // ex GClassicVcrDatabase::addEntry
    return m_battles.pushBackNew(battle);
}

size_t
game::vcr::classic::Database::getNumBattles() const
{
    // ex GClassicVcrDatabase::getNumBattles
    return m_battles.size();
}

game::vcr::classic::Battle*
game::vcr::classic::Database::getBattle(size_t nr)
{
    // ex GClassicVcrDatabase::getBattle
    if (nr < m_battles.size()) {
        return m_battles[nr];
    } else {
        return 0;
    }
}
