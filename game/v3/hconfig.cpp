/**
  *  \file game/v3/hconfig.cpp
  *  \brief HConfig Access Functions
  */

#include "game/v3/hconfig.hpp"

namespace gt = game::v3::structures;

namespace {
    /*
     *  Import (image -> internal)
     */

    /** Import 11 WORDs from HCONFIG image (per-player settings). */
    void importArray16(game::config::HostConfiguration::StandardOption_t& option, const gt::Int16_t (&image)[12])
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

    
    /*
     *  Export (internal -> image)
     */

    const int DEFAULT_RACE = 3;

    /** Export arrayized integer.
        Internally, we store the array. HCONFIG contains just one element, so arbitrarily pick one (DEFAULT_RACE). */
    void exportArrayizedInteger(gt::Int16_t& out, const game::config::HostConfiguration::StandardOption_t& in)
    {
        out = static_cast<int16_t>(in(DEFAULT_RACE));
    }

    /** Export single 16-bit integer. */
    void exportInteger(gt::Int16_t& out, const game::config::IntegerOption& in)
    {
        out = static_cast<int16_t>(in());
    }

    /** Export 11-element array. This also initializes the unused 12th element.  */
    void exportArray16(gt::Int16_t (&image)[12], const game::config::HostConfiguration::StandardOption_t& option)
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
        config[config.RecycleRate].set(data.RecycleRate); config[config.RecycleRate].setSource(source);
        config[config.RandomMeteorRate].set(data.RandomMeteorRate); config[config.RandomMeteorRate].setSource(source);
        config[config.AllowMinefields].set(data.AllowMinefields); config[config.AllowMinefields].setSource(source);
        config[config.AllowAlchemy].set(data.AllowAlchemy); config[config.AllowAlchemy].setSource(source);
        config[config.DeleteOldMessages].set(data.DeleteOldMessages); config[config.DeleteOldMessages].setSource(source);
    }
    if (size >= 186) {
        config[config.DisablePasswords].set(data.DisablePasswords); config[config.DisablePasswords].setSource(source);
        importArray16(config[config.GroundKillFactor], data.GroundKillFactor); config[config.GroundKillFactor].setSource(source);
        importArray16(config[config.GroundDefenseFactor], data.GroundDefenseFactor); config[config.GroundDefenseFactor].setSource(source);
        importArray16(config[config.FreeFighters], data.FreeFighters); config[config.FreeFighters].setSource(source);
        importArray16(config[config.RaceMiningRate], data.RaceMiningRate); config[config.RaceMiningRate].setSource(source);
        importArray16(config[config.ColonistTaxRate], data.ColonistTaxRate); config[config.ColonistTaxRate].setSource(source);
        config[config.RebelsBuildFighters].set(data.RebelsBuildFighters); config[config.RebelsBuildFighters].setSource(source);
        config[config.ColoniesBuildFighters].set(data.ColoniesBuildFighters); config[config.ColoniesBuildFighters].setSource(source);
        config[config.RobotsBuildFighters].set(data.RobotsBuildFighters); config[config.RobotsBuildFighters].setSource(source);
        config[config.CloakFailureRate].set(data.CloakFailureRate); config[config.CloakFailureRate].setSource(source);
        config[config.RobCloakedShips].set(data.RobCloakedShips); config[config.RobCloakedShips].setSource(source);
        config[config.ScanRange].set(data.ScanRange); config[config.ScanRange].setSource(source);
        config[config.DarkSenseRange].set(data.DarkSenseRange); config[config.DarkSenseRange].setSource(source);
        config[config.AllowHiss].set(data.AllowHiss); config[config.AllowHiss].setSource(source);
        config[config.AllowRebelGroundAttack].set(data.AllowRebelGroundAttack); config[config.AllowRebelGroundAttack].setSource(source);
        config[config.AllowSuperRefit].set(data.AllowSuperRefit); config[config.AllowSuperRefit].setSource(source);
        config[config.AllowWebMines].set(data.AllowWebMines); config[config.AllowWebMines].setSource(source);
        config[config.CloakFuelBurn].set(data.CloakFuelBurn); config[config.CloakFuelBurn].setSource(source);
        config[config.SensorRange].set(data.SensorRange); config[config.SensorRange].setSource(source);
        config[config.AllowNewNatives].set(data.AllowNewNatives); config[config.AllowNewNatives].setSource(source);
        config[config.AllowPlanetAttacks].set(data.AllowPlanetAttacks); config[config.AllowPlanetAttacks].setSource(source);
        config[config.BorgAssimilationRate].set(data.BorgAssimilationRate); config[config.BorgAssimilationRate].setSource(source);
        config[config.WebMineDecayRate].set(data.WebMineDecayRate); config[config.WebMineDecayRate].setSource(source);
        config[config.MineDecayRate].set(data.MineDecayRate); config[config.MineDecayRate].setSource(source);
        config[config.MaximumMinefieldRadius].set(data.MaximumMinefieldRadius); config[config.MaximumMinefieldRadius].setSource(source);
        config[config.TransuraniumDecayRate].set(data.TransuraniumDecayRate); config[config.TransuraniumDecayRate].setSource(source);
        config[config.StructureDecayPerTurn].set(data.StructureDecayPerTurn); config[config.StructureDecayPerTurn].setSource(source);
        config[config.AllowEatingSupplies].set(data.AllowEatingSupplies); config[config.AllowEatingSupplies].setSource(source);
        config[config.AllowNoFuelMovement].set(data.AllowNoFuelMovement); config[config.AllowNoFuelMovement].setSource(source);
        config[config.MineHitOdds].set(data.MineHitOdds); config[config.MineHitOdds].setSource(source);
        config[config.WebMineHitOdds].set(data.WebMineHitOdds); config[config.WebMineHitOdds].setSource(source);
        config[config.MineScanRange].set(data.MineScanRange); config[config.MineScanRange].setSource(source);
        config[config.AllowMinesDestroyMines].set(data.AllowMinesDestroyMines); config[config.AllowMinesDestroyMines].setSource(source);
    }
    if (size >= 288) {
        config[config.AllowEngineShieldBonus].set(data.AllowEngineShieldBonus); config[config.AllowEngineShieldBonus].setSource(source);
        config[config.EngineShieldBonusRate].set(data.EngineShieldBonusRate); config[config.EngineShieldBonusRate].setSource(source);
        // FIXME: _ColonialFighterSweepRate
        config[config.AllowColoniesSweepWebs].set(data.AllowColoniesSweepWebs); config[config.AllowColoniesSweepWebs].setSource(source);
        config[config.MineSweepRate].set(data.MineSweepRate); config[config.MineSweepRate].setSource(source);
        config[config.WebMineSweepRate].set(data.WebMineSweepRate); config[config.WebMineSweepRate].setSource(source);
        config[config.HissEffectRate].set(data.HissEffectRate); config[config.HissEffectRate].setSource(source);
        config[config.RobFailureOdds].set(data.RobFailureOdds); config[config.RobFailureOdds].setSource(source);
        config[config.PlanetsAttackRebels].set(data.PlanetsAttackRebels); config[config.PlanetsAttackRebels].setSource(source);
        config[config.PlanetsAttackKlingons].set(data.PlanetsAttackKlingons); config[config.PlanetsAttackKlingons].setSource(source);
        config[config.MineSweepRange].set(data.MineSweepRange); config[config.MineSweepRange].setSource(source);
        config[config.WebMineSweepRange].set(data.WebMineSweepRange); config[config.WebMineSweepRange].setSource(source);
        config[config.AllowScienceMissions].set(data.AllowScienceMissions); config[config.AllowScienceMissions].setSource(source);
        config[config.MineHitOddsWhenCloakedX10].set(data.MineHitOddsWhenCloakedX10); config[config.MineHitOddsWhenCloakedX10].setSource(source);
        config[config.DamageLevelForCloakFail].set(data.DamageLevelForCloakFail); config[config.DamageLevelForCloakFail].setSource(source);
        config[config.AllowFedCombatBonus].set(data.AllowFedCombatBonus); config[config.AllowFedCombatBonus].setSource(source);
        config[config.MeteorShowerOdds].set(data.MeteorShowerOdds); config[config.MeteorShowerOdds].setSource(source);
        importArray32(config[config.MeteorShowerOreRanges], data.MeteorShowerOreRanges); config[config.MeteorShowerOreRanges].setSource(source);
        config[config.LargeMeteorsImpacting].set(data.LargeMeteorsImpacting); config[config.LargeMeteorsImpacting].setSource(source);
        importArray32(config[config.LargeMeteorOreRanges], data.LargeMeteorOreRanges); config[config.LargeMeteorOreRanges].setSource(source);
        config[config.AllowMeteorMessages].set(data.AllowMeteorMessages); config[config.AllowMeteorMessages].setSource(source);
    }
    if (size >= 298) {
        config[config.AllowOneEngineTowing].set(data.AllowOneEngineTowing); config[config.AllowOneEngineTowing].setSource(source);
        config[config.AllowHyperWarps].set(data.AllowHyperWarps); config[config.AllowHyperWarps].setSource(source);
        config[config.ClimateDeathRate].set(data.ClimateDeathRate); config[config.ClimateDeathRate].setSource(source);
        config[config.AllowGravityWells].set(data.AllowGravityWells); config[config.AllowGravityWells].setSource(source);
        config[config.CrystalsPreferDeserts].set(data.CrystalsPreferDeserts); config[config.CrystalsPreferDeserts].setSource(source);
    }
    if (size >= 302) {
        config[config.AllowMinesDestroyWebs].set(data.AllowMinesDestroyWebs); config[config.AllowMinesDestroyWebs].setSource(source);
        config[config.ClimateLimitsPopulation].set(data.ClimateLimitsPopulation); config[config.ClimateLimitsPopulation].setSource(source);
    }
    if (size >= 328) {
        config[config.MaxPlanetaryIncome].set(data.MaxPlanetaryIncome); config[config.MaxPlanetaryIncome].setSource(source);
        config[config.IonStormActivity].set(data.IonStormActivity); config[config.IonStormActivity].setSource(source);
        config[config.AllowChunneling].set(data.AllowChunneling); config[config.AllowChunneling].setSource(source);
        config[config.AllowDeluxeSuperSpy].set(data.AllowDeluxeSuperSpy); config[config.AllowDeluxeSuperSpy].setSource(source);
        config[config.IonStormsHideMines].set(data.IonStormsHideMines); config[config.IonStormsHideMines].setSource(source);
        config[config.AllowGloryDevice].set(data.AllowGloryDevice); config[config.AllowGloryDevice].setSource(source);
        config[config.AllowAntiCloakShips].set(data.AllowAntiCloakShips); config[config.AllowAntiCloakShips].setSource(source);
        config[config.AllowGamblingShips].set(data.AllowGamblingShips); config[config.AllowGamblingShips].setSource(source);
        config[config.AllowCloakedShipsAttack].set(data.AllowCloakedShipsAttack); config[config.AllowCloakedShipsAttack].setSource(source);
        config[config.AllowShipCloning].set(data.AllowShipCloning); config[config.AllowShipCloning].setSource(source);
        config[config.AllowBoardingParties].set(data.AllowBoardingParties); config[config.AllowBoardingParties].setSource(source);
        config[config.AllowImperialAssault].set(data.AllowImperialAssault); config[config.AllowImperialAssault].setSource(source);
    }
    if (size >= 336) {
        config[config.RamScoopFuelPerLY].set(data.RamScoopFuelPerLY); config[config.RamScoopFuelPerLY].setSource(source);
        config[config.AllowAdvancedRefinery].set(data.AllowAdvancedRefinery); config[config.AllowAdvancedRefinery].setSource(source);
        config[config.AllowBioscanners].set(data.AllowBioscanners); config[config.AllowBioscanners].setSource(source);
        config[config.HullTechNotSlowedByMines].set(data.HullTechNotSlowedByMines); config[config.HullTechNotSlowedByMines].setSource(source);
    }
    // FIXME: _LokiDecloaksBirds
    if (size >= 340) {
        config[config.AllowVPAFeatures].set(data.AllowVPAFeatures); config[config.AllowVPAFeatures].setSource(source);
    }
}

// Pack HCONFIG.HST from internal structure.
void
game::v3::packHConfig(structures::HConfig& data, const game::config::HostConfiguration& config)
{
    exportArrayizedInteger(data.RecycleRate,               config[config.RecycleRate]);
    exportInteger(data.RandomMeteorRate,                   config[config.RandomMeteorRate]);
    exportInteger(data.AllowMinefields,                    config[config.AllowMinefields]);
    exportInteger(data.AllowAlchemy,                       config[config.AllowAlchemy]);
    exportInteger(data.DeleteOldMessages,                  config[config.DeleteOldMessages]);

    exportInteger(data.DisablePasswords,                   config[config.DisablePasswords]);
    exportArray16(data.GroundKillFactor,                   config[config.GroundKillFactor]);
    exportArray16(data.GroundDefenseFactor,                config[config.GroundDefenseFactor]);
    exportArray16(data.FreeFighters,                       config[config.FreeFighters]);
    exportArray16(data.RaceMiningRate,                     config[config.RaceMiningRate]);
    exportArray16(data.ColonistTaxRate,                    config[config.ColonistTaxRate]);
    exportInteger(data.RebelsBuildFighters,                config[config.RebelsBuildFighters]);
    exportInteger(data.ColoniesBuildFighters,              config[config.ColoniesBuildFighters]);
    exportInteger(data.RobotsBuildFighters,                config[config.RobotsBuildFighters]);
    exportArrayizedInteger(data.CloakFailureRate,          config[config.CloakFailureRate]);
    exportInteger(data.RobCloakedShips,                    config[config.RobCloakedShips]);
    exportArrayizedInteger(data.ScanRange,                 config[config.ScanRange]);
    exportArrayizedInteger(data.DarkSenseRange,            config[config.DarkSenseRange]);
    exportInteger(data.AllowHiss,                          config[config.AllowHiss]);
    exportInteger(data.AllowRebelGroundAttack,             config[config.AllowRebelGroundAttack]);
    exportInteger(data.AllowSuperRefit,                    config[config.AllowSuperRefit]);
    exportInteger(data.AllowWebMines,                      config[config.AllowWebMines]);
    exportArrayizedInteger(data.CloakFuelBurn,             config[config.CloakFuelBurn]);
    exportArrayizedInteger(data.SensorRange,               config[config.SensorRange]);
    exportInteger(data.AllowNewNatives,                    config[config.AllowNewNatives]);
    exportInteger(data.AllowPlanetAttacks,                 config[config.AllowPlanetAttacks]);
    exportArrayizedInteger(data.BorgAssimilationRate,      config[config.BorgAssimilationRate]);
    exportArrayizedInteger(data.WebMineDecayRate,          config[config.WebMineDecayRate]);
    exportArrayizedInteger(data.MineDecayRate,             config[config.MineDecayRate]);
    exportArrayizedInteger(data.MaximumMinefieldRadius,    config[config.MaximumMinefieldRadius]);
    exportInteger(data.TransuraniumDecayRate,              config[config.TransuraniumDecayRate]);
    exportArrayizedInteger(data.StructureDecayPerTurn,     config[config.StructureDecayPerTurn]);
    exportArrayizedInteger(data.AllowEatingSupplies,       config[config.AllowEatingSupplies]);
    exportInteger(data.AllowNoFuelMovement,                config[config.AllowNoFuelMovement]);
    exportArrayizedInteger(data.MineHitOdds,               config[config.MineHitOdds]);
    exportArrayizedInteger(data.WebMineHitOdds,            config[config.WebMineHitOdds]);
    exportArrayizedInteger(data.MineScanRange,             config[config.MineScanRange]);
    exportInteger(data.AllowMinesDestroyMines,             config[config.AllowMinesDestroyMines]);

    exportInteger(data.AllowEngineShieldBonus,             config[config.AllowEngineShieldBonus]);
    exportArrayizedInteger(data.EngineShieldBonusRate,     config[config.EngineShieldBonusRate]);
    data._ColonialFighterSweepRate = 20;   // FIXME: handle this option
    exportInteger(data.AllowColoniesSweepWebs,             config[config.AllowColoniesSweepWebs]);
    exportArrayizedInteger(data.MineSweepRate,             config[config.MineSweepRate]);
    exportArrayizedInteger(data.WebMineSweepRate,          config[config.WebMineSweepRate]);
    exportArrayizedInteger(data.HissEffectRate,            config[config.HissEffectRate]);
    exportArrayizedInteger(data.RobFailureOdds,            config[config.RobFailureOdds]);
    exportInteger(data.PlanetsAttackRebels,                config[config.PlanetsAttackRebels]);
    exportInteger(data.PlanetsAttackKlingons,              config[config.PlanetsAttackKlingons]);
    exportArrayizedInteger(data.MineSweepRange,            config[config.MineSweepRange]);
    exportArrayizedInteger(data.WebMineSweepRange,         config[config.WebMineSweepRange]);
    exportInteger(data.AllowScienceMissions,               config[config.AllowScienceMissions]);
    exportArrayizedInteger(data.MineHitOddsWhenCloakedX10, config[config.MineHitOddsWhenCloakedX10]);
    exportInteger(data.DamageLevelForCloakFail,            config[config.DamageLevelForCloakFail]);
    exportInteger(data.AllowFedCombatBonus,                config[config.AllowFedCombatBonus]);
    exportInteger(data.MeteorShowerOdds,                   config[config.MeteorShowerOdds]);
    exportArray32(data.MeteorShowerOreRanges,              config[config.MeteorShowerOreRanges]);
    exportInteger(data.LargeMeteorsImpacting,              config[config.LargeMeteorsImpacting]);
    exportArray32(data.LargeMeteorOreRanges,               config[config.LargeMeteorOreRanges]);
    exportInteger(data.AllowMeteorMessages,                config[config.AllowMeteorMessages]);

    exportInteger(data.AllowOneEngineTowing,               config[config.AllowOneEngineTowing]);
    exportInteger(data.AllowHyperWarps,                    config[config.AllowHyperWarps]);
    exportArrayizedInteger(data.ClimateDeathRate,          config[config.ClimateDeathRate]);
    exportInteger(data.AllowGravityWells,                  config[config.AllowGravityWells]);
    exportInteger(data.CrystalsPreferDeserts,              config[config.CrystalsPreferDeserts]);

    exportInteger(data.AllowMinesDestroyWebs,              config[config.AllowMinesDestroyWebs]);
    exportInteger(data.ClimateLimitsPopulation,            config[config.ClimateLimitsPopulation]);

    data.MaxPlanetaryIncome =                              config[config.MaxPlanetaryIncome](DEFAULT_RACE);
    exportInteger(data.IonStormActivity,                   config[config.IonStormActivity]);
    exportInteger(data.AllowChunneling,                    config[config.AllowChunneling]);
    exportInteger(data.AllowDeluxeSuperSpy,                config[config.AllowDeluxeSuperSpy]);
    exportInteger(data.IonStormsHideMines,                 config[config.IonStormsHideMines]);
    exportInteger(data.AllowGloryDevice,                   config[config.AllowGloryDevice]);
    exportInteger(data.AllowAntiCloakShips,                config[config.AllowAntiCloakShips]);
    exportInteger(data.AllowGamblingShips,                 config[config.AllowGamblingShips]);
    exportInteger(data.AllowCloakedShipsAttack,            config[config.AllowCloakedShipsAttack]);
    exportInteger(data.AllowShipCloning,                   config[config.AllowShipCloning]);
    exportInteger(data.AllowBoardingParties,               config[config.AllowBoardingParties]);
    exportInteger(data.AllowImperialAssault,               config[config.AllowImperialAssault]);

    exportInteger(data.RamScoopFuelPerLY,                  config[config.RamScoopFuelPerLY]);
    exportInteger(data.AllowAdvancedRefinery,              config[config.AllowAdvancedRefinery]);
    exportInteger(data.AllowBioscanners,                   config[config.AllowBioscanners]);
    exportInteger(data.HullTechNotSlowedByMines,           config[config.HullTechNotSlowedByMines]);
    data._LokiDecloaksBirds = 0; // FIXME: handle this option
    exportInteger(data.AllowVPAFeatures,                   config[config.AllowVPAFeatures]);
}
