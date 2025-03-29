/**
  *  \file game/hostversion.hpp
  *  \brief Class game::HostVersion
  */
#ifndef C2NG_GAME_HOSTVERSION_HPP
#define C2NG_GAME_HOSTVERSION_HPP

#include "game/config/hostconfiguration.hpp"

/** \def MKVERSION
    Make version code from three numbers.
    For example, MKVERSION(3,22,31) yiels 322031, the version code for THost 3.22.031.
    If the final component is a letter, use its position in the alphabet, e.g. MKVERSION(3,2,5) for 3.2e. */
#define MKVERSION(x,y,z) ((x)*100000+(y)*1000+(z))

namespace game {

    /** Knowledge about Host Properties.
        Essentially, we distinguish between families of hosts (Tim's host, PHost, etc.).
        In addition, there are version dependencies
        (like: Tim's Host in version 3.22.031+ does not allow you to place high-tech torps on a low-tech base;
        or, PHost introduced death rays in version 4.0.).
        This module should contain all such host version dependencies.

        As a rule, if the version dependency is "in X, the rule is this, and in Y, the rule is that",
        we provide just a function to distinguish the X and Y cases.
        In some cases, we also provide a more detailed function, like "what is the limit for Z in this host version?". */
    class HostVersion {
     public:
        /** Host type. */
        enum Kind {
            Unknown,            ///< Not yet determined (must be first).
            Host,               ///< Tim-Host, any version.
            SRace,              ///< SRace, any version.
            PHost,              ///< PHost, any version.
            NuHost              ///< NuHost, any version.
        };

        /** Default constructor.
            Makes an "Unknown" host version. */
        HostVersion();

        /** Constructor.
            Makes a specific host version.
            \param kind Host type
            \param version Host version, @see MKVERSION. */
        HostVersion(Kind kind, int32_t version);

        /** Set specific host version.
            \param kind Host type
            \param version Host version, @see MKVERSION. */
        void set(Kind kind, int32_t version);

        /** Get host type.
            \return host type */
        Kind getKind() const;

        /** Get host version.
            \return host version, @see MKVERSION. */
        int32_t getVersion() const;

        /** Check for PHost.
            \return true if host is PHost */
        bool isPHost() const;

        /** Format as string.
            \return host version formatted as a string */
        String_t toString() const;

        /** Parse from strings.
            If the given pair represents a recognized host version,
            set this object's content to it.
            \param hostType     Host type
            \param hostVersion  Version
            \return true if version recognized and set; false if not recognized and object unchanged */
        bool fromString(String_t hostType, String_t hostVersion);

        /** Parse from single string.
            If the given string represents a recognized host version,
            set this object's content to it.
            This is an inverse to toString().
            \param str String consisting of type and version
            \return true if version recognized and set; false if not recognized and object unchanged */
        bool fromString(String_t str);

        /** Get ship command argument limit.
            Many versions have this limited to 500, PHost 3.3b raised it to 10000. */
        int32_t getCommandArgumentLimit() const;

        /** Check whether this host version has Death Rays. */
        bool hasDeathRays() const;

        /** Check whether this host has experience levels. */
        bool hasExperienceLevels() const;

        /** Check whether this host has ship-specific hull functions. */
        bool hasShipSpecificFunctions() const;

        /** Check whether hullfunc.txt assignments are cumulative in this host version.
            This applies to PHost 4.0i/3.4k and later. */
        bool hasCumulativeHullfunc() const;

        /** Check whether ImperialAssault implies PlanetImmunity ability.
            This applies to all hosts except for PHost 4.0i and higher,
            where PlanetImmunity is a separate ability. */
        bool hasImmuneAssaultShip() const;

        /** Check whether this host has restrictions in loading high-tech torps onto low-tech bases.
            This is a bug introduced in 3.22.031, and - given the time Tim spends on Host 3.x - is expected to stay for a while.
            These new hosts do not allow you to unload torpedoes onto a starbase if the base doesn't have enough tech to build these torps. */
        bool hasHighTechTorpedoBug() const;

        /** Check whether siliconoid natives have desert advantage in this host.
            This applies to Tim-Host, as well as PHost 3.3c+. */
        bool hasSiliconoidDesertAdvantage() const;

        /** Check whether this host allows large cargo transfers.
            A large cargo transfer is one where the cargo amount in transit plus the ship content exceed the cargo room size.
            This is allowed by most Host versions, but was blocked/made configurable in THost 3.22.31. */
        bool hasLargeCargoTransfer() const;

        /** Check whether the "Lay mines in" mission automatically fills in the minefield owner.
            When enlarging a foreing minefield, PHost versions before 3.4c require you to set an 'miX' friendly code, newer do not. */
        bool hasAutomaticMineIdentity() const;

        /** Get post-taxation happiness limit.
            This is the highest level of happiness after taxation happiness effect at which you'll still collect taxes */
        int getPostTaxationHappinessLimit() const;

        /** Check whether host allows negative numeric friendly codes.
            This applies to PHost 2.9 and later. */
        bool hasNegativeFCodes() const;

        /** Check whether host allows space-padding in numeric friendly codes.
            This applies to PHost 4.0h or 3.4j and later. */
        bool hasSpacePaddedFCodes() const;

        /** Check whether host has case-insensitive universal minefield friendly codes.
            (Note that this means determination of the universal minefield friendly code is caseblind,
            not comparison of the code with a ship's code.).
            This applies to THost. */
        bool hasCaseInsensitiveUniversalMinefieldFCodes() const;

        /** Get the maximum tax for this race.
            In THost, Borg cannot tax natives above 20, Lizards should not tax natives above 75.
            Note that the 75% rule is not enforced by current THost versions due to a bug
            (it computes native/colonist deaths, but forgets to write them back to the data files). */
        int getNativeTaxRateLimit(int player, const game::config::HostConfiguration& config) const;

        /** Get the maximum tax for this race.
            In THost, Lizards should not tax colonists above 75.
            Note that the 75% rule is not enforced by current THost versions due to a bug
            (it computes native/colonist deaths, but forgets to write them back to the data files). */
        int getColonistTaxRateLimit(int player, const game::config::HostConfiguration& config) const;

        /** Check whether PHost rounds in mining formulas.
            This was changed in PHost 4.1/3.5 to be more player-friendly.
            Note that this function only applies to PHost.
            THost has different formulas. */
        bool isPHostRoundingMiningResults() const;

        /** Check for exact hyperjump distance.
            340/360 is inclusive in PHost, but not in THost.
            Note that this is actually version-dependant, too (old host versions never jump exactly).
            \param distSquared Squared distance to check */
        bool isExactHyperjumpDistance2(int32_t distSquared) const;

        /** Get minimum hyperjump distance, squared.
            For a hyperjump to be recognized, the waypoint needs to be >= this distance.
            \return squared distance */
        int32_t getMinimumHyperjumpDistance2() const;

        /** Check whether host automatically resets friendly codes after a hyperjump.
            \return status */
        bool hasAutomaticHyperjumpReset() const;

        /** Check mission.
            This allows the host implementation to filter out missions. */
        bool isMissionAllowed(int mission) const;

        /** Check for Minefield-Center bug.
            If this bug is present, we look for the closest minefield center when laying mines,
            regardless whether we actually are in that field.
            This bug is in all THost versions so far. */
        bool hasMinefieldCenterBug() const;

        /** Check whether mine laying is before or after decay. */
        bool isMineLayingAfterMineDecay() const;

        /** Check whether mine decay uses rounding. */
        bool isRoundingMineDecay() const;

        /** Check whether mine scooping needs beams. */
        bool isBeamRequiredForMineScooping() const;

        /** Check for ability to do two cargo transfers from a ship.
            Classic VGAP can do that, Nu has only one slot to store the transfer. */
        bool hasParallelShipTransfers() const;

        /** Check for bug in UseAccurateFuelModel computation.
            For PHost with UseAccurateFuelModel enabled, a ship can burn, say, 3.2 kt fuel.
            This means if it starts the move with 4 kt, it will have 1 kt remaining,
            but if it starts the move with 3 kt, it'll run dry. */
        bool hasAccurateFuelModelBug() const;

        /** Check for alchemy combination function support.
            In PHost 4.0i/3.4k, combinations of alchemy functions produce sensible "best of" functionality,
            e.g. Refinery + AdvancedRefinery = AdvancedRefinery.
            Previously, combinations were not very well thought-out, making the above
            combination produce Refinery only. */
        bool hasAlchemyCombinations() const;

        /** Check for refinery friendly code support.
            If true, the "alX" and "naX" friendly codes affect refinery ships as well. */
        bool hasRefineryFCodes() const;

        /** Check for alchemy exclusion friendly codes ("naX"). */
        bool hasAlchemyExclusionFCodes() const;

        /** Check for rounding in "alX" alchemy.
            It was originally believed that even with "alX" friendly code, THost consumes supplies in steps of 9,
            but that turns out to not be true. */
        bool isAlchemyRounding() const;

        /** Check for valid chunnel distance.
            Host and PHost implement rules different here.
            \param dist2 Distance, squared
            \param config Host configuration
            \return true if a chunnel with this distance will work */
        bool isValidChunnelDistance2(int32_t dist2, const game::config::HostConfiguration& config) const;

        /** Get minimum fuel to initiate a chunnel.
            Whereas PHost requires the ship to have remaining fuel after burning 50 kt,
            Host accepts ships that run dry. */
        int getMinimumFuelToInitiateChunnel() const;

        /** Check for premissive climate population limits.
            If true, HOST's permissive formula ("won't die if less than") applies. */
        bool hasPermissiveClimateLimits() const;

        /** Set configuration options implied by this host version.
            This way, code can just use the configuration
            instead of implementing a "should I query hconfig or use fixed value" switch.
            \param [out] config Configuration to update */
        void setImpliedHostConfiguration(game::config::HostConfiguration& config);

     private:
        /** Host type. */
        Kind m_kind;

        /** Host version.
            Encoded in the same way as it's visible to scripts: 322040 = 3.22.40. @see MKVERSION(). */
        int32_t m_version;
    };
}


// Default constructor.
inline
game::HostVersion::HostVersion()
    : m_kind(Unknown),
      m_version(0)
{ }

// Constructor.
inline
game::HostVersion::HostVersion(Kind kind, int32_t version)
    : m_kind(kind),
      m_version(version)
{ }

// Set specific host version.
inline void
game::HostVersion::set(Kind kind, int32_t version)
{
    m_kind = kind;
    m_version = version;
}

// Get host type.
inline game::HostVersion::Kind
game::HostVersion::getKind() const
{
    return m_kind;
}

// Get host version.
inline int32_t
game::HostVersion::getVersion() const
{
    return m_version;
}

// Check for PHost.
inline bool
game::HostVersion::isPHost() const
{
    return m_kind == PHost;
}

// Get post-taxation happiness limit.
inline int
game::HostVersion::getPostTaxationHappinessLimit() const
{
    // ex pdata.pas:TaxLimit
    return m_kind == PHost ? 30 : 31;
}

// Check whether host has case-insensitive universal minefield friendly codes.
inline bool
game::HostVersion::hasCaseInsensitiveUniversalMinefieldFCodes() const
{
    return m_kind != PHost;
}

// Get minimum hyperjump distance, squared.
inline int32_t
game::HostVersion::getMinimumHyperjumpDistance2() const
{
    if (m_kind == PHost) {
        // PHost has no minimum distance other than the waypoint needs to be nonzero
        return 1;
    } else {
        // Host requires >20
        return 20*20+1;
    }
}

// Check whether host automatically resets friendly codes after a hyperjump.
inline bool
game::HostVersion::hasAutomaticHyperjumpReset() const
{
    // PHost automatically resets friendly codes, Tim-Host does not. NuHost is unknown.
    return (m_kind == PHost);
}

// Check mission.
inline bool
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
inline bool
game::HostVersion::hasMinefieldCenterBug() const
{
    return m_kind != PHost;
}

// Check whether mine laying is before or after decay.
inline bool
game::HostVersion::isMineLayingAfterMineDecay() const
{
    return m_kind == PHost;
}

// Check whether mine decay uses rounding.
inline bool
game::HostVersion::isRoundingMineDecay() const
{
    return m_kind != PHost;
}

// Check whether beams are required for mine scooping.
inline bool
game::HostVersion::isBeamRequiredForMineScooping() const
{
    return m_kind == PHost;
}

// Check for ability to do two cargo transfers from a ship.
inline bool
game::HostVersion::hasParallelShipTransfers() const
{
    return m_kind != NuHost;
}

// Check for alchemy exclusion friendly codes ("naX"). */
inline bool
game::HostVersion::hasAlchemyExclusionFCodes() const
{
    return m_kind == PHost;
}

// Check for rounding in "alX" alchemy.
inline bool
game::HostVersion::isAlchemyRounding() const
{
    // It was originally believed that THost converts with 9->3 granularity, not 3->1, with "alX" friendly codes.
    // It turns out that no host version that I can find actually does that.
    return false;
}

// Get minimum fuel to initiate a chunnel.
inline int
game::HostVersion::getMinimumFuelToInitiateChunnel() const
{
    if (m_kind == PHost) {
        return 51;
    } else {
        return 50;
    }
}

// Check for premissive climate population limits.
inline bool
game::HostVersion::hasPermissiveClimateLimits() const
{
    return m_kind == Host || m_kind == SRace;
}

#endif
