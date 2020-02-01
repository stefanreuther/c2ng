/**
  *  \file game/hostversion.cpp
  *  \brief Class game::HostVersion (ex GHost)
  */

#include "game/hostversion.hpp"
#include "afl/string/format.hpp"
#include "game/limits.hpp"
#include "util/math.hpp"

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
}

// Default constructor.
game::HostVersion::HostVersion()
    : m_kind(Unknown),
      m_version(0)
{ }

// Constructor.
game::HostVersion::HostVersion(Kind kind, int32_t version)
    : m_kind(kind),
      m_version(version)
{ }

// Set specific host version.
void
game::HostVersion::set(Kind kind, int32_t version)
{
    m_kind = kind;
    m_version = version;
}

// Get host type.
game::HostVersion::Kind
game::HostVersion::getKind() const
{
    return m_kind;
}

// Get host version.
int32_t
game::HostVersion::getVersion() const
{
    return m_version;
}

// Check for PHost.
bool
game::HostVersion::isPHost() const
{
    return m_kind == PHost;
}


// Format as string.
String_t
game::HostVersion::toString(afl::string::Translator& tx) const
{
    switch (m_kind) {
     case Unknown:
        return tx("unknown");

     case Host:
        return formatVersion(tx("Host"), m_version, true);

     case SRace:
        return formatVersion(tx("SRace"), m_version, true);

     case PHost:
        return formatVersion(tx("PHost"), m_version, false);

     case NuHost:
        return formatVersion(tx("NuHost"), m_version, true);
    }
    return String_t();
}

// Get ship command argument limit.
int32_t
game::HostVersion::getCommandArgumentLimit() const
{
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

// Get post-taxation happiness limit.
int
game::HostVersion::getPostTaxationHappinessLimit() const
{
    return m_kind == PHost ? 30 : 31;
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

// Check whether host has case-insensitive universal minefield friendly codes.
bool
game::HostVersion::hasCaseInsensitiveUniversalMinefieldFCodes() const
{
    return m_kind != PHost;
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
        int32_t adjust = m_kind == PHost ? 0 : 1;
        return distSquared-adjust >= 340L*340
            && distSquared+adjust <= 360L*360;
    }
}

// Check mission.
bool
game::HostVersion::isMissionAllowed(int mission) const
{
    // ex GHost::isMissionAllowed
    // SRace cannot have mission 1
    // FIXME: NuHost also has some limits here
    if (mission == 1 && m_kind == SRace) {
        return false;
    } else {
        return true;
    }
}

// Check for Minefield-Center bug.
bool
game::HostVersion::hasMinefieldCenterBug() const
{
    return m_kind != PHost;
}

// Check whether mine laying is before or after decay.
bool
game::HostVersion::isMineLayingAfterMineDecay() const
{
    return m_kind == PHost;
}

// Check whether mine decay uses rounding.
bool
game::HostVersion::isRoundingMineDecay() const
{
    return m_kind != PHost;
}

// Check whether the build system of this host has PBP style.
bool
game::HostVersion::isPBPGame(const game::config::HostConfiguration& config) const
{
    // ex GHost::isPBPGame
    // ex pconfig.pas:IsPBPGame
    return m_kind != PHost
        || afl::string::strCaseCompare(config[config.BuildQueue]().substr(0, 3), "pbp") == 0;
}

// Check whether this is a game where ships burn fuel each turn for just being there.
bool
game::HostVersion::isEugeneGame(const game::config::HostConfiguration& config) const
{
    // ex pconfig.pas:IsEugeneGame
    return m_kind == PHost
        && (config.getPlayersWhereEnabled(config.FuelUsagePerFightFor100KT).nonempty()
            || config.getPlayersWhereEnabled(config.FuelUsagePerTurnFor100KT).nonempty());
}

// Check for doubled effective torpedo power.
bool
game::HostVersion::hasDoubleTorpedoPower(const game::config::HostConfiguration& config) const
{
    return !(m_kind == PHost && config[config.AllowAlternativeCombat]());
}

// Check for ability to do two cargo transfers from a ship.
bool
game::HostVersion::hasParallelShipTransfers() const
{
    return m_kind != NuHost;
}

// Check for extended missions.
bool
game::HostVersion::hasExtendedMissions(const game::config::HostConfiguration& config) const
{
    return m_kind == PHost
        && config[config.AllowExtendedMissions]() != 0;
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

// Check for alchemy exclusion friendly codes ("naX"). */
bool
game::HostVersion::hasAlchemyExclusionFCodes() const
{
    return m_kind == PHost;
}

// Check for rounding in "alX" alchemy.
bool
game::HostVersion::isAlchemyRounding() const
{
    // Don't know about NuHost, but let's assume they don't emulate this.
    return m_kind == Host
        || m_kind == SRace;
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

// Get minimum fuel to initiate a chunnel.
int
game::HostVersion::getMinimumFuelToInitiateChunnel() const
{
    if (m_kind == PHost) {
        return 51;
    } else {
        return 50;
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
        break;

     case PHost:
        // pconfig.pas:IsShowCommandAvailable
        if (m_version < MKVERSION(4,0,8)) {
            config[HostConfiguration::CPEnableShow].set(false);
        }
        break;
    }
}
