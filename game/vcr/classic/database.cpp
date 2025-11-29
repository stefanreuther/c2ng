/**
  *  \file game/vcr/classic/database.cpp
  *  \brief Class game::vcr::classic::Database
  */

#include "game/vcr/classic/database.hpp"
#include "game/v3/structures.hpp"
#include "game/vcr/classic/types.hpp"

namespace gt = game::v3::structures;

using game::config::HostConfiguration;

namespace {
    const uint16_t PHOST_MAGIC = 48879;
    const uint16_t NU_MAGIC = 0x554E;

    /** Check whether battle record bears the PHost magic number. */
    bool hasPHostMagic(const gt::Vcr& vcr)
    {
        // some docs claim that there is another magic, 65261, but I have never seen that one in the wild.
        // Actually, all docs from PHost 1.1 up to 2.13 as well as 3.x and 4.x use 48879.
        return ((vcr.randomSeed + vcr.signature) & 0xFFFFU) == PHOST_MAGIC;
    }


    /** Unpack VCR.
        This unpacks a VCR from a classic VCR file.
        \param raw Data from file
        \param side Side to unpack, 0 or 1 */
    game::vcr::Object unpack(gt::Vcr& in,
                             game::vcr::classic::Side side,
                             const game::config::HostConfiguration& config,
                             afl::charset::Charset& charset)
    {
        // ex GVcrObject::unpack
        // Copy everything
        game::vcr::Object out;
        const gt::VcrObject& obj = in.objects[side];
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

    void packObject(gt::Vcr& out, size_t side, const game::vcr::Object& in, afl::charset::Charset& cs, const HostConfiguration& config, bool isPHost)
    {
        const int owner = in.getOwner();
        const int race = config.getPlayerRaceNumber(in.getOwner());

        gt::VcrObject& obj = out.objects[side];
        obj.name               = cs.encode(afl::string::toMemory(in.getName()));
        obj.damage             = static_cast<int16_t>(in.getDamage());
        obj.crew               = static_cast<int16_t>(in.getCrew());
        obj.id                 = static_cast<int16_t>(in.getId());
        obj.owner              = static_cast<int8_t>(owner);
        obj.raceOrZero         = static_cast<int8_t>((isPHost && owner != race) ? race : 0);
        obj.pictureNumber      = static_cast<int8_t>(in.getPicture());
        obj.hullTypeOrZero     = static_cast<int8_t>(in.getHull());
        obj.beamType           = static_cast<int16_t>(in.getBeamType());
        obj.numBeams           = static_cast<int8_t>(in.getNumBeams());
        obj.experienceLevel    = static_cast<int8_t>(in.getExperienceLevel());
        obj.numBays            = static_cast<int16_t>(in.getNumBays());
        obj.torpedoType        = static_cast<int16_t>(in.getTorpedoType());
        obj.ammo               = static_cast<int16_t>(in.getNumBays() > 0 ? in.getNumFighters() : in.getNumLaunchers() > 0 ? in.getNumTorpedoes() : 0);
        obj.numLaunchersPacked = static_cast<int16_t>((in.isPlanet() && config[HostConfiguration::PlanetsHaveTubes]())
                                                      ? in.getNumLaunchers() + 256*std::min(255, in.getNumTorpedoes())
                                                      : in.getNumLaunchers());
        out.mass[side]         = static_cast<int16_t>(in.getMass());
        out.shield[side]       = static_cast<int16_t>(in.getShield());
    }

    void packBattle(gt::Vcr& out, const game::vcr::classic::Battle& in, bool isFirst, afl::charset::Charset& cs, const HostConfiguration& config)
    {
        const bool isPHost = game::vcr::classic::isPHost(in.getType());
        packObject(out, 0, in.left(),  cs, config, isPHost);
        packObject(out, 1, in.right(), cs, config, isPHost);

        // Seed
        out.randomSeed = in.getSeed();

        // Signature
        switch (in.getType()) {
         case game::vcr::classic::Unknown:
         case game::vcr::classic::Host:
            out.signature = 0;
            break;
         case game::vcr::classic::UnknownPHost:
         case game::vcr::classic::PHost2:
         case game::vcr::classic::PHost3:
         case game::vcr::classic::PHost4:
            out.signature = isFirst ? static_cast<int16_t>(PHOST_MAGIC - in.getSeed()) : 0;
            break;
         case game::vcr::classic::NuHost:
            out.signature = NU_MAGIC;
            break;
        }

        // Flags
        if (isFirst) {
            uint16_t cap = in.getCapabilities();
            if (cap != 0) {
                cap |= gt::ValidCapabilities;
            }
            out.flags = cap;
        } else {
            out.flags = 0;
        }

        // Battle type
        out.battleType = (in.right().isPlanet());
    }

    void packConfig(gt::VcrConfiguration& out, const game::config::HostConfiguration& in)
    {
        // Clear it (in particular, the unused field)
        afl::base::fromObject(out).fill(0);

        out.signature            = 0xB0E00E0F;
        out.version              = 0x0F02;   // 2.15, which does not exist
        out.size                 = 64;
        out.BayRechargeRate      = static_cast<int16_t>(in[HostConfiguration::BayRechargeRate](1));
        out.BayRechargeBonus     = static_cast<int16_t>(in[HostConfiguration::BayRechargeBonus](1));
        out.BeamRechargeRate     = static_cast<int16_t>(in[HostConfiguration::BeamRechargeRate](1));
        out.BeamRechargeBonus    = static_cast<int16_t>(in[HostConfiguration::BeamRechargeBonus](1));
        out.TubeRechargeRate     = static_cast<int16_t>(in[HostConfiguration::TubeRechargeRate](1));
        out.BeamHitFighterCharge = static_cast<int16_t>(in[HostConfiguration::BeamHitFighterCharge](1));
        out.BeamHitShipCharge    = static_cast<int16_t>(in[HostConfiguration::BeamHitShipCharge](1));
        out.TorpFiringRange      =                      in[HostConfiguration::TorpFiringRange](1);
        out.BeamFiringRange      =                      in[HostConfiguration::BeamFiringRange](1);
        out.TorpHitOdds          = static_cast<int16_t>(in[HostConfiguration::TorpHitOdds](1));
        out.BeamHitOdds          = static_cast<int16_t>(in[HostConfiguration::BeamHitOdds](1));
        out.BeamHitBonus         = static_cast<int16_t>(in[HostConfiguration::BeamHitBonus](1));
        out.StrikesPerFighter    = static_cast<int16_t>(in[HostConfiguration::StrikesPerFighter](1));
        out.FighterKillOdds      = static_cast<int16_t>(in[HostConfiguration::FighterKillOdds](1));
        out.FighterBeamExplosive = static_cast<int16_t>(in[HostConfiguration::FighterBeamExplosive](1));
        out.FighterBeamKill      = static_cast<int16_t>(in[HostConfiguration::FighterBeamKill](1));
        out.ShipMovementSpeed    = static_cast<int16_t>(in[HostConfiguration::ShipMovementSpeed](1));
        out.FighterMovementSpeed = static_cast<int16_t>(in[HostConfiguration::FighterMovementSpeed](1));
        out.BayLaunchInterval    = static_cast<int16_t>(in[HostConfiguration::BayLaunchInterval](1));
        out.MaxFightersLaunched  = static_cast<int16_t>(in[HostConfiguration::MaxFightersLaunched](1));
        out.AlternativeCombat    = static_cast<int16_t>(in[HostConfiguration::AllowAlternativeCombat]());
        out.StandoffDistance     =                      in[HostConfiguration::StandoffDistance]();
        out.PlanetsHaveTubes     = static_cast<int16_t>(in[HostConfiguration::PlanetsHaveTubes]());
        out.FireOnAttackFighters = static_cast<int16_t>(in[HostConfiguration::FireOnAttackFighters]());
        out.TorpHitBonus         = static_cast<int16_t>(in[HostConfiguration::TorpHitBonus](1));
        out.TubeRechargeBonus    = static_cast<int16_t>(in[HostConfiguration::TubeRechargeBonus](1));
        out.ShieldDamageScaling  = static_cast<int16_t>(in[HostConfiguration::ShieldDamageScaling](1));
        out.HullDamageScaling    = static_cast<int16_t>(in[HostConfiguration::HullDamageScaling](1));
        out.CrewKillScaling      = static_cast<int16_t>(in[HostConfiguration::CrewKillScaling](1));
    }
}


game::vcr::classic::Database::Database()
    : m_battles()
{
    // ex GClassicVcrDatabase::GClassicVcrDatabase
}

game::vcr::classic::Database::~Database()
{ }

void
game::vcr::classic::Database::load(afl::io::Stream& file,
                                   const game::config::HostConfiguration& config,
                                   afl::charset::Charset& charset)
{
    // ex GClassicVcrDatabase::load, ccmain.pas:LoadVCRs, ccmain.pas:CheckCurrentVCRs
    gt::Vcr rawVcr;
    gt::Int16_t rawCount;

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
                                rawVcr.randomSeed, rawVcr.signature));
    }

    // If it hasn't been detected as PHost 2, it might be 3 or newer
    if (mayBePHost && type == Host) {
        type = PHost3;
        if ((firstFlags & gt::ValidCapabilities) != 0) {
            capabilities = uint16_t(firstFlags & ~gt::ValidCapabilities);
        }
        if (capabilities != 0) {
            type = PHost4;
        }
    }

    // If it still looks like Host, it might be NuHost VCRs unpacked by c2nu
    if (type == Host && firstSignature == NU_MAGIC) {
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

void
game::vcr::classic::Database::save(afl::io::Stream& out, size_t first, size_t num, const game::config::HostConfiguration& config, afl::charset::Charset& cs)
{
    // ex vcrplay.pas:SaveVCRs
    // Check parameters
    first = std::min(first, m_battles.size());
    num   = std::min(num, m_battles.size() - first);
    num   = std::min(num, size_t(0x7FFE));                // so we can safely add one for PHost 2

    // Count
    bool useConfig = false;
    if (const Battle* b = getBattle(first)) {
        useConfig = (b->getType() == PHost2);
    }

    gt::Int16_t count;
    count = static_cast<int16_t>(num + useConfig);
    out.fullWrite(count.m_bytes);

    // Battles
    for (size_t i = 0; i < num; ++i) {
        if (const Battle* b = getBattle(first + i)) {
            gt::Vcr vcr;
            packBattle(vcr, *b, i == 0, cs, config);
            out.fullWrite(afl::base::fromObject(vcr));
        }
    }

    // Config
    if (useConfig) {
        gt::VcrConfiguration vcr;
        packConfig(vcr, config);
        out.fullWrite(afl::base::fromObject(vcr));
    }
}
