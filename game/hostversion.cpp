/**
  *  \file game/hostversion.cpp
  *  \brief Class game::HostVersion (ex GHost)
  */

#include "game/hostversion.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "util/math.hpp"
#include "util/stringparser.hpp"

using game::config::HostConfiguration;

namespace {
    String_t formatVersion(String_t hostName, int32_t version, bool host)
    {
        if (version == 0) {
            return hostName;
        } else {
            int patch = version % 1000;
            if (patch == 0) {
                return afl::string::Format("%s %d.%d", hostName, version/100000, version%100000/1000);
            } else if (!host && patch <= 26) {
                return afl::string::Format("%s %d.%d%c", hostName, version/100000, version%100000/1000, 'a'-1 + patch);
            } else {
                return afl::string::Format("%s %d.%d.%03d", hostName, version/100000, version%100000/1000, patch);
            }
        }
    }

    int32_t parseHostVersion(const String_t& text, bool host)
    {
        // ex game/storage/overview.cc:parseHostVersion, readmsg.pas::ParseHostVersion
        util::StringParser p(text);
        while (p.parseCharacter(' ') || p.parseCharacter('v'))
            ;

        // Major number
        int val;
        if (!p.parseInt(val) || val < 0) {
            return 0;
        }
        int major = val;
        int minor = 0;
        int patch = 0;

        // Minor number
        if (p.parseCharacter('.')) {
            if (!p.parseInt(val) || val < 0) {
                return 0;
            }
            // THost: 3.0, 3.1, 3.14, 3.2, 3.21
            // PHost: 2.7, 2.8, 2.9, 2.10, ...
            minor = val;
            if (host && minor < 10) {
                minor *= 10;
            }
        }

        // Patchlevel
        if (p.parseCharacter('.')) {
            if (p.parseInt(val) && val >= 0) {
                patch = val;
            }
        } else {
            char ch;
            if (p.getCurrentCharacter().get(ch) && ch >= 'a' && ch <= 'z') {
                patch = (ch - 'a' + 1);
            }
        }
        return MKVERSION(major, minor, patch);
    }
}

// Format as string.
String_t
game::HostVersion::toString() const
{
    // Host names are proper names and are also used on the fromString interface; thus they should not be translated.
    // Users should normally not see "unknown", so not translating that is reasonable.
    switch (m_kind) {
     case Unknown:
        return "unknown";

     case Host:
        return formatVersion("Host", m_version, true);

     case SRace:
        return formatVersion("SRace", m_version, true);

     case PHost:
        return formatVersion("PHost", m_version, false);

     case NuHost:
        return formatVersion("NuHost", m_version, true);
    }
    return String_t();
}

// Parse from strings.
bool
game::HostVersion::fromString(String_t hostType, String_t hostVersion)
{
    hostType = afl::string::strLCase(hostType);
    if (!hostVersion.empty()) {
        if (hostType == "host") {
            set(Host, parseHostVersion(hostVersion, true));
            return true;
        }
        if (hostType == "srace") {
            set(SRace, parseHostVersion(hostVersion, true));
            return true;
        }
        if (hostType == "phost") {
            set(PHost, parseHostVersion(hostVersion, false));
            return true;
        }
        if (hostType == "nuhost") {
            set(NuHost, parseHostVersion(hostVersion, true));
            return true;
        }
    }
    return false;
}

// Parse from single string.
bool
game::HostVersion::fromString(String_t str)
{
    String_t::size_type p = str.find(' ');
    return p != String_t::npos
        && fromString(str.substr(0, p), str.substr(p+1));
}

// Get ship command argument limit.
int32_t
game::HostVersion::getCommandArgumentLimit() const
{
    // ex mission.pas:MissionArgLimit
    // \change This differs from PCC2, but is consistent with PCC1.
    if (m_kind == PHost) {
        if (m_version >= MKVERSION(3,3,2)) {
            return MAX_NUMBER;
        } else {
            return 500;
        }
    } else {
        // No way to know whether it's Host999.
        return 999;
    }
}

// Check whether this host version has Death Rays.
bool
game::HostVersion::hasDeathRays() const
{
    return (m_kind == PHost && m_version >= MKVERSION(4,0,0));
}

// Check whether this host has experience levels.
bool
game::HostVersion::hasExperienceLevels() const
{
    return (m_kind == PHost && m_version >= MKVERSION(4,0,0));
}

// Check whether this host has ship-specific hull functions.
bool
game::HostVersion::hasShipSpecificFunctions() const
{
    return (m_kind == PHost && m_version >= MKVERSION(4,0,0));
}

// Check whether hullfunc.txt assignments are cumulative in this host version.
bool
game::HostVersion::hasCumulativeHullfunc() const
{
    return (m_kind == PHost
            && (m_version >= MKVERSION(4,0,9)
                || (m_version < MKVERSION(4,0,0)
                    && m_version >= MKVERSION(3,4,11))));
}

// Check whether ImperialAssault implies PlanetImmunity ability.
bool
game::HostVersion::hasImmuneAssaultShip() const
{
    return (m_kind != PHost || m_version < MKVERSION(4,0,9));
}

// Check whether this host has restrictions in loading high-tech torps onto low-tech bases.
bool
game::HostVersion::hasHighTechTorpedoBug() const
{
    return (m_kind != PHost && m_version >= MKVERSION(3,22,31));
}

// Check whether siliconoid natives have desert advantage in this host.
bool
game::HostVersion::hasSiliconoidDesertAdvantage() const
{
    return (m_kind != PHost || (m_kind == PHost && m_version >= MKVERSION(3,3,3)));
}

// Check whether this host allows large cargo transfers.
bool
game::HostVersion::hasLargeCargoTransfer() const
{
    return (m_kind == PHost || (m_kind != PHost && m_version <= MKVERSION(3,22,30)));
}

// Check whether the "Lay mines in" mission automatically fills in the minefield owner.
bool
game::HostVersion::hasAutomaticMineIdentity() const
{
    return (m_kind == PHost && m_version >= MKVERSION(3,4,3));
}

// Check whether host allows negative numeric friendly codes.
bool
game::HostVersion::hasNegativeFCodes() const
{
    return m_kind == PHost && m_version >= MKVERSION(2,9,0);
}

// Check whether host allows space-padding in numeric friendly codes.
bool
game::HostVersion::hasSpacePaddedFCodes() const
{
    return m_kind == PHost &&
        (m_version >= MKVERSION(4,0,8)
         || (m_version < MKVERSION(4,0,0)
             && m_version >= MKVERSION(3,4,10)));
}

// Get the maximum tax for this race.
int
game::HostVersion::getNativeTaxRateLimit(int player, const game::config::HostConfiguration& config) const
{
    // ex GHost::getNativeTaxRateLimit
    if (m_kind != PHost) {
        int race = config.getPlayerRaceNumber(player);
        if (race == 6) {
            return 20;
        } else if (race == 2) {
            return 75;
        } else {
            return 100;
        }
    } else {
        return 100;
    }
}

// Get the maximum tax for this race.
int
game::HostVersion::getColonistTaxRateLimit(int player, const game::config::HostConfiguration& config) const
{
    // ex GHost::getColonistTaxRateLimit
    if (m_kind != PHost && config.getPlayerRaceNumber(player) == 2) {
        return 75;
    } else {
        return 100;
    }
}

// Check whether PHost rounds in mining formulas.
bool
game::HostVersion::isPHostRoundingMiningResults() const
{
    return m_version >= MKVERSION(4,1,0)
        || (m_version < MKVERSION(4,0,0)
            && m_version >= MKVERSION(3,5,0));
}

// Check for exact hyperjump distance.
bool
game::HostVersion::isExactHyperjumpDistance2(int32_t distSquared) const
{
    // ex GHost::isExactHyperjumpDistance2
    // ex shipacc.pas:IsExactHyperjump
    if (m_kind != PHost && m_version < MKVERSION(3,20,0)) {
        // These hosts do waypoint trimming, so all jumps are inexact.
        // FIXME: PCC 1.x additionally tests for Dosplan TRN format
        // and Host < 3.22.019, because those trim waypoints too early.
        return false;
    } else {
        // PHost tests >=340, <=360; Host tests >340, <360
        int32_t adjust = m_kind == PHost ? 0 : 1;
        return distSquared-adjust >= 340L*340
            && distSquared+adjust <= 360L*360;
    }
}

// Check for bug in UseAccurateFuelModel computation.
bool
game::HostVersion::hasAccurateFuelModelBug() const
{
    return m_kind == PHost
        && ((m_version < MKVERSION(3,4,8))
            || (m_version >= MKVERSION(4,0,0)
                && m_version < MKVERSION(4,0,5)));
}

// Check for alchemy combination function support.
bool
game::HostVersion::hasAlchemyCombinations() const
{
    return m_kind == PHost
        && (m_version >= MKVERSION(4,0,9)              // 4.0i
            || (m_version < MKVERSION(4,0,0)
                && m_version >= MKVERSION(3,4,11)));   // 3.4k
}

// Check for refinery friendly code support.
bool
game::HostVersion::hasRefineryFCodes() const
{
    return m_kind == PHost
        && (m_version >= MKVERSION(4,0,11)             // 4.0k
            || (m_version < MKVERSION(4,0,0)
                && m_version >= MKVERSION(3,4,13)));   // 3.4m
}

// Check for valid chunnel distance.
bool
game::HostVersion::isValidChunnelDistance2(int32_t dist2, const game::config::HostConfiguration& config) const
{
    if (m_kind == PHost) {
        // PHost comparison is "Trunc(Sqrt(dist2)) >= MCD". i.e. "Sqrt(dist2) >= MCD".
        return dist2 >= util::squareInteger(config[config.MinimumChunnelDistance]());
    } else {
        // Host comparison is "ERND(Sqrt(dist2)) >= 100", i.e. "Sqrt(dist2) >= 99.5"
        // Host bug: up to 3.22.25, host forgot the Sqrt() and compares "dist2 >= 100", making the limit 10 ly; however, it has always been documented as 100 ly.
        return dist2 >= 9901;
    }
}

// Set configuration options implied by this host version.
void
game::HostVersion::setImpliedHostConfiguration(game::config::HostConfiguration& config)
{
    switch (m_kind) {
     case Unknown:
     case Host:
     case SRace:
     case NuHost:
        // pconfig.pas:GetMaxDefenseOnBase
        config[HostConfiguration::MaximumDefenseOnBase].set(200);

        // pconfig.pas:GetMaxFightersOnBase
        config[HostConfiguration::MaximumFightersOnBase].set(60);

        // pconfig.pas:GetShipFighterCost
        config[HostConfiguration::ShipFighterCost].set("T3 M2 S5");

        // pconfig.pas:GetFighterCost
        config[HostConfiguration::BaseFighterCost].set("T3 M2 $100");

        // pconfig.pas:GetBaseCost
        config[HostConfiguration::StarbaseCost].set("T402 D120 M340 $900");

        // pconfig.pas:IsShowCommandAvailable
        config[HostConfiguration::CPEnableShow].set(false);

        // WPreferencesDialog::init
        config[HostConfiguration::CPEnableLanguage].set(false);
        config[HostConfiguration::CPEnableRaceName].set(false);
        config[HostConfiguration::CPEnableRemote].set(false);
        config[HostConfiguration::CPEnableSend].set(false);
        config[HostConfiguration::DisablePasswords].set(false);

        // ex WCloneCargoCostTransaction::update()
        config[HostConfiguration::ShipCloneCostRate].set(200);

        // computeTurn, pdata.pas:ComputePlanetTurn, game/planetform.h:getBovinoidSupplyContribution, game/planetform.h:getBovinoidSupplyContributionLimited, client/tiles/planetgrowth.cc:getHissEffect, client/dlg-tax.cc:getHissEffect
        config[HostConfiguration::ProductionRate].set(100);
        config[HostConfiguration::MaxShipsHissing].set(MAX_NUMBER);
        config[HostConfiguration::TerraformRate].set(1);

        // WTorpInfo::drawContent
        for (int i = 1; i <= MAX_PLAYERS; ++i) {
            int rate = config[HostConfiguration::PlayerSpecialMission](i) == 9 ? 400 : 100;
            config[HostConfiguration::UnitsPerTorpRate].set(i, rate);
            config[HostConfiguration::UnitsPerWebRate].set(i, rate);
        }

        // shipacc.pas:CheckChunnelFailures - in Host, ships can exist with >100 damage, so PHost's default 100 is not sufficient
        config[HostConfiguration::DamageLevelForChunnelFail].set(151);

        // getSensorVisibility
        config[HostConfiguration::DefenseForUndetectable].set(15);
        config[HostConfiguration::MinesForDetectable].set(21);
        config[HostConfiguration::FactoriesForDetectable].set(16);

        // Locker::findWarpWellEdge
        config[HostConfiguration::AllowHyperjumpGravWells].set(1);

        // Tim-Host defaults; ex game/config.cc:initConfig
        config[HostConfiguration::RoundGravityWells].set(1);
        config[HostConfiguration::CPEnableRemote].set(0);
        config[HostConfiguration::MapTruehullByPlayerRace].set(0);

        // No extended missions
        config[HostConfiguration::AllowExtendedMissions].set(0);

        // No per-turn fuel usage
        config[HostConfiguration::FuelUsagePerFightFor100KT].set(0);
        config[HostConfiguration::FuelUsagePerTurnFor100KT].set(0);

        // PBP queue
        // ex GHost::isPBPGame, pconfig.pas:IsPBPGame
        config[HostConfiguration::BuildQueue].set("PBP");

        // Intentionally not handled here:
        // - AllowAlternativeCombat (could be THost with FLAK)
        break;

     case PHost:
        // pconfig.pas:IsShowCommandAvailable
        if (m_version < MKVERSION(4,0,8)) {
            config[HostConfiguration::CPEnableShow].set(false);
        }
        break;
    }
}
