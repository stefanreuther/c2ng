/**
  *  \file game/config/hostconfiguration.cpp
  *  \brief Class game::config::HostConfiguration
  */

#include "game/config/hostconfiguration.hpp"
#include "game/config/integervalueparser.hpp"
#include "game/config/booleanvalueparser.hpp"
#include "afl/string/format.hpp"

namespace {
    template<typename T>
    void setDependantOption(T& out, const T& in)
    {
        if (!out.wasSet()) {
            out.copyFrom(in);
        }
    }
}


const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ConfigLevel = {
    "ConfigLevel",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PlayerRace = {
    "PlayerRace",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::RecycleRate = {
    "RecycleRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RandomMeteorRate = {
    "RandomMeteorRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowMinefields = {
    "AllowMinefields",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAlchemy = {
    "AllowAlchemy",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DeleteOldMessages = {
    "DeleteOldMessages",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DisablePasswords = {
    "DisablePasswords",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::GroundKillFactor = {
    "GroundKillFactor",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::GroundDefenseFactor = {
    "GroundDefenseFactor",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FreeFighters = {
    "FreeFighters",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::RaceMiningRate = {
    "RaceMiningRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::AllowBuildFighters = {
    "AllowBuildFighters",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RebelsBuildFighters = {
    "RebelsBuildFighters",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ColoniesBuildFighters = {
    "ColoniesBuildFighters",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RobotsBuildFighters = {
    "RobotsBuildFighters",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::CloakFailureRate = {
    "CloakFailureRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RobCloakedShips = {
    "RobCloakedShips",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ScanRange = {
    "ScanRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SensorRange = {
    "SensorRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::DarkSenseRange = {
    "DarkSenseRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowHiss = {
    "AllowHiss",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowRebelGroundAttack = {
    "AllowRebelGroundAttack",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowSuperRefit = {
    "AllowSuperRefit",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowWebMines = {
    "AllowWebMines",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::CloakFuelBurn = {
    "CloakFuelBurn",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowNewNatives = {
    "AllowNewNatives",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowPlanetAttacks = {
    "AllowPlanetAttacks",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BorgAssimilationRate = {
    "BorgAssimilationRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineDecayRate = {
    "MineDecayRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineDecayRate = {
    "WebMineDecayRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaximumMinefieldRadius = {
    "MaximumMinefieldRadius",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaximumWebMinefieldRadius = {
    "MaximumWebMinefieldRadius",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::TransuraniumDecayRate = {
    "TransuraniumDecayRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::StructureDecayPerTurn = {
    "StructureDecayPerTurn",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::StructureDecayOnUnowned = {
    "StructureDecayOnUnowned",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ClimateLimitsPopulation = {
    "ClimateLimitsPopulation",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::AllowEatingSupplies = {
    "AllowEatingSupplies",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowNoFuelMovement = {
    "AllowNoFuelMovement",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineHitOdds = {
    "MineHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineHitOdds = {
    "WebMineHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineScanRange = {
    "MineScanRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowMinesDestroyMines = {
    "AllowMinesDestroyMines",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowEngineShieldBonus = {
    "AllowEngineShieldBonus",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EngineShieldBonusRate = {
    "EngineShieldBonusRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterSweepRate = {
    "FighterSweepRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowColoniesSweepWebs = {
    "AllowColoniesSweepWebs",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineSweepRate = {
    "MineSweepRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineSweepRate = {
    "WebMineSweepRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::HissEffectRate = {
    "HissEffectRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::RobFailureOdds = {
    "RobFailureOdds",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::PlanetsAttackRebels = {
    "PlanetsAttackRebels",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::PlanetsAttackKlingons = {
    "PlanetsAttackKlingons",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineSweepRange = {
    "MineSweepRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineSweepRange = {
    "WebMineSweepRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowScienceMissions = {
    "AllowScienceMissions",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineHitOddsWhenCloakedX10 = {
    "MineHitOddsWhenCloakedX10",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DamageLevelForCloakFail = {
    "DamageLevelForCloakFail",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowFedCombatBonus = {
    "AllowFedCombatBonus",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MeteorShowerOdds = {
    "MeteorShowerOdds",
    &IntegerValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<8> game::config::HostConfiguration::MeteorShowerOreRanges = {
    "MeteorShowerOreRanges",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::LargeMeteorsImpacting = {
    "LargeMeteorsImpacting",
    &IntegerValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<8> game::config::HostConfiguration::LargeMeteorOreRanges = {
    "LargeMeteorOreRanges",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowMeteorMessages = {
    "AllowMeteorMessages",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowOneEngineTowing = {
    "AllowOneEngineTowing",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowHyperWarps = {
    "AllowHyperWarps",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ClimateDeathRate = {
    "ClimateDeathRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowGravityWells = {
    "AllowGravityWells",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CrystalsPreferDeserts = {
    "CrystalsPreferDeserts",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowMinesDestroyWebs = {
    "AllowMinesDestroyWebs",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaxPlanetaryIncome = {
    "MaxPlanetaryIncome",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::IonStormActivity = {
    "IonStormActivity",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowChunneling = {
    "AllowChunneling",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowDeluxeSuperSpy = {
    "AllowDeluxeSuperSpy",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::IonStormsHideMines = {
    "IonStormsHideMines",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowGloryDevice = {
    "AllowGloryDevice",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAntiCloakShips = {
    "AllowAntiCloakShips",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowGamblingShips = {
    "AllowGamblingShips",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowCloakedShipsAttack = {
    "AllowCloakedShipsAttack",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowShipCloning = {
    "AllowShipCloning",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowBoardingParties = {
    "AllowBoardingParties",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowImperialAssault = {
    "AllowImperialAssault",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RamScoopFuelPerLY = {
    "RamScoopFuelPerLY",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAdvancedRefinery = {
    "AllowAdvancedRefinery",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowBioscanners = {
    "AllowBioscanners",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::HullTechNotSlowedByMines = {
    "HullTechNotSlowedByMines",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::UseAccurateFuelModel = {
    "UseAccurateFuelModel",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DefenseForUndetectable = {
    "DefenseForUndetectable",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DefenseToBlockBioscan = {
    "DefenseToBlockBioscan",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::FactoriesForDetectable = {
    "FactoriesForDetectable",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MinesForDetectable = {
    "MinesForDetectable",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterSweepRange = {
    "FighterSweepRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MineHitDamageFor100KT = {
    "MineHitDamageFor100KT",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WebHitDamageFor100KT = {
    "WebHitDamageFor100KT",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowRegisteredFunctions = {
    "AllowRegisteredFunctions",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::GravityWellRange = {
    "GravityWellRange",
    &IntegerValueParser::instance,
};
const game::config::StringArrayOptionDescriptor game::config::HostConfiguration::Language = {
    "Language",
    0,
    game::MAX_PLAYERS+1
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowPlayerMessages = {
    "AllowPlayerMessages",
    &BooleanValueParser::instance,
};
const game::config::StringOptionDescriptor game::config::HostConfiguration::ScoringMethod = {
    "ScoringMethod",
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::TowedShipsBreakFree = {
    "TowedShipsBreakFree",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NativeClimateDeathRate = {
    "NativeClimateDeathRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::AllowMoreThan50Targets = {
    "AllowMoreThan50Targets",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CrystalSinTempBehavior = {
    "CrystalSinTempBehavior",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RGANeedsBeams = {
    "RGANeedsBeams",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowRGAOnUnowned = {
    "AllowRGAOnUnowned",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableLanguage = {
    "CPEnableLanguage",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableBigTargets = {
    "CPEnableBigTargets",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableRaceName = {
    "CPEnableRaceName",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableAllies = {
    "CPEnableAllies",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableMessage = {
    "CPEnableMessage",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAnonymousMessages = {
    "AllowAnonymousMessages",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DelayAllianceCommands = {
    "DelayAllianceCommands",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TerraformRate = {
    "TerraformRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MaxColTempSlope = {
    "MaxColTempSlope",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WebDrainFuelLoss = {
    "WebDrainFuelLoss",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WebHitFuelLoss = {
    "WebHitFuelLoss",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowWormholes = {
    "AllowWormholes",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmDisplacement = {
    "WrmDisplacement",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmRandDisplacement = {
    "WrmRandDisplacement",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmStabilityAddX10 = {
    "WrmStabilityAddX10",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmRandStability = {
    "WrmRandStability",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmMassAdd = {
    "WrmMassAdd",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmRandMass = {
    "WrmRandMass",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmVoluntaryTravel = {
    "WrmVoluntaryTravel",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmTravelDistDivisor = {
    "WrmTravelDistDivisor",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmTravelWarpSpeed = {
    "WrmTravelWarpSpeed",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmTravelCloaked = {
    "WrmTravelCloaked",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WrmEntryPowerX100 = {
    "WrmEntryPowerX100",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableGive = {
    "CPEnableGive",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowTowCloakedShips = {
    "AllowTowCloakedShips",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RobCloakedChance = {
    "RobCloakedChance",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::UnitsPerTorpRate = {
    "UnitsPerTorpRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::UnitsPerWebRate = {
    "UnitsPerWebRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowESBonusAgainstPlanets = {
    "AllowESBonusAgainstPlanets",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ShipCloneCostRate = {
    "ShipCloneCostRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowHyperjumpGravWells = {
    "AllowHyperjumpGravWells",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NativeCombatSurvivalRate = {
    "NativeCombatSurvivalRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowPrivateerTowCapture = {
    "AllowPrivateerTowCapture",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowCrystalTowCapture = {
    "AllowCrystalTowCapture",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::RoundGravityWells = {
    "RoundGravityWells",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableSend = {
    "CPEnableSend",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CumulativePillaging = {
    "CumulativePillaging",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowInterceptAttack = {
    "AllowInterceptAttack",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::RaceGrowthRate = {
    "RaceGrowthRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ProductionRate = {
    "ProductionRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineOddsWarpBonusX100 = {
    "MineOddsWarpBonusX100",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::CloakMineOddsWarpBonusX100 = {
    "CloakMineOddsWarpBonusX100",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineOddsWarpBonusX100 = {
    "WebMineOddsWarpBonusX100",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MineTravelSafeWarp = {
    "MineTravelSafeWarp",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::CloakedMineTravelSafeWarp = {
    "CloakedMineTravelSafeWarp",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WebMineTravelSafeWarp = {
    "WebMineTravelSafeWarp",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowCloakFailMessages = {
    "AllowCloakFailMessages",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::TonsScoreCountsPlanets = {
    "TonsScoreCountsPlanets",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowExtendedMissions = {
    "AllowExtendedMissions",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ExtMissionsStartAt = {
    "ExtMissionsStartAt",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::WormholeUFOsStartAt = {
    "WormholeUFOsStartAt",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MaxShipsHissing = {
    "MaxShipsHissing",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::SpyDetectionChance = {
    "SpyDetectionChance",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MapTruehullByPlayerRace = {
    "MapTruehullByPlayerRace",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowWraparoundMap = {
    "AllowWraparoundMap",
    &BooleanValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<4> game::config::HostConfiguration::WraparoundRectangle = {
    "WraparoundRectangle",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableRemote = {
    "CPEnableRemote",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAlliedChunneling = {
    "AllowAlliedChunneling",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ColonistTaxRate = {
    "ColonistTaxRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::NativeTaxRate = {
    "NativeTaxRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAlternativeTowing = {
    "AllowAlternativeTowing",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowBeamUpClans = {
    "AllowBeamUpClans",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowBeamUpMultiple = {
    "AllowBeamUpMultiple",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DamageLevelForTerraformFail = {
    "DamageLevelForTerraformFail",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DamageLevelForAntiCloakFail = {
    "DamageLevelForAntiCloakFail",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DamageLevelForChunnelFail = {
    "DamageLevelForChunnelFail",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::DamageLevelForHyperjumpFail = {
    "DamageLevelForHyperjumpFail",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MinimumChunnelDistance = {
    "MinimumChunnelDistance",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TowStrengthEngineScale = {
    "TowStrengthEngineScale",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TowStrengthDistanceScale = {
    "TowStrengthDistanceScale",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowPriorityBuild = {
    "AllowPriorityBuild",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SBQBuildPALBoost = {
    "SBQBuildPALBoost",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SBQNewBuildPALBoost = {
    "SBQNewBuildPALBoost",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SBQPointsForAging = {
    "SBQPointsForAging",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SBQBuildChangePenalty = {
    "SBQBuildChangePenalty",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::SBQBoostExpX100 = {
    "SBQBoostExpX100",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BuildChangeRelativePenalty = {
    "BuildChangeRelativePenalty",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALDecayPerTurn = {
    "PALDecayPerTurn",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALPlayerRate = {
    "PALPlayerRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALCombatAggressor = {
    "PALCombatAggressor",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALAggressorPointsPer10KT = {
    "PALAggressorPointsPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALOpponentPointsPer10KT = {
    "PALOpponentPointsPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALAggressorKillPointsPer10KT = {
    "PALAggressorKillPointsPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALOpponentKillPointsPer10KT = {
    "PALOpponentKillPointsPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALCombatPlanetScaling = {
    "PALCombatPlanetScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALCombatBaseScaling = {
    "PALCombatBaseScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALShipCapturePer10Crew = {
    "PALShipCapturePer10Crew",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALRecyclingPer10KT = {
    "PALRecyclingPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALBoardingPartyPer10Crew = {
    "PALBoardingPartyPer10Crew",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALGroundAttackPer100Clans = {
    "PALGroundAttackPer100Clans",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALGloryDevice = {
    "PALGloryDevice",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALGloryDevicePer10KT = {
    "PALGloryDevicePer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALGloryDamagePer10KT = {
    "PALGloryDamagePer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALGloryKillPer10KT = {
    "PALGloryKillPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALImperialAssault = {
    "PALImperialAssault",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALRGA = {
    "PALRGA",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALPillage = {
    "PALPillage",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALIncludesESB = {
    "PALIncludesESB",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FilterPlayerMessages = {
    "FilterPlayerMessages",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AlternativeAntiCloak = {
    "AlternativeAntiCloak",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::AntiCloakImmunity = {
    "AntiCloakImmunity",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::AllowMoreThan500Minefields = {
    "AllowMoreThan500Minefields",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NumMinefields = {
    "NumMinefields",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PALShipMinekillPer10KT = {
    "PALShipMinekillPer10KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaximumMinefieldsPerPlayer = {
    "MaximumMinefieldsPerPlayer",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MineIdNeedsPermission = {
    "MineIdNeedsPermission",
    &BooleanValueParser::instance,
};
const game::config::StringOptionDescriptor game::config::HostConfiguration::BuildQueue = {
    "BuildQueue",
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PBPCostPer100KT = {
    "PBPCostPer100KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PBPMinimumCost = {
    "PBPMinimumCost",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PBPCloneCostRate = {
    "PBPCloneCostRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowShipNames = {
    "AllowShipNames",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::BuildPointReport = {
    "BuildPointReport",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AlternativeMinesDestroyMines = {
    "AlternativeMinesDestroyMines",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NumShips = {
    "NumShips",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ExtendedSensorSweep = {
    "ExtendedSensorSweep",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ColonistCombatSurvivalRate = {
    "ColonistCombatSurvivalRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NewNativesPerTurn = {
    "NewNativesPerTurn",
    &IntegerValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<2> game::config::HostConfiguration::NewNativesPopulationRange = {
    "NewNativesPopulationRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<9> game::config::HostConfiguration::NewNativesRaceRate = {
    "NewNativesRaceRate",
    &IntegerValueParser::instance,
};
const game::config::IntegerArrayOptionDescriptor<9> game::config::HostConfiguration::NewNativesGovernmentRate = {
    "NewNativesGovernmentRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PlayerSpecialMission = {
    "PlayerSpecialMission",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::TowedShipsCooperate = {
    "TowedShipsCooperate",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::WrmScanRange = {
    "WrmScanRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FuelUsagePerFightFor100KT = {
    "FuelUsagePerFightFor100KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FuelUsagePerTurnFor100KT = {
    "FuelUsagePerTurnFor100KT",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableEnemies = {
    "CPEnableEnemies",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableShow = {
    "CPEnableShow",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::CPEnableRefit = {
    "CPEnableRefit",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowIncompatibleConfiguration = {
    "AllowIncompatibleConfiguration",
    &BooleanValueParser::instance,
};
const game::config::CostArrayOptionDescriptor game::config::HostConfiguration::FreeFighterCost = {
    "FreeFighterCost",
};
const game::config::CostArrayOptionDescriptor game::config::HostConfiguration::StarbaseCost = {
    "StarbaseCost",
};
const game::config::CostArrayOptionDescriptor game::config::HostConfiguration::BaseFighterCost = {
    "BaseFighterCost",
};
const game::config::CostArrayOptionDescriptor game::config::HostConfiguration::ShipFighterCost = {
    "ShipFighterCost",
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaximumFightersOnBase = {
    "MaximumFightersOnBase",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaximumDefenseOnBase = {
    "MaximumDefenseOnBase",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::NumExperienceLevels = {
    "NumExperienceLevels",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::ExperienceLevels = {
    "ExperienceLevels",
    &IntegerValueParser::instance,
};
const game::config::StringArrayOptionDescriptor game::config::HostConfiguration::ExperienceLevelNames = {
    "ExperienceLevelNames",
    0,
    MAX_EXPERIENCE_LEVELS+1
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ExperienceLimit = {
    "ExperienceLimit",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EPRecrewScaling = {
    "EPRecrewScaling",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipAging = {
    "EPShipAging",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPPlanetAging = {
    "EPPlanetAging",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPPlanetGovernment = {
    "EPPlanetGovernment",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipMovement100LY = {
    "EPShipMovement100LY",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipHyperjump = {
    "EPShipHyperjump",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipChunnel = {
    "EPShipChunnel",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipIonStorm100MEV = {
    "EPShipIonStorm100MEV",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPCombatKillScaling = {
    "EPCombatKillScaling",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPCombatDamageScaling = {
    "EPCombatDamageScaling",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::EPShipAlchemy100KT = {
    "EPShipAlchemy100KT",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EPCombatBoostRate = {
    "EPCombatBoostRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EPCombatBoostLevel = {
    "EPCombatBoostLevel",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EPTrainingScale = {
    "EPTrainingScale",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::ExactExperienceReports = {
    "ExactExperienceReports",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBayRechargeRate = {
    "EModBayRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBayRechargeBonus = {
    "EModBayRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBeamRechargeRate = {
    "EModBeamRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBeamRechargeBonus = {
    "EModBeamRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModTubeRechargeRate = {
    "EModTubeRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBeamHitFighterCharge = {
    "EModBeamHitFighterCharge",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModTorpHitOdds = {
    "EModTorpHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBeamHitOdds = {
    "EModBeamHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModBeamHitBonus = {
    "EModBeamHitBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModStrikesPerFighter = {
    "EModStrikesPerFighter",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModFighterBeamExplosive = {
    "EModFighterBeamExplosive",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModFighterBeamKill = {
    "EModFighterBeamKill",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModFighterMovementSpeed = {
    "EModFighterMovementSpeed",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModMaxFightersLaunched = {
    "EModMaxFightersLaunched",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModTorpHitBonus = {
    "EModTorpHitBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModTubeRechargeBonus = {
    "EModTubeRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModExtraFighterBays = {
    "EModExtraFighterBays",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModEngineShieldBonusRate = {
    "EModEngineShieldBonusRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModShieldDamageScaling = {
    "EModShieldDamageScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModShieldKillScaling = {
    "EModShieldKillScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModHullDamageScaling = {
    "EModHullDamageScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModCrewKillScaling = {
    "EModCrewKillScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModPlanetaryTorpsPerTube = {
    "EModPlanetaryTorpsPerTube",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::ExperienceOptionDescriptor_t game::config::HostConfiguration::EModMineHitOddsBonus = {
    "EModMineHitOddsBonus",
    &IntegerValueParser::instance,
};
const game::config::StringOptionDescriptor game::config::HostConfiguration::GameName = {
    "GameName",
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BayRechargeRate = {
    "BayRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BayRechargeBonus = {
    "BayRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamRechargeRate = {
    "BeamRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamRechargeBonus = {
    "BeamRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TubeRechargeRate = {
    "TubeRechargeRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamHitFighterCharge = {
    "BeamHitFighterCharge",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamHitShipCharge = {
    "BeamHitShipCharge",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TorpFiringRange = {
    "TorpFiringRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamFiringRange = {
    "BeamFiringRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TorpHitOdds = {
    "TorpHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamHitOdds = {
    "BeamHitOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamHitBonus = {
    "BeamHitBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::StrikesPerFighter = {
    "StrikesPerFighter",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterKillOdds = {
    "FighterKillOdds",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterBeamExplosive = {
    "FighterBeamExplosive",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterBeamKill = {
    "FighterBeamKill",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ShipMovementSpeed = {
    "ShipMovementSpeed",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterMovementSpeed = {
    "FighterMovementSpeed",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BayLaunchInterval = {
    "BayLaunchInterval",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::MaxFightersLaunched = {
    "MaxFightersLaunched",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowAlternativeCombat = {
    "AllowAlternativeCombat",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::StandoffDistance = {
    "StandoffDistance",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::PlanetsHaveTubes = {
    "PlanetsHaveTubes",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::PlanetaryTorpsPerTube = {
    "PlanetaryTorpsPerTube",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::FireOnAttackFighters = {
    "FireOnAttackFighters",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TorpHitBonus = {
    "TorpHitBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::TubeRechargeBonus = {
    "TubeRechargeBonus",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ShieldDamageScaling = {
    "ShieldDamageScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::HullDamageScaling = {
    "HullDamageScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::CrewKillScaling = {
    "CrewKillScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ShieldKillScaling = {
    "ShieldKillScaling",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ExtraFighterBays = {
    "ExtraFighterBays",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BeamHitFighterRange = {
    "BeamHitFighterRange",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::FighterFiringRange = {
    "FighterFiringRange",
    &IntegerValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::AllowVPAFeatures = {
    "AllowVPAFeatures",
    &BooleanValueParser::instance,
};
const game::config::IntegerOptionDescriptor game::config::HostConfiguration::MinimumHappiness = {
    "MinimumHappiness",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::ColonistCombatCaptureRate = {
    "ColonistCombatCaptureRate",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EPAcademyScale = {
    "EPAcademyScale",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::UseBaseTorpsInCombat = {
    "UseBaseTorpsInCombat",
    &BooleanValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::BaseTechCost = {
    "BaseTechCost",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EPShipBuild1000TorpUnits = {
    "EPShipBuild1000TorpUnits",
    &IntegerValueParser::instance,
};
const game::config::HostConfiguration::StandardOptionDescriptor_t game::config::HostConfiguration::EPShipBuild10Fighters = {
    "EPShipBuild10Fighters",
    &IntegerValueParser::instance,
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::CPEnableRumor = {
    "CPEnableRumor",
    "AllowAnonymousMessages",
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::RaceTaxRate = {
    "RaceTaxRate",
    "ColonistTaxRate",
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::CPNumMinefields = {
    "CPNumMinefields",
    "NumMinefields",
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::NativeClansRange = {
    "NativeClansRange",
    "NewNativesPopulationRange",
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::NativeTypeFrequencies = {
    "NativeTypeFrequencies",
    "NewNativesGovernmentRate",
};
const game::config::AliasOptionDescriptor game::config::HostConfiguration::NativeGovFrequencies = {
    "NativeGovFrequencies",
    "NewNativesGovernmentRate",
};



// Default constructor.
inline
game::config::HostConfiguration::HostConfiguration()
{
    setDefaultValues();
}

afl::base::Ref<game::config::HostConfiguration>
game::config::HostConfiguration::create()
{
    return *new HostConfiguration();
}

// Assign default values to all options.
void
game::config::HostConfiguration::setDefaultValues()
{
    // ex GGameConfig::assignDefaults

    // Set all options that have a value
    // These defaults are (mostly) the PHost default values.
    // A HCONFIG.HST file is never incomplete, so we need not deal with partially populated Tim-Host configuration at this point.
    HostConfiguration& me = *this;
    me[ConfigLevel].set(0);
    me[PlayerRace].set(0);
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        me[PlayerRace].set(i, i);
    }
    me[RecycleRate].set(75);
    me[RandomMeteorRate].set(2);
    me[AllowMinefields].set(1);
    me[AllowAlchemy].set(1);
    me[DeleteOldMessages].set(1);
    me[DisablePasswords].set(0);
    me[GroundKillFactor].set("1,30,1,15,1,1,1,1,1,1,1");
    me[GroundDefenseFactor].set("1,10,1,5,1,1,1,1,1,1,1");
    me[FreeFighters].set("0,0,0,0,0,0,0,10,0,0,0");
    me[RaceMiningRate].set("70,200,100,100,100,100,100,100,100,100,100");
    me[AllowBuildFighters].set("No,No,No,No,No,No,No,No,Yes,Yes,Yes");
    me[RebelsBuildFighters].set(1);
    me[ColoniesBuildFighters].set(1);
    me[RobotsBuildFighters].set(1);
    me[CloakFailureRate].set(1);
    me[RobCloakedShips].set(0);
    me[ScanRange].set(300);
    me[SensorRange].set(200);
    me[DarkSenseRange].copyFrom(me[SensorRange]);
    me[AllowHiss].set(1);
    me[AllowRebelGroundAttack].set(1);
    me[AllowSuperRefit].set(1);
    me[AllowWebMines].copyFrom(me[AllowMinefields]);
    me[CloakFuelBurn].set(5);
    me[AllowNewNatives].set(1);
    me[AllowPlanetAttacks].set(1);
    me[BorgAssimilationRate].set(100);
    me[MineDecayRate].set(5);
    me[WebMineDecayRate].copyFrom(me[MineDecayRate]);
    me[MaximumMinefieldRadius].set(150);
    me[MaximumWebMinefieldRadius].copyFrom(me[MaximumMinefieldRadius]);
    me[TransuraniumDecayRate].set(5);
    me[StructureDecayPerTurn].set(1);
    me[StructureDecayOnUnowned].set(1);
    me[ClimateLimitsPopulation].set(1);
    me[AllowEatingSupplies].set(0);
    me[AllowNoFuelMovement].set(1);
    me[MineHitOdds].set(1);
    me[WebMineHitOdds].set(5);
    me[MineScanRange].copyFrom(me[SensorRange]);
    me[AllowMinesDestroyMines].set(1);
    me[AllowEngineShieldBonus].set(0);
    me[EngineShieldBonusRate].set(0);
    me[FighterSweepRate].set("0,0,0,0,0,0,0,0,0,0,20");
    me[AllowColoniesSweepWebs].set(0);
    me[MineSweepRate].set(4);
    me[WebMineSweepRate].set(3);
    me[HissEffectRate].set(5);
    me[RobFailureOdds].set(1);
    me[PlanetsAttackRebels].set(0);
    me[PlanetsAttackKlingons].set(0);
    me[MineSweepRange].set(10);
    me[WebMineSweepRange].set(5);
    me[AllowScienceMissions].set(1);
    me[MineHitOddsWhenCloakedX10].set(5);
    me[DamageLevelForCloakFail].set(1);
    me[AllowFedCombatBonus].set(1);
    me[MeteorShowerOdds].set(0);
    me[MeteorShowerOreRanges].set("10,10,10,10,200,200,200,200");
    me[LargeMeteorsImpacting].set(0);
    me[LargeMeteorOreRanges].set("100,100,100,100,10000,9000,9000,7000");
    me[AllowMeteorMessages].set(1);
    me[AllowOneEngineTowing].set(0);
    me[AllowHyperWarps].set(1);
    me[ClimateDeathRate].set(10);
    me[AllowGravityWells].set(1);
    me[CrystalsPreferDeserts].set(1);
    me[AllowMinesDestroyWebs].set(0);
    me[MaxPlanetaryIncome].set(5000);
    me[IonStormActivity].set(0);
    me[AllowChunneling].set(1);
    me[AllowDeluxeSuperSpy].set(1);
    me[IonStormsHideMines].set(1);
    me[AllowGloryDevice].set(1);
    me[AllowAntiCloakShips].set(1);
    me[AllowGamblingShips].set(1);
    me[AllowCloakedShipsAttack].set(1);
    me[AllowShipCloning].set(1);
    me[AllowBoardingParties].set(1);
    me[AllowImperialAssault].set(1);
    me[RamScoopFuelPerLY].set(2);
    me[AllowAdvancedRefinery].set(1);
    me[AllowBioscanners].set(1);
    me[HullTechNotSlowedByMines].set(7);
    me[UseAccurateFuelModel].set(0);
    me[DefenseForUndetectable].set(15);
    me[DefenseToBlockBioscan].set(20);
    me[FactoriesForDetectable].set(15);
    me[MinesForDetectable].set(20);
    me[FighterSweepRange].set("0,0,0,0,0,0,0,0,0,0,100");
    me[MineHitDamageFor100KT].set(100);
    me[WebHitDamageFor100KT].set(10);
    me[AllowRegisteredFunctions].set(1);
    me[GravityWellRange].set(3);
    me[Language].set("English");
    me[AllowPlayerMessages].set(1);
    me[ScoringMethod].set("Compatible");
    me[TowedShipsBreakFree].set(0);
    me[NativeClimateDeathRate].set(0);
    me[AllowMoreThan50Targets].set(0);
    me[CrystalSinTempBehavior].set(0);
    me[RGANeedsBeams].set(0);
    me[AllowRGAOnUnowned].set(0);
    me[CPEnableLanguage].set(1);
    me[CPEnableBigTargets].set(1);
    me[CPEnableRaceName].set(1);
    me[CPEnableAllies].set(1);
    me[CPEnableMessage].set(1);
    me[AllowAnonymousMessages].copyFrom(me[AllowPlayerMessages]);
    me[DelayAllianceCommands].set(0);
    me[TerraformRate].set(1);
    me[MaxColTempSlope].set(1000);
    me[WebDrainFuelLoss].set(25);
    me[WebHitFuelLoss].set(50);
    me[AllowWormholes].set(1);
    me[WrmDisplacement].set(1);
    me[WrmRandDisplacement].set(0);
    me[WrmStabilityAddX10].set(0);
    me[WrmRandStability].set(0);
    me[WrmMassAdd].set(0);
    me[WrmRandMass].set(0);
    me[WrmVoluntaryTravel].set(1);
    me[WrmTravelDistDivisor].set(100);
    me[WrmTravelWarpSpeed].set(9);
    me[WrmTravelCloaked].set(0);
    me[WrmEntryPowerX100].set(25);
    me[CPEnableGive].set(1);
    me[AllowTowCloakedShips].set(0);
    me[RobCloakedChance].set(0);
    me[UnitsPerTorpRate].set("100,100,100,100,100,100,100,100,400,100,100");
    me[UnitsPerWebRate].copyFrom(me[UnitsPerTorpRate]);
    me[AllowESBonusAgainstPlanets].set(1);
    me[ShipCloneCostRate].set("200,200,200,200,32767,200,32767,200,200,200,200");
    me[AllowHyperjumpGravWells].copyFrom(me[AllowGravityWells]);
    me[NativeCombatSurvivalRate].set(75);
    me[AllowPrivateerTowCapture].copyFrom(me[AllowBoardingParties]);
    me[AllowCrystalTowCapture].copyFrom(me[AllowBoardingParties]);
    me[RoundGravityWells].set(0);
    me[CPEnableSend].set(1);
    me[CumulativePillaging].set(1);
    me[AllowInterceptAttack].set(1);
    me[RaceGrowthRate].set(100);
    me[ProductionRate].set(100);
    me[MineOddsWarpBonusX100].set(0);
    me[CloakMineOddsWarpBonusX100].set(0);
    me[WebMineOddsWarpBonusX100].set(0);
    me[MineTravelSafeWarp].set(0);
    me[CloakedMineTravelSafeWarp].copyFrom(me[MineTravelSafeWarp]);
    me[WebMineTravelSafeWarp].copyFrom(me[MineTravelSafeWarp]);
    me[AllowCloakFailMessages].set(1);
    me[TonsScoreCountsPlanets].set(0);
    me[AllowExtendedMissions].set(1);
    me[ExtMissionsStartAt].set(20);
    me[WormholeUFOsStartAt].set(51);
    me[MaxShipsHissing].set(500);
    me[SpyDetectionChance].set(20);
    me[MapTruehullByPlayerRace].set(0);
    me[AllowWraparoundMap].set(0);
    me[WraparoundRectangle].set("1000,1000,3000,3000");
    me[CPEnableRemote].set(1);
    me[AllowAlliedChunneling].set(1);
    me[ColonistTaxRate].set("200,100,100,100,100,100,100,100,100,100,100");
    me[NativeTaxRate].copyFrom(me[ColonistTaxRate]);
    me[AllowAlternativeTowing].set(0);
    me[AllowBeamUpClans].set(1);
    me[AllowBeamUpMultiple].set(1);
    me[DamageLevelForTerraformFail].set(100);
    me[DamageLevelForAntiCloakFail].set(20);
    me[DamageLevelForChunnelFail].set(100);
    me[DamageLevelForHyperjumpFail].set(100);
    me[MinimumChunnelDistance].set(100);
    me[TowStrengthEngineScale].set(1);
    me[TowStrengthDistanceScale].set(19);
    me[AllowPriorityBuild].set(1);
    me[SBQBuildPALBoost].set(1);
    me[SBQNewBuildPALBoost].set(1);
    me[SBQPointsForAging].set(1200);
    me[SBQBuildChangePenalty].set(2147483647);
    me[SBQBoostExpX100].set(0);
    me[BuildChangeRelativePenalty].set(100);
    me[PALDecayPerTurn].set(20);
    me[PALPlayerRate].set(100);
    me[PALCombatAggressor].set(0);
    me[PALAggressorPointsPer10KT].set(2);
    me[PALOpponentPointsPer10KT].set(2);
    me[PALAggressorKillPointsPer10KT].set(10);
    me[PALOpponentKillPointsPer10KT].set(10);
    me[PALCombatPlanetScaling].set(50);
    me[PALCombatBaseScaling].set(80);
    me[PALShipCapturePer10Crew].set(5);
    me[PALRecyclingPer10KT].set(4);
    me[PALBoardingPartyPer10Crew].set(3);
    me[PALGroundAttackPer100Clans].set(100);
    me[PALGloryDevice].set(100);
    me[PALGloryDevicePer10KT].set(0);
    me[PALGloryDamagePer10KT].set(2);
    me[PALGloryKillPer10KT].set(0);
    me[PALImperialAssault].set(100);
    me[PALRGA].set(10);
    me[PALPillage].set(10);
    me[PALIncludesESB].set(1);
    me[FilterPlayerMessages].set(0);
    me[AlternativeAntiCloak].set(0);
    me[AntiCloakImmunity].set("Yes,Yes,Yes,No,No,No,No,No,No,No,No");
    me[AllowMoreThan500Minefields].set(0);
    me[NumMinefields].set(500);
    me[PALShipMinekillPer10KT].set(0);
    me[MaximumMinefieldsPerPlayer].set(10000);
    me[MineIdNeedsPermission].set(0);
    me[BuildQueue].set("PAL");
    me[PBPCostPer100KT].set(200);
    me[PBPMinimumCost].set(400);
    me[PBPCloneCostRate].set(200);
    me[AllowShipNames].set(1);
    me[BuildPointReport].set(2 /* Allies */);
    me[AlternativeMinesDestroyMines].set(0);
    me[NumShips].set(500);
    me[ExtendedSensorSweep].set(1);
    me[ColonistCombatSurvivalRate].set(0);
    me[NewNativesPerTurn].set(1);
    me[NewNativesPopulationRange].set("2500,5000");
    me[NewNativesRaceRate].set(1);
    me[NewNativesGovernmentRate].set(1);
    me[PlayerSpecialMission].copyFrom(me[PlayerRace]);
    me[TowedShipsCooperate].set(1);
    me[WrmScanRange].set(100);
    me[FuelUsagePerFightFor100KT].set(0);
    me[FuelUsagePerTurnFor100KT].set(0);
    me[CPEnableEnemies].set(1);
    me[CPEnableShow].set(1);
    me[CPEnableRefit].set(1);
    me[AllowIncompatibleConfiguration].set(0);
    me[FreeFighterCost].set("T3 M2");
    me[StarbaseCost].set("T402 D120 M340 $900");
    me[BaseFighterCost].set("T3 M2 $100");
    me[ShipFighterCost].set("T3 M2 S5");
    me[MaximumFightersOnBase].set(60);
    me[MaximumDefenseOnBase].set(200);
    me[NumExperienceLevels].set(0);
    me[ExperienceLevels].set("750,1500,3000,6000");
    me[ExperienceLevelNames].set("Recruit,Soldier,Experienced,Elite,Ultra Elite");
    me[ExperienceLimit].set(1000000);
    me[EPRecrewScaling].set(30);
    me[EPShipAging].set(15);
    me[EPPlanetAging].set(25);
    me[EPPlanetGovernment].set(0);
    me[EPShipMovement100LY].set(40);
    me[EPShipHyperjump].set(30);
    me[EPShipChunnel].set(30);
    me[EPShipIonStorm100MEV].set(80);
    me[EPCombatKillScaling].set(800);
    me[EPCombatDamageScaling].set(200);
    me[EPShipAlchemy100KT].set(5);
    me[EPCombatBoostRate].set(100);
    me[EPCombatBoostLevel].set(0);
    me[EPTrainingScale].set(70);
    me[ExactExperienceReports].set(0);
    me[EModBayRechargeRate].set("1,2,3,4");
    me[EModBayRechargeBonus].set("0,0,0,0");
    me[EModBeamRechargeRate].set("0,0,0,0");
    me[EModBeamRechargeBonus].set("0,0,0,0");
    me[EModTubeRechargeRate].set("1,2,3,8");
    me[EModBeamHitFighterCharge].set("0,0,0,0");
    me[EModTorpHitOdds].set("9,18,27,35");
    me[EModBeamHitOdds].set("0,0,0,0");
    me[EModBeamHitBonus].set("0,0,0,0");
    me[EModStrikesPerFighter].set("1,2,3,4");
    me[EModFighterBeamExplosive].set("0,0,0,0");
    me[EModFighterBeamKill].set("0,0,0,0");
    me[EModFighterMovementSpeed].set("0,0,0,0");
    me[EModMaxFightersLaunched].set("0,0,0,0");
    me[EModTorpHitBonus].set("0,0,0,0");
    me[EModTubeRechargeBonus].set("0,0,0,0");
    me[EModExtraFighterBays].set("0,0,0,0");
    me[EModEngineShieldBonusRate].set("0,0,0,0");
    me[EModShieldDamageScaling].set("0,0,0,0");
    me[EModShieldKillScaling].set("0,0,0,0");
    me[EModHullDamageScaling].set("0,0,0,0");
    me[EModCrewKillScaling].set("-5,-10,-15,-20");
    me[EModPlanetaryTorpsPerTube].set("0,0,0,0");
    me[EModMineHitOddsBonus].set("5,10,15,20");
    me[GameName].set("Nameless Game");
    me[BayRechargeRate].set(52);
    me[BayRechargeBonus].set(2);
    me[BeamRechargeRate].set(8);
    me[BeamRechargeBonus].set(0);
    me[TubeRechargeRate].set(45);
    me[BeamHitFighterCharge].set(500);
    me[BeamHitShipCharge].set(600);
    me[TorpFiringRange].set(30000);
    me[BeamFiringRange].set(20000);
    me[TorpHitOdds].set(65);
    me[BeamHitOdds].set(100);
    me[BeamHitBonus].set(0);
    me[StrikesPerFighter].set(7);
    me[FighterKillOdds].set(20);
    me[FighterBeamExplosive].set(2);
    me[FighterBeamKill].set(2);
    me[ShipMovementSpeed].set(75);
    me[FighterMovementSpeed].set(235);
    me[BayLaunchInterval].set(3);
    me[MaxFightersLaunched].set(19);
    me[AllowAlternativeCombat].set(0);
    me[StandoffDistance].set(3000);
    me[PlanetsHaveTubes].set(0);
    me[PlanetaryTorpsPerTube].set(3);
    me[FireOnAttackFighters].set(0);
    me[TorpHitBonus].set(0);
    me[TubeRechargeBonus].set(0);
    me[ShieldDamageScaling].set(80);
    me[HullDamageScaling].set(80);
    me[CrewKillScaling].set(80);
    me[ShieldKillScaling].set(0);
    me[ExtraFighterBays].set("3,0,0,0,0,0,0,0,0,0,0");
    me[BeamHitFighterRange].set(100000);
    me[FighterFiringRange].set(3000);
    me[AllowVPAFeatures].set(1);
    me[MinimumHappiness].set(20);
    me[ColonistCombatCaptureRate].set(100);
    me[EPAcademyScale].set(400);
    me[UseBaseTorpsInCombat].set(1);
    me[BaseTechCost].set(100);
    me[EPShipBuild1000TorpUnits].set(0);
    me[EPShipBuild10Fighters].set(0);

    // Access all alias options to have their slot created
    me[CPEnableRumor];
    me[RaceTaxRate];
    me[CPNumMinefields];
    me[NativeClansRange];
    me[NativeTypeFrequencies];
    me[NativeGovFrequencies];

    markAllOptionsUnset();
}


// Assign dependant options.
void
game::config::HostConfiguration::setDependantOptions()
{
    // GGameConfig::assignDependantOptions()
    HostConfiguration& me = *this;
    setDependantOption(me[DarkSenseRange], me[SensorRange]);
    setDependantOption(me[AllowWebMines], me[AllowMinefields]);
    setDependantOption(me[WebMineDecayRate], me[MineDecayRate]);
    setDependantOption(me[MaximumWebMinefieldRadius], me[MaximumMinefieldRadius]);
    setDependantOption(me[MineScanRange], me[SensorRange]);
    setDependantOption(me[AllowAnonymousMessages], me[AllowPlayerMessages]);
    setDependantOption(me[UnitsPerWebRate], me[UnitsPerTorpRate]);
    setDependantOption(me[AllowHyperjumpGravWells], me[AllowGravityWells]);
    setDependantOption(me[AllowPrivateerTowCapture], me[AllowBoardingParties]);
    setDependantOption(me[AllowCrystalTowCapture], me[AllowBoardingParties]);
    setDependantOption(me[CloakedMineTravelSafeWarp], me[MineTravelSafeWarp]);
    setDependantOption(me[WebMineTravelSafeWarp], me[MineTravelSafeWarp]);
    setDependantOption(me[NativeTaxRate], me[ColonistTaxRate]);
    setDependantOption(me[PlayerSpecialMission], me[PlayerRace]);
}


// Get player race number.
int32_t
game::config::HostConfiguration::getPlayerRaceNumber(int player) const
{
    // ex game/config.h::raceId
    if (player > 0 && player <= MAX_PLAYERS) {
        return (*this)[PlayerRace](player);
    } else {
        return player;
    }
}

// Get player mission number.
int32_t
game::config::HostConfiguration::getPlayerMissionNumber(int player) const
{
    // ex game/config.h::raceMissionId, mission.pas:PlayerMission
    if (player > 0 && player <= MAX_PLAYERS) {
        return (*this)[PlayerSpecialMission](player);
    } else {
        return player;
    }
}

// Get experience level name.
String_t
game::config::HostConfiguration::getExperienceLevelName(int level, afl::string::Translator& tx) const
{
    // ex game/config.cc::getExperienceLevelName
    // ex pconfig.pas:ExperienceLevelName
    String_t s = (*this)[ExperienceLevelNames](level);
    if (s.size()) {
        return s;
    } else {
        return afl::string::Format(tx("Level %d"), level);
    }
}

// Get experience level, given a number of experience points.
int
game::config::HostConfiguration::getExperienceLevelFromPoints(int32_t points) const
{
    int level = 0;
    while (level < (*this)[NumExperienceLevels]() && points >= (*this)[ExperienceLevels](level+1)) {
        ++level;
    }
    return level;
}

// Get experience bonus.
int32_t
game::config::HostConfiguration::getExperienceBonus(const ExperienceOptionDescriptor_t& option, int level) const
{
    // ex game/config.cc::getExperienceBonus
    if (level <= 0 || level > MAX_EXPERIENCE_LEVELS) {
        return 0;
    } else {
        return (*this)[option](level);
    }
}

// Get set of all players of a particular race.
game::PlayerSet_t
game::config::HostConfiguration::getPlayersOfRace(int race) const
{
    // ex game/config.cc::getPlayersOfRace
    return getPlayersWhere(PlayerRace, race);
}

// Get set of all players where an option is enabled.
game::PlayerSet_t
game::config::HostConfiguration::getPlayersWhereEnabled(const StandardOptionDescriptor_t& opt) const
{
    const StandardOption_t& optionValue = (*this)[opt];
    PlayerSet_t result;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (optionValue(i) != 0) {
            result += i;
        }
    }
    return result;
}

// Get set of all players where an option has a given scalar value.
game::PlayerSet_t
game::config::HostConfiguration::getPlayersWhere(const StandardOptionDescriptor_t& opt, int value) const
{
    const StandardOption_t& optionValue = (*this)[opt];
    PlayerSet_t result;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (optionValue(i) == value) {
            result += i;
        }
    }
    return result;
}

// Get set of all players where an option has a given cost value.
game::PlayerSet_t
game::config::HostConfiguration::getPlayersWhere(const CostArrayOptionDescriptor& opt, const game::spec::Cost& value) const
{
    const CostArrayOption& optionValue = (*this)[opt];
    PlayerSet_t result;
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        if (optionValue(i) == value) {
            result += i;
        }
    }
    return result;
}

// Check for presence of extra per-turn/per-fight fuel consumption.
bool
game::config::HostConfiguration::hasExtraFuelConsumption() const
{
    // ex pconfig.pas:IsEugeneGame, HostVersion::isEugeneGame()
    return getPlayersWhereEnabled(FuelUsagePerFightFor100KT).nonempty()
        || getPlayersWhereEnabled(FuelUsagePerTurnFor100KT).nonempty();
}

// Check for PBP build queue.
bool
game::config::HostConfiguration::isPBPGame() const
{
    return afl::string::strCaseCompare((*this)[BuildQueue]().substr(0, 3), "pbp") == 0;
}

// Check for doubled torpedo power.
bool
game::config::HostConfiguration::hasDoubleTorpedoPower() const
{
    // ex HostVersion::hasDoubleTorpedoPower
    return (*this)[AllowAlternativeCombat]() == 0;
}
