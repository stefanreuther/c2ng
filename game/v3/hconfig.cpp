/**
  *  \file game/v3/hconfig.cpp
  *  \brief HConfig Access Functions
  */

#include "game/v3/hconfig.hpp"

namespace gt = game::v3::structures;
using game::config::HostConfiguration;

namespace {
    /*
     *  Import (image -> internal)
     */

    /** Import 11 WORDs from HCONFIG image (per-player settings). */
    void importArray16(HostConfiguration::StandardOption_t& option, const gt::Int16_t (&image)[12])
    {
        for (int i = 1; i <= 11; ++i) {
            option.set(i, image[i]);
        }
    }

    /** Import 8 DWORDs from HCONFIG image (meteor settings). */
    void importArray32(game::config::IntegerArrayOption<8>& option, const gt::Int32_t (&image)[8])
    {
        for (int i = 1; i <= 8; ++i) {
            option.set(i, image[i-1]);
        }
    }

    /** Import a race-specific scalar option into a per-player array.
        Elements that match race are set to ifTrue, others are set to ifFalse. */
    void importRaceArray(HostConfiguration::StandardOption_t& option, const HostConfiguration::StandardOption_t& playerRace, int race, int ifTrue, int ifFalse)
    {
        for (int i = 1; i <= gt::NUM_PLANETS; ++i) {
            option.set(i, playerRace(i) == race ? ifTrue : ifFalse);
        }
    }


    /*
     *  Export (internal -> image)
     */

    const int DEFAULT_RACE = 3;

    /** Export arrayized integer.
        Internally, we store the array. HCONFIG contains just one element, so arbitrarily pick one (DEFAULT_RACE). */
    void exportArrayizedInteger(gt::Int16_t& out, const HostConfiguration::StandardOption_t& in)
    {
        out = static_cast<int16_t>(in(DEFAULT_RACE));
    }

    /** Export single 16-bit integer. */
    void exportInteger(gt::Int16_t& out, const game::config::IntegerOption& in)
    {
        out = static_cast<int16_t>(in());
    }

    /** Export 11-element array. This also initializes the unused 12th element.  */
    void exportArray16(gt::Int16_t (&image)[12], const HostConfiguration::StandardOption_t& option)
    {
        image[0] = 0;
        for (int i = 1; i <= 11; ++i) {
            image[i] = static_cast<int16_t>(option(i));
        }
    }

    /** Export 8-element array. */
    void exportArray32(gt::Int32_t (&image)[8], const game::config::IntegerArrayOption<8>& option)
    {
        for (int i = 1; i <= 8; ++i) {
            image[i-1] = option(i);
        }
    }

    /** Export a race-specific scalar option from a per-player array.
        Return an element that matches the given race; ifNone if none found. */
    int16_t exportRaceArray(const HostConfiguration::StandardOption_t& option, const HostConfiguration::StandardOption_t& playerRace, int race, int16_t ifNone)
    {
        for (int i = 1; i <= gt::NUM_PLANETS; ++i) {
            if (playerRace(i) == race) {
                return static_cast<int16_t>(option(i));
            }
        }
        return ifNone;
    }
}

// Unpack HCONFIG.HST image into internal structure.
void
game::v3::unpackHConfig(const structures::HConfig& data, const size_t size,
                        game::config::HostConfiguration& config,
                        const game::config::ConfigurationOption::Source source)
{
    // Assign values.
    // Instead of checking each option's position, we only check known version boundaries.
    if (size >= 10) {
        config[HostConfiguration::RecycleRate].set(data.RecycleRate); config[HostConfiguration::RecycleRate].setSource(source);
        config[HostConfiguration::RandomMeteorRate].set(data.RandomMeteorRate); config[HostConfiguration::RandomMeteorRate].setSource(source);
        config[HostConfiguration::AllowMinefields].set(data.AllowMinefields); config[HostConfiguration::AllowMinefields].setSource(source);
        config[HostConfiguration::AllowAlchemy].set(data.AllowAlchemy); config[HostConfiguration::AllowAlchemy].setSource(source);
        config[HostConfiguration::DeleteOldMessages].set(data.DeleteOldMessages); config[HostConfiguration::DeleteOldMessages].setSource(source);
    }
    if (size >= 186) {
        config[HostConfiguration::DisablePasswords].set(data.DisablePasswords); config[HostConfiguration::DisablePasswords].setSource(source);
        importArray16(config[HostConfiguration::GroundKillFactor], data.GroundKillFactor); config[HostConfiguration::GroundKillFactor].setSource(source);
        importArray16(config[HostConfiguration::GroundDefenseFactor], data.GroundDefenseFactor); config[HostConfiguration::GroundDefenseFactor].setSource(source);
        importArray16(config[HostConfiguration::FreeFighters], data.FreeFighters); config[HostConfiguration::FreeFighters].setSource(source);
        importArray16(config[HostConfiguration::RaceMiningRate], data.RaceMiningRate); config[HostConfiguration::RaceMiningRate].setSource(source);
        importArray16(config[HostConfiguration::ColonistTaxRate], data.ColonistTaxRate); config[HostConfiguration::ColonistTaxRate].setSource(source);
        config[HostConfiguration::RebelsBuildFighters].set(data.RebelsBuildFighters); config[HostConfiguration::RebelsBuildFighters].setSource(source);
        config[HostConfiguration::ColoniesBuildFighters].set(data.ColoniesBuildFighters); config[HostConfiguration::ColoniesBuildFighters].setSource(source);
        config[HostConfiguration::RobotsBuildFighters].set(data.RobotsBuildFighters); config[HostConfiguration::RobotsBuildFighters].setSource(source);
        config[HostConfiguration::CloakFailureRate].set(data.CloakFailureRate); config[HostConfiguration::CloakFailureRate].setSource(source);
        config[HostConfiguration::RobCloakedShips].set(data.RobCloakedShips); config[HostConfiguration::RobCloakedShips].setSource(source);
        config[HostConfiguration::ScanRange].set(data.ScanRange); config[HostConfiguration::ScanRange].setSource(source);
        config[HostConfiguration::DarkSenseRange].set(data.DarkSenseRange); config[HostConfiguration::DarkSenseRange].setSource(source);
        config[HostConfiguration::AllowHiss].set(data.AllowHiss); config[HostConfiguration::AllowHiss].setSource(source);
        config[HostConfiguration::AllowRebelGroundAttack].set(data.AllowRebelGroundAttack); config[HostConfiguration::AllowRebelGroundAttack].setSource(source);
        config[HostConfiguration::AllowSuperRefit].set(data.AllowSuperRefit); config[HostConfiguration::AllowSuperRefit].setSource(source);
        config[HostConfiguration::AllowWebMines].set(data.AllowWebMines); config[HostConfiguration::AllowWebMines].setSource(source);
        config[HostConfiguration::CloakFuelBurn].set(data.CloakFuelBurn); config[HostConfiguration::CloakFuelBurn].setSource(source);
        config[HostConfiguration::SensorRange].set(data.SensorRange); config[HostConfiguration::SensorRange].setSource(source);
        config[HostConfiguration::AllowNewNatives].set(data.AllowNewNatives); config[HostConfiguration::AllowNewNatives].setSource(source);
        config[HostConfiguration::AllowPlanetAttacks].set(data.AllowPlanetAttacks); config[HostConfiguration::AllowPlanetAttacks].setSource(source);
        config[HostConfiguration::BorgAssimilationRate].set(data.BorgAssimilationRate); config[HostConfiguration::BorgAssimilationRate].setSource(source);
        config[HostConfiguration::WebMineDecayRate].set(data.WebMineDecayRate); config[HostConfiguration::WebMineDecayRate].setSource(source);
        config[HostConfiguration::MineDecayRate].set(data.MineDecayRate); config[HostConfiguration::MineDecayRate].setSource(source);
        config[HostConfiguration::MaximumMinefieldRadius].set(data.MaximumMinefieldRadius); config[HostConfiguration::MaximumMinefieldRadius].setSource(source);
        config[HostConfiguration::TransuraniumDecayRate].set(data.TransuraniumDecayRate); config[HostConfiguration::TransuraniumDecayRate].setSource(source);
        config[HostConfiguration::StructureDecayPerTurn].set(data.StructureDecayPerTurn); config[HostConfiguration::StructureDecayPerTurn].setSource(source);
        config[HostConfiguration::AllowEatingSupplies].set(data.AllowEatingSupplies); config[HostConfiguration::AllowEatingSupplies].setSource(source);
        config[HostConfiguration::AllowNoFuelMovement].set(data.AllowNoFuelMovement); config[HostConfiguration::AllowNoFuelMovement].setSource(source);
        config[HostConfiguration::MineHitOdds].set(data.MineHitOdds); config[HostConfiguration::MineHitOdds].setSource(source);
        config[HostConfiguration::WebMineHitOdds].set(data.WebMineHitOdds); config[HostConfiguration::WebMineHitOdds].setSource(source);
        config[HostConfiguration::MineScanRange].set(data.MineScanRange); config[HostConfiguration::MineScanRange].setSource(source);
        config[HostConfiguration::AllowMinesDestroyMines].set(data.AllowMinesDestroyMines); config[HostConfiguration::AllowMinesDestroyMines].setSource(source);
    }
    if (size >= 288) {
        config[HostConfiguration::AllowEngineShieldBonus].set(data.AllowEngineShieldBonus); config[HostConfiguration::AllowEngineShieldBonus].setSource(source);
        config[HostConfiguration::EngineShieldBonusRate].set(data.EngineShieldBonusRate); config[HostConfiguration::EngineShieldBonusRate].setSource(source);

        importRaceArray(config[HostConfiguration::FighterSweepRate], config[HostConfiguration::PlayerRace], 11, data.ColonialFighterSweepRate, 0);
        config[HostConfiguration::FighterSweepRate].setSource(source);

        config[HostConfiguration::AllowColoniesSweepWebs].set(data.AllowColoniesSweepWebs); config[HostConfiguration::AllowColoniesSweepWebs].setSource(source);
        config[HostConfiguration::MineSweepRate].set(data.MineSweepRate); config[HostConfiguration::MineSweepRate].setSource(source);
        config[HostConfiguration::WebMineSweepRate].set(data.WebMineSweepRate); config[HostConfiguration::WebMineSweepRate].setSource(source);
        config[HostConfiguration::HissEffectRate].set(data.HissEffectRate); config[HostConfiguration::HissEffectRate].setSource(source);
        config[HostConfiguration::RobFailureOdds].set(data.RobFailureOdds); config[HostConfiguration::RobFailureOdds].setSource(source);
        config[HostConfiguration::PlanetsAttackRebels].set(data.PlanetsAttackRebels); config[HostConfiguration::PlanetsAttackRebels].setSource(source);
        config[HostConfiguration::PlanetsAttackKlingons].set(data.PlanetsAttackKlingons); config[HostConfiguration::PlanetsAttackKlingons].setSource(source);
        config[HostConfiguration::MineSweepRange].set(data.MineSweepRange); config[HostConfiguration::MineSweepRange].setSource(source);
        config[HostConfiguration::WebMineSweepRange].set(data.WebMineSweepRange); config[HostConfiguration::WebMineSweepRange].setSource(source);
        config[HostConfiguration::AllowScienceMissions].set(data.AllowScienceMissions); config[HostConfiguration::AllowScienceMissions].setSource(source);
        config[HostConfiguration::MineHitOddsWhenCloakedX10].set(data.MineHitOddsWhenCloakedX10); config[HostConfiguration::MineHitOddsWhenCloakedX10].setSource(source);
        config[HostConfiguration::DamageLevelForCloakFail].set(data.DamageLevelForCloakFail); config[HostConfiguration::DamageLevelForCloakFail].setSource(source);
        config[HostConfiguration::AllowFedCombatBonus].set(data.AllowFedCombatBonus); config[HostConfiguration::AllowFedCombatBonus].setSource(source);
        config[HostConfiguration::MeteorShowerOdds].set(data.MeteorShowerOdds); config[HostConfiguration::MeteorShowerOdds].setSource(source);
        importArray32(config[HostConfiguration::MeteorShowerOreRanges], data.MeteorShowerOreRanges); config[HostConfiguration::MeteorShowerOreRanges].setSource(source);
        config[HostConfiguration::LargeMeteorsImpacting].set(data.LargeMeteorsImpacting); config[HostConfiguration::LargeMeteorsImpacting].setSource(source);
        importArray32(config[HostConfiguration::LargeMeteorOreRanges], data.LargeMeteorOreRanges); config[HostConfiguration::LargeMeteorOreRanges].setSource(source);
        config[HostConfiguration::AllowMeteorMessages].set(data.AllowMeteorMessages); config[HostConfiguration::AllowMeteorMessages].setSource(source);
    }
    if (size >= 298) {
        config[HostConfiguration::AllowOneEngineTowing].set(data.AllowOneEngineTowing); config[HostConfiguration::AllowOneEngineTowing].setSource(source);
        config[HostConfiguration::AllowHyperWarps].set(data.AllowHyperWarps); config[HostConfiguration::AllowHyperWarps].setSource(source);
        config[HostConfiguration::ClimateDeathRate].set(data.ClimateDeathRate); config[HostConfiguration::ClimateDeathRate].setSource(source);
        config[HostConfiguration::AllowGravityWells].set(data.AllowGravityWells); config[HostConfiguration::AllowGravityWells].setSource(source);
        config[HostConfiguration::CrystalsPreferDeserts].set(data.CrystalsPreferDeserts); config[HostConfiguration::CrystalsPreferDeserts].setSource(source);
    }
    if (size >= 302) {
        config[HostConfiguration::AllowMinesDestroyWebs].set(data.AllowMinesDestroyWebs); config[HostConfiguration::AllowMinesDestroyWebs].setSource(source);
        config[HostConfiguration::ClimateLimitsPopulation].set(data.ClimateLimitsPopulation); config[HostConfiguration::ClimateLimitsPopulation].setSource(source);
    }
    if (size >= 328) {
        config[HostConfiguration::MaxPlanetaryIncome].set(data.MaxPlanetaryIncome); config[HostConfiguration::MaxPlanetaryIncome].setSource(source);
        config[HostConfiguration::IonStormActivity].set(data.IonStormActivity); config[HostConfiguration::IonStormActivity].setSource(source);
        config[HostConfiguration::AllowChunneling].set(data.AllowChunneling); config[HostConfiguration::AllowChunneling].setSource(source);
        config[HostConfiguration::AllowDeluxeSuperSpy].set(data.AllowDeluxeSuperSpy); config[HostConfiguration::AllowDeluxeSuperSpy].setSource(source);
        config[HostConfiguration::IonStormsHideMines].set(data.IonStormsHideMines); config[HostConfiguration::IonStormsHideMines].setSource(source);
        config[HostConfiguration::AllowGloryDevice].set(data.AllowGloryDevice); config[HostConfiguration::AllowGloryDevice].setSource(source);
        config[HostConfiguration::AllowAntiCloakShips].set(data.AllowAntiCloakShips); config[HostConfiguration::AllowAntiCloakShips].setSource(source);
        config[HostConfiguration::AllowGamblingShips].set(data.AllowGamblingShips); config[HostConfiguration::AllowGamblingShips].setSource(source);
        config[HostConfiguration::AllowCloakedShipsAttack].set(data.AllowCloakedShipsAttack); config[HostConfiguration::AllowCloakedShipsAttack].setSource(source);
        config[HostConfiguration::AllowShipCloning].set(data.AllowShipCloning); config[HostConfiguration::AllowShipCloning].setSource(source);
        config[HostConfiguration::AllowBoardingParties].set(data.AllowBoardingParties); config[HostConfiguration::AllowBoardingParties].setSource(source);
        config[HostConfiguration::AllowImperialAssault].set(data.AllowImperialAssault); config[HostConfiguration::AllowImperialAssault].setSource(source);
    }
    if (size >= 336) {
        config[HostConfiguration::RamScoopFuelPerLY].set(data.RamScoopFuelPerLY); config[HostConfiguration::RamScoopFuelPerLY].setSource(source);
        config[HostConfiguration::AllowAdvancedRefinery].set(data.AllowAdvancedRefinery); config[HostConfiguration::AllowAdvancedRefinery].setSource(source);
        config[HostConfiguration::AllowBioscanners].set(data.AllowBioscanners); config[HostConfiguration::AllowBioscanners].setSource(source);
        config[HostConfiguration::HullTechNotSlowedByMines].set(data.HullTechNotSlowedByMines); config[HostConfiguration::HullTechNotSlowedByMines].setSource(source);
    }
    if (size >= 338) {
        importRaceArray(config[HostConfiguration::AntiCloakImmunity], config[HostConfiguration::PlayerRace], 3, !data.LokiDecloaksBirds, 0);
        config[HostConfiguration::AntiCloakImmunity].setSource(source);
    }
    if (size >= 340) {
        config[HostConfiguration::AllowVPAFeatures].set(data.AllowVPAFeatures); config[HostConfiguration::AllowVPAFeatures].setSource(source);
    }
}

// Pack HCONFIG.HST from internal structure.
void
game::v3::packHConfig(structures::HConfig& data, const HostConfiguration& config)
{
    exportArrayizedInteger(data.RecycleRate,               config[HostConfiguration::RecycleRate]);
    exportInteger(data.RandomMeteorRate,                   config[HostConfiguration::RandomMeteorRate]);
    exportInteger(data.AllowMinefields,                    config[HostConfiguration::AllowMinefields]);
    exportInteger(data.AllowAlchemy,                       config[HostConfiguration::AllowAlchemy]);
    exportInteger(data.DeleteOldMessages,                  config[HostConfiguration::DeleteOldMessages]);

    exportInteger(data.DisablePasswords,                   config[HostConfiguration::DisablePasswords]);
    exportArray16(data.GroundKillFactor,                   config[HostConfiguration::GroundKillFactor]);
    exportArray16(data.GroundDefenseFactor,                config[HostConfiguration::GroundDefenseFactor]);
    exportArray16(data.FreeFighters,                       config[HostConfiguration::FreeFighters]);
    exportArray16(data.RaceMiningRate,                     config[HostConfiguration::RaceMiningRate]);
    exportArray16(data.ColonistTaxRate,                    config[HostConfiguration::ColonistTaxRate]);
    exportInteger(data.RebelsBuildFighters,                config[HostConfiguration::RebelsBuildFighters]);
    exportInteger(data.ColoniesBuildFighters,              config[HostConfiguration::ColoniesBuildFighters]);
    exportInteger(data.RobotsBuildFighters,                config[HostConfiguration::RobotsBuildFighters]);
    exportArrayizedInteger(data.CloakFailureRate,          config[HostConfiguration::CloakFailureRate]);
    exportInteger(data.RobCloakedShips,                    config[HostConfiguration::RobCloakedShips]);
    exportArrayizedInteger(data.ScanRange,                 config[HostConfiguration::ScanRange]);
    exportArrayizedInteger(data.DarkSenseRange,            config[HostConfiguration::DarkSenseRange]);
    exportInteger(data.AllowHiss,                          config[HostConfiguration::AllowHiss]);
    exportInteger(data.AllowRebelGroundAttack,             config[HostConfiguration::AllowRebelGroundAttack]);
    exportInteger(data.AllowSuperRefit,                    config[HostConfiguration::AllowSuperRefit]);
    exportInteger(data.AllowWebMines,                      config[HostConfiguration::AllowWebMines]);
    exportArrayizedInteger(data.CloakFuelBurn,             config[HostConfiguration::CloakFuelBurn]);
    exportArrayizedInteger(data.SensorRange,               config[HostConfiguration::SensorRange]);
    exportInteger(data.AllowNewNatives,                    config[HostConfiguration::AllowNewNatives]);
    exportInteger(data.AllowPlanetAttacks,                 config[HostConfiguration::AllowPlanetAttacks]);
    exportArrayizedInteger(data.BorgAssimilationRate,      config[HostConfiguration::BorgAssimilationRate]);
    exportArrayizedInteger(data.WebMineDecayRate,          config[HostConfiguration::WebMineDecayRate]);
    exportArrayizedInteger(data.MineDecayRate,             config[HostConfiguration::MineDecayRate]);
    exportArrayizedInteger(data.MaximumMinefieldRadius,    config[HostConfiguration::MaximumMinefieldRadius]);
    exportInteger(data.TransuraniumDecayRate,              config[HostConfiguration::TransuraniumDecayRate]);
    exportArrayizedInteger(data.StructureDecayPerTurn,     config[HostConfiguration::StructureDecayPerTurn]);
    exportArrayizedInteger(data.AllowEatingSupplies,       config[HostConfiguration::AllowEatingSupplies]);
    exportInteger(data.AllowNoFuelMovement,                config[HostConfiguration::AllowNoFuelMovement]);
    exportArrayizedInteger(data.MineHitOdds,               config[HostConfiguration::MineHitOdds]);
    exportArrayizedInteger(data.WebMineHitOdds,            config[HostConfiguration::WebMineHitOdds]);
    exportArrayizedInteger(data.MineScanRange,             config[HostConfiguration::MineScanRange]);
    exportInteger(data.AllowMinesDestroyMines,             config[HostConfiguration::AllowMinesDestroyMines]);

    exportInteger(data.AllowEngineShieldBonus,             config[HostConfiguration::AllowEngineShieldBonus]);
    exportArrayizedInteger(data.EngineShieldBonusRate,     config[HostConfiguration::EngineShieldBonusRate]);
    data.ColonialFighterSweepRate = exportRaceArray(config[HostConfiguration::FighterSweepRate], config[HostConfiguration::PlayerRace], 11, 20);
    exportInteger(data.AllowColoniesSweepWebs,             config[HostConfiguration::AllowColoniesSweepWebs]);
    exportArrayizedInteger(data.MineSweepRate,             config[HostConfiguration::MineSweepRate]);
    exportArrayizedInteger(data.WebMineSweepRate,          config[HostConfiguration::WebMineSweepRate]);
    exportArrayizedInteger(data.HissEffectRate,            config[HostConfiguration::HissEffectRate]);
    exportArrayizedInteger(data.RobFailureOdds,            config[HostConfiguration::RobFailureOdds]);
    exportInteger(data.PlanetsAttackRebels,                config[HostConfiguration::PlanetsAttackRebels]);
    exportInteger(data.PlanetsAttackKlingons,              config[HostConfiguration::PlanetsAttackKlingons]);
    exportArrayizedInteger(data.MineSweepRange,            config[HostConfiguration::MineSweepRange]);
    exportArrayizedInteger(data.WebMineSweepRange,         config[HostConfiguration::WebMineSweepRange]);
    exportInteger(data.AllowScienceMissions,               config[HostConfiguration::AllowScienceMissions]);
    exportArrayizedInteger(data.MineHitOddsWhenCloakedX10, config[HostConfiguration::MineHitOddsWhenCloakedX10]);
    exportInteger(data.DamageLevelForCloakFail,            config[HostConfiguration::DamageLevelForCloakFail]);
    exportInteger(data.AllowFedCombatBonus,                config[HostConfiguration::AllowFedCombatBonus]);
    exportInteger(data.MeteorShowerOdds,                   config[HostConfiguration::MeteorShowerOdds]);
    exportArray32(data.MeteorShowerOreRanges,              config[HostConfiguration::MeteorShowerOreRanges]);
    exportInteger(data.LargeMeteorsImpacting,              config[HostConfiguration::LargeMeteorsImpacting]);
    exportArray32(data.LargeMeteorOreRanges,               config[HostConfiguration::LargeMeteorOreRanges]);
    exportInteger(data.AllowMeteorMessages,                config[HostConfiguration::AllowMeteorMessages]);

    exportInteger(data.AllowOneEngineTowing,               config[HostConfiguration::AllowOneEngineTowing]);
    exportInteger(data.AllowHyperWarps,                    config[HostConfiguration::AllowHyperWarps]);
    exportArrayizedInteger(data.ClimateDeathRate,          config[HostConfiguration::ClimateDeathRate]);
    exportInteger(data.AllowGravityWells,                  config[HostConfiguration::AllowGravityWells]);
    exportInteger(data.CrystalsPreferDeserts,              config[HostConfiguration::CrystalsPreferDeserts]);

    exportInteger(data.AllowMinesDestroyWebs,              config[HostConfiguration::AllowMinesDestroyWebs]);
    exportInteger(data.ClimateLimitsPopulation,            config[HostConfiguration::ClimateLimitsPopulation]);

    data.MaxPlanetaryIncome =                              config[HostConfiguration::MaxPlanetaryIncome](DEFAULT_RACE);
    exportInteger(data.IonStormActivity,                   config[HostConfiguration::IonStormActivity]);
    exportInteger(data.AllowChunneling,                    config[HostConfiguration::AllowChunneling]);
    exportInteger(data.AllowDeluxeSuperSpy,                config[HostConfiguration::AllowDeluxeSuperSpy]);
    exportInteger(data.IonStormsHideMines,                 config[HostConfiguration::IonStormsHideMines]);
    exportInteger(data.AllowGloryDevice,                   config[HostConfiguration::AllowGloryDevice]);
    exportInteger(data.AllowAntiCloakShips,                config[HostConfiguration::AllowAntiCloakShips]);
    exportInteger(data.AllowGamblingShips,                 config[HostConfiguration::AllowGamblingShips]);
    exportInteger(data.AllowCloakedShipsAttack,            config[HostConfiguration::AllowCloakedShipsAttack]);
    exportInteger(data.AllowShipCloning,                   config[HostConfiguration::AllowShipCloning]);
    exportInteger(data.AllowBoardingParties,               config[HostConfiguration::AllowBoardingParties]);
    exportInteger(data.AllowImperialAssault,               config[HostConfiguration::AllowImperialAssault]);

    exportInteger(data.RamScoopFuelPerLY,                  config[HostConfiguration::RamScoopFuelPerLY]);
    exportInteger(data.AllowAdvancedRefinery,              config[HostConfiguration::AllowAdvancedRefinery]);
    exportInteger(data.AllowBioscanners,                   config[HostConfiguration::AllowBioscanners]);
    exportInteger(data.HullTechNotSlowedByMines,           config[HostConfiguration::HullTechNotSlowedByMines]);
    data.LokiDecloaksBirds = !exportRaceArray(config[HostConfiguration::AntiCloakImmunity], config[HostConfiguration::PlayerRace], 3, 1);
    exportInteger(data.AllowVPAFeatures,                   config[HostConfiguration::AllowVPAFeatures]);
}
