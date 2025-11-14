/**
  *  \file test/game/hostversiontest.cpp
  *  \brief Test for game::HostVersion
  */

#include "game/hostversion.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "game/config/hostconfiguration.hpp"

using afl::base::Ref;
using game::HostVersion;
using game::config::HostConfiguration;

/** Test formatting. */
AFL_TEST("game.HostVersion:toString", a)
{
    // Unknown
    a.checkEqual("01", HostVersion().toString(), "unknown");
    a.checkEqual("02", HostVersion(HostVersion::Unknown, MKVERSION(3,0,0)).toString(), "unknown");

    // Tim-Host
    a.checkEqual("11", HostVersion(HostVersion::Host, 0).toString(), "Host");
    a.checkEqual("12", HostVersion(HostVersion::Host, MKVERSION(3,0,0)).toString(), "Host 3.0");
    a.checkEqual("13", HostVersion(HostVersion::Host, MKVERSION(3,16,1)).toString(), "Host 3.16.001");
    a.checkEqual("14", HostVersion(HostVersion::Host, MKVERSION(3,20,0)).toString(), "Host 3.20");
    a.checkEqual("15", HostVersion(HostVersion::Host, MKVERSION(3,22,27)).toString(), "Host 3.22.027");

    // PHost
    a.checkEqual("21", HostVersion(HostVersion::PHost, 0).toString(), "PHost");
    a.checkEqual("22", HostVersion(HostVersion::PHost, MKVERSION(3,0,0)).toString(), "PHost 3.0");
    a.checkEqual("23", HostVersion(HostVersion::PHost, MKVERSION(3,16,1)).toString(), "PHost 3.16a");
    a.checkEqual("24", HostVersion(HostVersion::PHost, MKVERSION(3,20,0)).toString(), "PHost 3.20");
    a.checkEqual("25", HostVersion(HostVersion::PHost, MKVERSION(3,4,5)).toString(), "PHost 3.4e");
    a.checkEqual("26", HostVersion(HostVersion::PHost, MKVERSION(3,22,27)).toString(), "PHost 3.22.027");
    a.checkEqual("27", HostVersion(HostVersion::PHost, MKVERSION(3,4,13)).toString(), "PHost 3.4m");

    // SRace (Tim-Host variant)
    a.checkEqual("31", HostVersion(HostVersion::SRace, 0).toString(), "SRace");
    a.checkEqual("32", HostVersion(HostVersion::SRace, MKVERSION(3,0,0)).toString(), "SRace 3.0");
    a.checkEqual("33", HostVersion(HostVersion::SRace, MKVERSION(3,16,1)).toString(), "SRace 3.16.001");

    // NuHost
    a.checkEqual("41", HostVersion(HostVersion::NuHost, 0).toString(), "NuHost");
    a.checkEqual("42", HostVersion(HostVersion::NuHost, MKVERSION(3,0,0)).toString(), "NuHost 3.0");
    a.checkEqual("43", HostVersion(HostVersion::NuHost, MKVERSION(3,16,1)).toString(), "NuHost 3.16.001");
}

/** Test accessors. */
AFL_TEST("game.HostVersion:accessor", a)
{
    HostVersion t;
    a.checkEqual("01. getKind", t.getKind(), HostVersion::Unknown);
    a.checkEqual("02. getVersion", t.getVersion(), 0);

    t.set(HostVersion::PHost, MKVERSION(4,1,0));
    a.checkEqual("11. getKind", t.getKind(), HostVersion::PHost);
    a.checkEqual("12. getVersion", t.getVersion(), MKVERSION(4,1,0));

    a.checkEqual("21. getKind", HostVersion(HostVersion::Host, MKVERSION(3,22,0)).getKind(), HostVersion::Host);
}

/** Test MKVERSION.
    These values are given to scripts and therefore should be verified against known values. */
AFL_TEST("game.HostVersion:MKVERSION", a)
{
    a.checkEqual("01", MKVERSION(0,0,0),   0);
    a.checkEqual("02", MKVERSION(3,22,46), 322046);
    a.checkEqual("03", MKVERSION(4,1,5),   401005);
}

/*
 *  Test host properties.
 */

AFL_TEST("game.HostVersion:getCommandArgumentLimit", a)
{
    // getCommandArgumentLimit
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).getCommandArgumentLimit(), 500);
    a.checkEqual("05", HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).getCommandArgumentLimit(), 10000);
    a.checkEqual("06", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getCommandArgumentLimit(), 10000);
    a.checkEqual("07", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getCommandArgumentLimit(), 999);
}

// hasDeathRays: PHost 4.0+
AFL_TEST("game.HostVersion:hasDeathRays", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasDeathRays());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasDeathRays());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasDeathRays());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasDeathRays());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasDeathRays());
    a.check("06", !HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasDeathRays());
}

// hasExperienceLevels: PHost 4.0+
AFL_TEST("game.HostVersion:hasExperienceLevels", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasExperienceLevels());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasExperienceLevels());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasExperienceLevels());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasExperienceLevels());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasExperienceLevels());
    a.check("06", !HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasExperienceLevels());
}

// hasShipSpecificFunctions: PHost 4.0+
AFL_TEST("game.HostVersion:hasShipSpecificFunctions", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasShipSpecificFunctions());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasShipSpecificFunctions());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasShipSpecificFunctions());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasShipSpecificFunctions());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasShipSpecificFunctions());
    a.check("06", !HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasShipSpecificFunctions());
}

// hasCumulativeHullfunc: PHost 4.0i+, 3.4k+
AFL_TEST("game.HostVersion:hasCumulativeHullfunc", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasCumulativeHullfunc());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasCumulativeHullfunc());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasCumulativeHullfunc());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3,4,10)).hasCumulativeHullfunc());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(3,4,11)).hasCumulativeHullfunc());
    a.check("06", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasCumulativeHullfunc());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasCumulativeHullfunc());
    a.check("08", !HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasCumulativeHullfunc());
}

// hasImmuneAssaultShip: all but PHost 4.0i+
AFL_TEST("game.HostVersion:hasImmuneAssaultShip", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasImmuneAssaultShip());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasImmuneAssaultShip());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasImmuneAssaultShip());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasImmuneAssaultShip());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasImmuneAssaultShip());
    a.check("06", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasImmuneAssaultShip());
    a.check("07",  HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasImmuneAssaultShip());
}

// hasHighTechTorpedoBug: Host 3.22.31+
AFL_TEST("game.HostVersion:hasHighTechTorpedoBug", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,30)).hasHighTechTorpedoBug());
    a.check("03",  HostVersion(HostVersion::Host,    MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    a.check("04",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,31)).hasHighTechTorpedoBug());
    a.check("05", !HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasHighTechTorpedoBug());
    a.check("06", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasHighTechTorpedoBug());
    a.check("07", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,9)).hasHighTechTorpedoBug());
    a.check("08",  HostVersion(HostVersion::NuHost,  MKVERSION(3,22,31)).hasHighTechTorpedoBug());
}

// hasSiliconoidDesertAdvantage: Tim and PHost 3.3+
AFL_TEST("game.HostVersion:hasSiliconoidDesertAdvantage", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasSiliconoidDesertAdvantage());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(3, 3,3)).hasSiliconoidDesertAdvantage());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasSiliconoidDesertAdvantage());
    a.check("07",  HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).hasSiliconoidDesertAdvantage());
}

// hasLargeCargoTransfer: PHost and Tim up to 3.22.30
AFL_TEST("game.HostVersion:hasLargeCargoTransfer", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasLargeCargoTransfer());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasLargeCargoTransfer());
    a.check("03",  HostVersion(HostVersion::Host,    MKVERSION(3,22,30)).hasLargeCargoTransfer());
    a.check("04", !HostVersion(HostVersion::Host,    MKVERSION(3,22,31)).hasLargeCargoTransfer());
    a.check("05",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasLargeCargoTransfer());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).hasLargeCargoTransfer());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasLargeCargoTransfer());
    a.check("08",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasLargeCargoTransfer());
}

// hasAutomaticMineIdentity: PHost 3.4c and newer
AFL_TEST("game.HostVersion:hasAutomaticMineIdentity", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    a.check("03", !HostVersion(HostVersion::Host,    MKVERSION(3,22,29)).hasAutomaticMineIdentity());
    a.check("04", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAutomaticMineIdentity());
    a.check("05", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,2)).hasAutomaticMineIdentity());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,3)).hasAutomaticMineIdentity());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasAutomaticMineIdentity());
    a.check("08", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAutomaticMineIdentity());
}

// getPostTaxationHappinessLimit
AFL_TEST("game.HostVersion:getPostTaxationHappinessLimit", a)
{
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).getPostTaxationHappinessLimit(), 30);
    a.checkEqual("05", HostVersion(HostVersion::PHost,   MKVERSION(3, 3,2)).getPostTaxationHappinessLimit(), 30);
    a.checkEqual("06", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getPostTaxationHappinessLimit(), 30);
    a.checkEqual("07", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getPostTaxationHappinessLimit(), 31);
}

// hasNegativeFCodes:
AFL_TEST("game.HostVersion:hasNegativeFCodes", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasNegativeFCodes());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasNegativeFCodes());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasNegativeFCodes());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(2, 8,9)).hasNegativeFCodes());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(2, 9,0)).hasNegativeFCodes());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasNegativeFCodes());
    a.check("07", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasNegativeFCodes());
}

// hasSpacePaddedFCodes: PHost 4.0h+, 3.4j+
AFL_TEST("game.HostVersion:hasSpacePaddedFCodes", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasSpacePaddedFCodes());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,9)).hasSpacePaddedFCodes());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(3,4,10)).hasSpacePaddedFCodes());
    a.check("06", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,7)).hasSpacePaddedFCodes());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,8)).hasSpacePaddedFCodes());
    a.check("08", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasSpacePaddedFCodes());
}

// hasCaseInsensitiveUniversalMinefieldFCodes:
AFL_TEST("game.HostVersion:hasCaseInsensitiveUniversalMinefieldFCodes", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 2,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    a.check("05", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
    a.check("06",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasCaseInsensitiveUniversalMinefieldFCodes());
}

// getNativeTaxRateLimit
AFL_TEST("game.HostVersion:getNativeTaxRateLimit", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.PlayerRace].set("1,2,3,4,5,6,7,8,9,10,11");
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(1, config), 100);
    a.checkEqual("05", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 100);

    a.checkEqual("11", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    a.checkEqual("12", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    a.checkEqual("13", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);
    a.checkEqual("14", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(2, config), 100);
    a.checkEqual("15", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 75);

    a.checkEqual("21", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    a.checkEqual("22", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    a.checkEqual("23", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
    a.checkEqual("24", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getNativeTaxRateLimit(6, config), 100);
    a.checkEqual("25", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getNativeTaxRateLimit(6, config), 20);
}

AFL_TEST("game.HostVersion:getNativeTaxRateLimit:PlayerRace", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.PlayerRace].set("6,1,2,1");
    a.checkEqual("01", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getNativeTaxRateLimit(1, config), 20);
    a.checkEqual("02", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(2, config), 100);
    a.checkEqual("03", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getNativeTaxRateLimit(3, config), 75);
}

// getColonistTaxRateLimit:
AFL_TEST("game.HostVersion:getColonistTaxRateLimit", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.PlayerRace].set("1,2,3,4,5,6,7,8,9,10,11");
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getColonistTaxRateLimit(1, config), 100);
    a.checkEqual("05", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);

    a.checkEqual("11", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    a.checkEqual("12", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    a.checkEqual("13", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
    a.checkEqual("14", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getColonistTaxRateLimit(2, config), 100);
    a.checkEqual("15", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 75);
}

AFL_TEST("game.HostVersion:getColonistTaxRateLimit:PlayerRace", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.PlayerRace].set("6,1,2,1");
    a.checkEqual("01", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getColonistTaxRateLimit(1, config), 100);
    a.checkEqual("02", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(2, config), 100);
    a.checkEqual("03", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getColonistTaxRateLimit(3, config), 75);
}

// isPHostRoundingMiningResults: PHost 4.1/3.5; does not apply to other-than-PHost.
AFL_TEST("game.HostVersion:isPHostRoundingMiningResults", a)
{
    a.check("01", !HostVersion(HostVersion::PHost,   MKVERSION(3,4,99)).isPHostRoundingMiningResults());
    a.check("02",  HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).isPHostRoundingMiningResults());
    a.check("03", !HostVersion(HostVersion::PHost,   MKVERSION(4,0,99)).isPHostRoundingMiningResults());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(4, 1,0)).isPHostRoundingMiningResults());
}

// isExactHyperjumpDistance2: 340/360 is inclusive in PHost, but not in THost.
AFL_TEST("game.HostVersion:isExactHyperjumpDistance2", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    a.check("03", !HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(340*340));
    a.check("04", !HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));
    a.check("06", !HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340));

    a.check("11", !HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    a.check("12", !HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    a.check("13",  HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(340*340+1));
    a.check("14", !HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    a.check("15",  HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));
    a.check("16", !HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(340*340+1));

    a.check("21", !HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    a.check("22", !HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    a.check("23", !HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(360*360));
    a.check("24", !HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    a.check("25",  HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));
    a.check("26", !HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360));

    a.check("31", !HostVersion(HostVersion::Unknown, MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    a.check("32", !HostVersion(HostVersion::Host,    MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    a.check("33",  HostVersion(HostVersion::Host,    MKVERSION(3,20,0)).isExactHyperjumpDistance2(360*360-1));
    a.check("34", !HostVersion(HostVersion::SRace,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    a.check("35",  HostVersion(HostVersion::PHost,   MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
    a.check("36", !HostVersion(HostVersion::NuHost,  MKVERSION(3,15,0)).isExactHyperjumpDistance2(360*360-1));
}

// getMinimumHyperjumpDistance2
AFL_TEST("game.HostVersion:getMinimumHyperjumpDistance2", a)
{
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).getMinimumHyperjumpDistance2(), 1);
    a.checkEqual("05", HostVersion(HostVersion::NuHost,  MKVERSION(3,22,0)).getMinimumHyperjumpDistance2(), 401);
}

// isMissionAllowed: SRace cannot have mission 1
AFL_TEST("game.HostVersion:isMissionAllowed", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMissionAllowed(1));
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMissionAllowed(1));
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMissionAllowed(1));
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMissionAllowed(1));
    a.check("05",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMissionAllowed(1));

    a.check("11",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMissionAllowed(2));
    a.check("12",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMissionAllowed(2));
    a.check("13",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMissionAllowed(2));
    a.check("14",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMissionAllowed(2));
    a.check("15",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMissionAllowed(2));
}

// hasMinefieldCenterBug: all TimHost
AFL_TEST("game.HostVersion:hasMinefieldCenterBug", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasMinefieldCenterBug());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasMinefieldCenterBug());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasMinefieldCenterBug());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasMinefieldCenterBug());
    a.check("05",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasMinefieldCenterBug());
}

// isMineLayingAfterMineDecay: all PHost
AFL_TEST("game.HostVersion:isMineLayingAfterMineDecay", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isMineLayingAfterMineDecay());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isMineLayingAfterMineDecay());
    a.check("05", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isMineLayingAfterMineDecay());
}

// isRoundingMineDecay: all TimHost
AFL_TEST("game.HostVersion:isRoundingMineDecay", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isRoundingMineDecay());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isRoundingMineDecay());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isRoundingMineDecay());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isRoundingMineDecay());
    a.check("05",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isRoundingMineDecay());
}

// isBeamRequiredForMineScooping: all PHost
AFL_TEST("game.HostVersion:isBeamRequiredForMineScooping", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isBeamRequiredForMineScooping());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isBeamRequiredForMineScooping());
    a.check("05", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isBeamRequiredForMineScooping());
}

// hasParallelShipTransfers: all but NuHost
AFL_TEST("game.HostVersion:hasParallelShipTransfers", a)
{
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasParallelShipTransfers());
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasParallelShipTransfers());
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasParallelShipTransfers());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasParallelShipTransfers());
    a.check("05", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasParallelShipTransfers());
}

// hasAccurateFuelModelBug: PHost <3.4h, 4.0e
AFL_TEST("game.HostVersion:hasAccurateFuelModelBug", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAccurateFuelModelBug());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasAccurateFuelModelBug());
    a.check("05",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,0)).hasAccurateFuelModelBug());
    a.check("06", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,8)).hasAccurateFuelModelBug());
    a.check("07", !HostVersion(HostVersion::PHost,   MKVERSION(3, 5,0)).hasAccurateFuelModelBug());
    a.check("08", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0,5)).hasAccurateFuelModelBug());
    a.check("09", !HostVersion(HostVersion::PHost,   MKVERSION(4, 1,0)).hasAccurateFuelModelBug());
    a.check("10", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAccurateFuelModelBug());
}

// hasAlchemyCombinations: PHost >= 4.0i, 3.4k
AFL_TEST("game.HostVersion:hasAlchemyCombinations", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22, 0)).hasAlchemyCombinations());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22, 0)).hasAlchemyCombinations());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22, 0)).hasAlchemyCombinations());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4, 0)).hasAlchemyCombinations());
    a.check("05", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 0)).hasAlchemyCombinations());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,11)).hasAlchemyCombinations());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(3, 5, 0)).hasAlchemyCombinations());
    a.check("08",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 9)).hasAlchemyCombinations());
    a.check("09",  HostVersion(HostVersion::PHost,   MKVERSION(4, 1, 0)).hasAlchemyCombinations());
    a.check("10", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0, 0)).hasAlchemyCombinations());
}

// hasRefineryFCodes: PHost >= 3.4m, 4.0k
AFL_TEST("game.HostVersion:hasRefineryFCodes", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22, 0)).hasRefineryFCodes());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22, 0)).hasRefineryFCodes());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22, 0)).hasRefineryFCodes());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4, 0)).hasRefineryFCodes());
    a.check("05", !HostVersion(HostVersion::PHost,   MKVERSION(4, 0, 0)).hasRefineryFCodes());
    a.check("06",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,13)).hasRefineryFCodes());
    a.check("07",  HostVersion(HostVersion::PHost,   MKVERSION(3, 5, 0)).hasRefineryFCodes());
    a.check("08",  HostVersion(HostVersion::PHost,   MKVERSION(4, 0,11)).hasRefineryFCodes());
    a.check("09",  HostVersion(HostVersion::PHost,   MKVERSION(4, 1, 0)).hasRefineryFCodes());
    a.check("10", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0, 0)).hasRefineryFCodes());
}

// hasAlchemyExclusionFCodes: PHost only
AFL_TEST("game.HostVersion:hasAlchemyExclusionFCodes", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasAlchemyExclusionFCodes());
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasAlchemyExclusionFCodes());
    a.check("05", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasAlchemyExclusionFCodes());
}

// isAlchemyRounding
AFL_TEST("game.HostVersion:isAlchemyRounding", a)
{
    a.check("01", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isAlchemyRounding());
    a.check("02", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isAlchemyRounding());
    a.check("03", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isAlchemyRounding());
    a.check("04", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isAlchemyRounding());
    a.check("05", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isAlchemyRounding());
}

// isValidChunnelDistance2
// - 10000 (=100 ly) is ok for everyone
AFL_TEST("game.HostVersion:isValidChunnelDistance2", a)
{
    Ref<HostConfiguration> rconfig = HostConfiguration::create();
    HostConfiguration& config = *rconfig;
    config[config.MinimumChunnelDistance].set(100);
    a.check("01",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    a.check("02",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    a.check("03",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(10000, config));
    a.check("04",  HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(10000, config));
    a.check("05",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(10000, config));

    // - 9901 (=99.5 ly) is ok for Host
    a.check("11",  HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    a.check("12",  HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    a.check("13",  HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(9901, config));
    a.check("14", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(9901, config));
    a.check("15",  HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(9901, config));

    // - 100 (=10 ly) is not ok for anyone
    a.check("21", !HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    a.check("22", !HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    a.check("23", !HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).isValidChunnelDistance2(10, config));
    a.check("24", !HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).isValidChunnelDistance2(10, config));
    a.check("25", !HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).isValidChunnelDistance2(10, config));
}

// getMinimumFuelToInitiateChunnel
AFL_TEST("game.HostVersion:getMinimumFuelToInitiateChunnel", a)
{
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).getMinimumFuelToInitiateChunnel(), 50);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).getMinimumFuelToInitiateChunnel(), 51);
    a.checkEqual("05", HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).getMinimumFuelToInitiateChunnel(), 50);
}

// hasPermissiveClimateLimits
AFL_TEST("game.HostVersion:hasPermissiveClimateLimits", a)
{
    a.checkEqual("01", HostVersion(HostVersion::Unknown, MKVERSION(3,22,0)).hasPermissiveClimateLimits(), false);
    a.checkEqual("02", HostVersion(HostVersion::Host,    MKVERSION(3,22,0)).hasPermissiveClimateLimits(), true);
    a.checkEqual("03", HostVersion(HostVersion::SRace,   MKVERSION(3,22,0)).hasPermissiveClimateLimits(), true);
    a.checkEqual("04", HostVersion(HostVersion::PHost,   MKVERSION(3, 4,0)).hasPermissiveClimateLimits(), false);
    a.checkEqual("05", HostVersion(HostVersion::NuHost,  MKVERSION(3, 0,0)).hasPermissiveClimateLimits(), false);
}

/*
 *  Test setImpliedHostConfiguration().
 */

// Base case
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:base", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    a.checkEqual("CPEnableShow", c[HostConfiguration::CPEnableShow](), true);
    a.checkEqual("AllowExtendedMissions", c[HostConfiguration::AllowExtendedMissions](), true);
}

// Host
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:Host", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    HostVersion(HostVersion::Host, MKVERSION(3,22,0)).setImpliedHostConfiguration(c);
    a.checkEqual("CPEnableShow", c[HostConfiguration::CPEnableShow](), false);
    a.checkEqual("AllowExtendedMissions", c[HostConfiguration::AllowExtendedMissions](), false);
}

// Old PHost
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:old", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(3,2,5)).setImpliedHostConfiguration(c);
    a.checkEqual("CPEnableShow", c[HostConfiguration::CPEnableShow](), false);
    a.checkEqual("AllowExtendedMissions", c[HostConfiguration::AllowExtendedMissions](), true);
}

// New PHost
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:new", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(4,1,5)).setImpliedHostConfiguration(c);
    a.checkEqual("CPEnableShow", c[HostConfiguration::CPEnableShow](), true);
    a.checkEqual("AllowExtendedMissions", c[HostConfiguration::AllowExtendedMissions](), true);
}

// ...but it's not unconditionally enabled
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:disabled", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    c[HostConfiguration::CPEnableShow].set(false);
    c[HostConfiguration::AllowExtendedMissions].set(false);
    HostVersion(HostVersion::PHost, MKVERSION(4,1,5)).setImpliedHostConfiguration(c);
    a.checkEqual("CPEnableShow", c[HostConfiguration::CPEnableShow](), false);
    a.checkEqual("AllowExtendedMissions", c[HostConfiguration::AllowExtendedMissions](), false);
}

AFL_TEST("game.HostVersion:setImpliedHostConfiguration:Host:minefields", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c[HostConfiguration::UnitsPerTorpRate].set("1,2,3,4,5,6,7,8,9,10");
    HostVersion(HostVersion::Host, MKVERSION(3,22,40)).setImpliedHostConfiguration(c);
    a.checkEqual("01", c[HostConfiguration::UnitsPerTorpRate](1), 100);
    a.checkEqual("02", c[HostConfiguration::UnitsPerTorpRate](6), 100);
    a.checkEqual("03", c[HostConfiguration::UnitsPerTorpRate](9), 400);
    a.checkEqual("04", c[HostConfiguration::UnitsPerWebRate](1), 100);
    a.checkEqual("05", c[HostConfiguration::UnitsPerWebRate](6), 100);
    a.checkEqual("06", c[HostConfiguration::UnitsPerWebRate](9), 400);
}

AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:minefields", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c[HostConfiguration::UnitsPerTorpRate].set("1,2,3,4,5,6,7,8,9,10");
    HostVersion(HostVersion::PHost, MKVERSION(3,2,5)).setImpliedHostConfiguration(c);
    a.checkEqual("01", c[HostConfiguration::UnitsPerTorpRate](1), 1);   // set above
    a.checkEqual("02", c[HostConfiguration::UnitsPerTorpRate](6), 6);
    a.checkEqual("03", c[HostConfiguration::UnitsPerTorpRate](9), 9);
    a.checkEqual("04", c[HostConfiguration::UnitsPerWebRate](1), 100);  // default
    a.checkEqual("05", c[HostConfiguration::UnitsPerWebRate](6), 100);
    a.checkEqual("06", c[HostConfiguration::UnitsPerWebRate](9), 400);
}

// Host
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:Host:fuel-usage", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    c[HostConfiguration::FuelUsagePerFightFor100KT].set(3);
    c[HostConfiguration::FuelUsagePerTurnFor100KT].set(2);
    HostVersion(HostVersion::Host, MKVERSION(3,22,48)).setImpliedHostConfiguration(c);
    a.checkEqual("FuelUsagePerFightFor100KT", c[HostConfiguration::FuelUsagePerFightFor100KT](1), 0);
    a.checkEqual("FuelUsagePerTurnFor100KT",  c[HostConfiguration::FuelUsagePerTurnFor100KT](1), 0);
    a.checkEqual("hasExtraFuelConsumption",   c.hasExtraFuelConsumption(), false);
}

// PHost
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:fuel-usage", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    c[HostConfiguration::FuelUsagePerFightFor100KT].set(3);
    c[HostConfiguration::FuelUsagePerTurnFor100KT].set(2);
    HostVersion(HostVersion::PHost, MKVERSION(4,1,0)).setImpliedHostConfiguration(c);
    a.checkEqual("FuelUsagePerFightFor100KT", c[HostConfiguration::FuelUsagePerFightFor100KT](1), 3);
    a.checkEqual("FuelUsagePerTurnFor100KT",  c[HostConfiguration::FuelUsagePerTurnFor100KT](1), 2);
    a.checkEqual("hasExtraFuelConsumption",   c.hasExtraFuelConsumption(), true);
}

// Host
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:Host:build-queue", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    HostVersion(HostVersion::Host, MKVERSION(3,22,48)).setImpliedHostConfiguration(c);
    a.checkEqual("isPBPGame", c.isPBPGame(), true);
}

// PHost
AFL_TEST("game.HostVersion:setImpliedHostConfiguration:PHost:build-queue", a)
{
    Ref<HostConfiguration> rc = HostConfiguration::create();
    HostConfiguration& c = *rc;
    c.setDefaultValues();
    HostVersion(HostVersion::PHost, MKVERSION(4,1,0)).setImpliedHostConfiguration(c);
    // Default is PAL!
    a.checkEqual("isPBPGame", c.isPBPGame(), false);
}

/** Test fromString(). */
AFL_TEST("game.HostVersion:fromString", a)
{
    HostVersion v;

    // Unknown
    a.check("01. fromString", !v.fromString("unknown"));

    // Tim-Host
    a.check("11. fromString", !v.fromString("Host"));

    a.check("21. fromString", v.fromString("Host 3.0"));
    a.checkEqual("22. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("23. getVersion", v.getVersion(), MKVERSION(3,0,0));

    a.check("31. fromString", v.fromString("Host 3.16.001"));
    a.checkEqual("32. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("33. getVersion", v.getVersion(), MKVERSION(3,16,1));

    a.check("41. fromString", v.fromString("Host 3.2"));
    a.checkEqual("42. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("43. getVersion", v.getVersion(), MKVERSION(3,20,0));

    a.check("51. fromString", v.fromString("Host 3.20"));
    a.checkEqual("52. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("53. getVersion", v.getVersion(), MKVERSION(3,20,0));

    a.check("61. fromString", v.fromString("Host 3.20a"));
    a.checkEqual("62. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("63. getVersion", v.getVersion(), MKVERSION(3,20,1));

    a.check("71. fromString", v.fromString("Host 3.22.027"));
    a.checkEqual("72. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("73. getVersion", v.getVersion(), MKVERSION(3,22,27));

    // - 2-arg variant
    a.check("81. fromString", v.fromString("host", "3.0"));
    a.checkEqual("82. getKind", v.getKind(), HostVersion::Host);
    a.checkEqual("83. getVersion", v.getVersion(), MKVERSION(3,0,0));

    // PHost
    a.check("91. fromString", !v.fromString("PHost"));

    a.check("101. fromString", v.fromString("PHost 3.0"));
    a.checkEqual("102. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("103. getVersion", v.getVersion(), MKVERSION(3,0,0));

    a.check("111. fromString", v.fromString("PHost 3.16a"));
    a.checkEqual("112. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("113. getVersion", v.getVersion(), MKVERSION(3,16,1));

    a.check("121. fromString", v.fromString("PHost 3.20"));
    a.checkEqual("122. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("123. getVersion", v.getVersion(), MKVERSION(3,20,0));

    a.check("131. fromString", v.fromString("PHost 3.4e"));
    a.checkEqual("132. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("133. getVersion", v.getVersion(), MKVERSION(3,4,5));

    a.check("141. fromString", v.fromString("PHost 3.22.027"));
    a.checkEqual("142. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("143. getVersion", v.getVersion(), MKVERSION(3,22,27));

    a.check("151. fromString", v.fromString("PHost 3.4m"));
    a.checkEqual("152. getKind", v.getKind(), HostVersion::PHost);
    a.checkEqual("153. getVersion", v.getVersion(), MKVERSION(3,4,13));

    // SRace (Tim-Host variant)
    a.check("161. fromString", !v.fromString("SRace"));

    a.check("171. fromString", v.fromString("SRace 3.0"));
    a.checkEqual("172. getKind", v.getKind(), HostVersion::SRace);
    a.checkEqual("173. getVersion", v.getVersion(), MKVERSION(3,0,0));

    a.check("181. fromString", v.fromString("SRace 3.16.001"));
    a.checkEqual("182. getKind", v.getKind(), HostVersion::SRace);
    a.checkEqual("183. getVersion", v.getVersion(), MKVERSION(3,16,1));

    // NuHost
    a.check("191. fromString", !v.fromString("NuHost"));

    a.check("201. fromString", v.fromString("NuHost 3.0"));
    a.checkEqual("202. getKind", v.getKind(), HostVersion::NuHost);
    a.checkEqual("203. getVersion", v.getVersion(), MKVERSION(3,0,0));

    a.check("211. fromString", v.fromString("NuHost 3.16.001"));
    a.checkEqual("212. getKind", v.getKind(), HostVersion::NuHost);
    a.checkEqual("213. getVersion", v.getVersion(), MKVERSION(3,16,1));
}
