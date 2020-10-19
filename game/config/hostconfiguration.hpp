/**
  *  \file game/config/hostconfiguration.hpp
  *  \brief Class game::config::HostConfiguration
  */
#ifndef C2NG_GAME_CONFIG_HOSTCONFIGURATION_HPP
#define C2NG_GAME_CONFIG_HOSTCONFIGURATION_HPP

#include "afl/string/translator.hpp"
#include "game/config/aliasoption.hpp"
#include "game/config/collapsibleintegerarrayoption.hpp"
#include "game/config/configuration.hpp"
#include "game/config/costarrayoption.hpp"
#include "game/config/integerarrayoption.hpp"
#include "game/config/integeroption.hpp"
#include "game/config/stringoption.hpp"
#include "game/limits.hpp"
#include "game/playerset.hpp"

namespace game { namespace config {

    /** Host Configuration.
        Represents a superset of pconfig.src and HConfig.
        All options from these sources can be stored.
        Array options are represented as arrays.
        Some options that are not originally arrays are represented as arrays here.
        (From HConfig point-of-view, this applies to all options array-ized in PHost.)

        It is derived from Configuration and can therefore store arbitrary key/value mappings:
        mappings to known types preserve the types (i.e. PlayerRace is an array of integers, NumMinefields is an integer). */
    class HostConfiguration : public Configuration {
     public:
        /** Standard option: an option that is indexed by a player number
            but can be stored as a single scalar if per-player setting is not used. */
        typedef CollapsibleIntegerArrayOption<MAX_PLAYERS> StandardOption_t;
        typedef CollapsibleIntegerArrayOptionDescriptor<MAX_PLAYERS> StandardOptionDescriptor_t;

        /** Experience option: an option that is indexed by an experience level. */
        typedef IntegerArrayOption<MAX_EXPERIENCE_LEVELS> ExperienceOption_t;
        typedef IntegerArrayOptionDescriptor<MAX_EXPERIENCE_LEVELS> ExperienceOptionDescriptor_t;

        /*
         *  Known Configuration Keys
         *
         *  The names correspond to pconfig.src keys, if applicable.
         */

        static const IntegerOptionDescriptor              ConfigLevel;
        static const StandardOptionDescriptor_t           PlayerRace;
        static const StandardOptionDescriptor_t           RecycleRate;
        static const IntegerOptionDescriptor              RandomMeteorRate;
        static const IntegerOptionDescriptor              AllowMinefields;
        static const IntegerOptionDescriptor              AllowAlchemy;
        static const IntegerOptionDescriptor              DeleteOldMessages;
        static const IntegerOptionDescriptor              DisablePasswords;
        static const StandardOptionDescriptor_t           GroundKillFactor;
        static const StandardOptionDescriptor_t           GroundDefenseFactor;
        static const StandardOptionDescriptor_t           FreeFighters;
        static const StandardOptionDescriptor_t           RaceMiningRate;
        static const StandardOptionDescriptor_t           AllowBuildFighters;
        static const IntegerOptionDescriptor              RebelsBuildFighters;
        static const IntegerOptionDescriptor              ColoniesBuildFighters;
        static const IntegerOptionDescriptor              RobotsBuildFighters;
        static const StandardOptionDescriptor_t           CloakFailureRate;
        static const IntegerOptionDescriptor              RobCloakedShips;
        static const StandardOptionDescriptor_t           ScanRange;
        static const StandardOptionDescriptor_t           SensorRange;
        static const StandardOptionDescriptor_t           DarkSenseRange;
        static const IntegerOptionDescriptor              AllowHiss;
        static const IntegerOptionDescriptor              AllowRebelGroundAttack;
        static const IntegerOptionDescriptor              AllowSuperRefit;
        static const IntegerOptionDescriptor              AllowWebMines;
        static const StandardOptionDescriptor_t           CloakFuelBurn;
        static const IntegerOptionDescriptor              AllowNewNatives;
        static const IntegerOptionDescriptor              AllowPlanetAttacks;
        static const StandardOptionDescriptor_t           BorgAssimilationRate;
        static const StandardOptionDescriptor_t           MineDecayRate;
        static const StandardOptionDescriptor_t           WebMineDecayRate;
        static const StandardOptionDescriptor_t           MaximumMinefieldRadius;
        static const StandardOptionDescriptor_t           MaximumWebMinefieldRadius;
        static const IntegerOptionDescriptor              TransuraniumDecayRate;
        static const StandardOptionDescriptor_t           StructureDecayPerTurn;
        static const IntegerOptionDescriptor              StructureDecayOnUnowned;
        static const IntegerOptionDescriptor              ClimateLimitsPopulation;
        static const StandardOptionDescriptor_t           AllowEatingSupplies;
        static const IntegerOptionDescriptor              AllowNoFuelMovement;
        static const StandardOptionDescriptor_t           MineHitOdds;
        static const StandardOptionDescriptor_t           WebMineHitOdds;
        static const StandardOptionDescriptor_t           MineScanRange;
        static const IntegerOptionDescriptor              AllowMinesDestroyMines;
        static const IntegerOptionDescriptor              AllowEngineShieldBonus;
        static const StandardOptionDescriptor_t           EngineShieldBonusRate;
        static const StandardOptionDescriptor_t           FighterSweepRate;
        static const IntegerOptionDescriptor              AllowColoniesSweepWebs;
        static const StandardOptionDescriptor_t           MineSweepRate;
        static const StandardOptionDescriptor_t           WebMineSweepRate;
        static const StandardOptionDescriptor_t           HissEffectRate;
        static const StandardOptionDescriptor_t           RobFailureOdds;
        static const IntegerOptionDescriptor              PlanetsAttackRebels;
        static const IntegerOptionDescriptor              PlanetsAttackKlingons;
        static const StandardOptionDescriptor_t           MineSweepRange;
        static const StandardOptionDescriptor_t           WebMineSweepRange;
        static const IntegerOptionDescriptor              AllowScienceMissions;
        static const StandardOptionDescriptor_t           MineHitOddsWhenCloakedX10;
        static const IntegerOptionDescriptor              DamageLevelForCloakFail;
        static const IntegerOptionDescriptor              AllowFedCombatBonus;
        static const IntegerOptionDescriptor              MeteorShowerOdds;
        static const IntegerArrayOptionDescriptor<8>      MeteorShowerOreRanges;
        static const IntegerOptionDescriptor              LargeMeteorsImpacting;
        static const IntegerArrayOptionDescriptor<8>      LargeMeteorOreRanges;
        static const IntegerOptionDescriptor              AllowMeteorMessages;
        static const IntegerOptionDescriptor              AllowOneEngineTowing;
        static const IntegerOptionDescriptor              AllowHyperWarps;
        static const StandardOptionDescriptor_t           ClimateDeathRate;
        static const IntegerOptionDescriptor              AllowGravityWells;
        static const IntegerOptionDescriptor              CrystalsPreferDeserts;
        static const IntegerOptionDescriptor              AllowMinesDestroyWebs;
        static const StandardOptionDescriptor_t           MaxPlanetaryIncome;
        static const IntegerOptionDescriptor              IonStormActivity;
        static const IntegerOptionDescriptor              AllowChunneling;
        static const IntegerOptionDescriptor              AllowDeluxeSuperSpy;
        static const IntegerOptionDescriptor              IonStormsHideMines;
        static const IntegerOptionDescriptor              AllowGloryDevice;
        static const IntegerOptionDescriptor              AllowAntiCloakShips;
        static const IntegerOptionDescriptor              AllowGamblingShips;
        static const IntegerOptionDescriptor              AllowCloakedShipsAttack;
        static const IntegerOptionDescriptor              AllowShipCloning;
        static const IntegerOptionDescriptor              AllowBoardingParties;
        static const IntegerOptionDescriptor              AllowImperialAssault;
        static const IntegerOptionDescriptor              RamScoopFuelPerLY;
        static const IntegerOptionDescriptor              AllowAdvancedRefinery;
        static const IntegerOptionDescriptor              AllowBioscanners;
        static const IntegerOptionDescriptor              HullTechNotSlowedByMines;
        static const IntegerOptionDescriptor              UseAccurateFuelModel;
        static const IntegerOptionDescriptor              DefenseForUndetectable;
        static const IntegerOptionDescriptor              DefenseToBlockBioscan;
        static const IntegerOptionDescriptor              FactoriesForDetectable;
        static const IntegerOptionDescriptor              MinesForDetectable;
        static const StandardOptionDescriptor_t           FighterSweepRange;
        static const IntegerOptionDescriptor              MineHitDamageFor100KT;
        static const IntegerOptionDescriptor              WebHitDamageFor100KT;
        static const IntegerOptionDescriptor              AllowRegisteredFunctions;
        static const IntegerOptionDescriptor              GravityWellRange;
        static const StringOptionDescriptor               Language;
        static const IntegerOptionDescriptor              AllowPlayerMessages;
        static const StringOptionDescriptor               ScoringMethod;
        static const IntegerOptionDescriptor              TowedShipsBreakFree;
        static const IntegerOptionDescriptor              NativeClimateDeathRate;
        static const StandardOptionDescriptor_t           AllowMoreThan50Targets;
        static const IntegerOptionDescriptor              CrystalSinTempBehavior;
        static const IntegerOptionDescriptor              RGANeedsBeams;
        static const IntegerOptionDescriptor              AllowRGAOnUnowned;
        static const IntegerOptionDescriptor              CPEnableLanguage;
        static const IntegerOptionDescriptor              CPEnableBigTargets;
        static const IntegerOptionDescriptor              CPEnableRaceName;
        static const IntegerOptionDescriptor              CPEnableAllies;
        static const IntegerOptionDescriptor              CPEnableMessage;
        static const IntegerOptionDescriptor              AllowAnonymousMessages;
        static const IntegerOptionDescriptor              DelayAllianceCommands;
        static const StandardOptionDescriptor_t           TerraformRate;
        static const IntegerOptionDescriptor              MaxColTempSlope;
        static const IntegerOptionDescriptor              WebDrainFuelLoss;
        static const IntegerOptionDescriptor              WebHitFuelLoss;
        static const IntegerOptionDescriptor              AllowWormholes;
        static const IntegerOptionDescriptor              WrmDisplacement;
        static const IntegerOptionDescriptor              WrmRandDisplacement;
        static const IntegerOptionDescriptor              WrmStabilityAddX10;
        static const IntegerOptionDescriptor              WrmRandStability;
        static const IntegerOptionDescriptor              WrmMassAdd;
        static const IntegerOptionDescriptor              WrmRandMass;
        static const IntegerOptionDescriptor              WrmVoluntaryTravel;
        static const IntegerOptionDescriptor              WrmTravelDistDivisor;
        static const IntegerOptionDescriptor              WrmTravelWarpSpeed;
        static const IntegerOptionDescriptor              WrmTravelCloaked;
        static const IntegerOptionDescriptor              WrmEntryPowerX100;
        static const IntegerOptionDescriptor              CPEnableGive;
        static const IntegerOptionDescriptor              AllowTowCloakedShips;
        static const IntegerOptionDescriptor              RobCloakedChance;
        static const StandardOptionDescriptor_t           UnitsPerTorpRate;
        static const StandardOptionDescriptor_t           UnitsPerWebRate;
        static const IntegerOptionDescriptor              AllowESBonusAgainstPlanets;
        static const StandardOptionDescriptor_t           ShipCloneCostRate;
        static const IntegerOptionDescriptor              AllowHyperjumpGravWells;
        static const IntegerOptionDescriptor              NativeCombatSurvivalRate;
        static const IntegerOptionDescriptor              AllowPrivateerTowCapture;
        static const IntegerOptionDescriptor              AllowCrystalTowCapture;
        static const IntegerOptionDescriptor              RoundGravityWells;
        static const IntegerOptionDescriptor              CPEnableSend;
        static const IntegerOptionDescriptor              CumulativePillaging;
        static const IntegerOptionDescriptor              AllowInterceptAttack;
        static const StandardOptionDescriptor_t           RaceGrowthRate;
        static const StandardOptionDescriptor_t           ProductionRate;
        static const StandardOptionDescriptor_t           MineOddsWarpBonusX100;
        static const StandardOptionDescriptor_t           CloakMineOddsWarpBonusX100;
        static const StandardOptionDescriptor_t           WebMineOddsWarpBonusX100;
        static const StandardOptionDescriptor_t           MineTravelSafeWarp;
        static const StandardOptionDescriptor_t           CloakedMineTravelSafeWarp;
        static const StandardOptionDescriptor_t           WebMineTravelSafeWarp;
        static const IntegerOptionDescriptor              AllowCloakFailMessages;
        static const IntegerOptionDescriptor              TonsScoreCountsPlanets;
        static const IntegerOptionDescriptor              AllowExtendedMissions;
        static const IntegerOptionDescriptor              ExtMissionsStartAt;
        static const IntegerOptionDescriptor              WormholeUFOsStartAt;
        static const IntegerOptionDescriptor              MaxShipsHissing;
        static const IntegerOptionDescriptor              SpyDetectionChance;
        static const IntegerOptionDescriptor              MapTruehullByPlayerRace;
        static const IntegerOptionDescriptor              AllowWraparoundMap;
        static const IntegerArrayOptionDescriptor<4>      WraparoundRectangle;
        static const IntegerOptionDescriptor              CPEnableRemote;
        static const IntegerOptionDescriptor              AllowAlliedChunneling;
        static const StandardOptionDescriptor_t           ColonistTaxRate;
        static const StandardOptionDescriptor_t           NativeTaxRate;
        static const IntegerOptionDescriptor              AllowAlternativeTowing;
        static const IntegerOptionDescriptor              AllowBeamUpClans;
        static const IntegerOptionDescriptor              AllowBeamUpMultiple;
        static const IntegerOptionDescriptor              DamageLevelForTerraformFail;
        static const IntegerOptionDescriptor              DamageLevelForAntiCloakFail;
        static const IntegerOptionDescriptor              DamageLevelForChunnelFail;
        static const IntegerOptionDescriptor              DamageLevelForHyperjumpFail;
        static const IntegerOptionDescriptor              MinimumChunnelDistance;
        static const StandardOptionDescriptor_t           TowStrengthEngineScale;
        static const StandardOptionDescriptor_t           TowStrengthDistanceScale;
        static const IntegerOptionDescriptor              AllowPriorityBuild;
        static const StandardOptionDescriptor_t           SBQBuildPALBoost;
        static const StandardOptionDescriptor_t           SBQNewBuildPALBoost;
        static const StandardOptionDescriptor_t           SBQPointsForAging;
        static const StandardOptionDescriptor_t           SBQBuildChangePenalty;
        static const StandardOptionDescriptor_t           SBQBoostExpX100;
        static const StandardOptionDescriptor_t           BuildChangeRelativePenalty;
        static const StandardOptionDescriptor_t           PALDecayPerTurn;
        static const StandardOptionDescriptor_t           PALPlayerRate;
        static const StandardOptionDescriptor_t           PALCombatAggressor;
        static const StandardOptionDescriptor_t           PALAggressorPointsPer10KT;
        static const StandardOptionDescriptor_t           PALOpponentPointsPer10KT;
        static const StandardOptionDescriptor_t           PALAggressorKillPointsPer10KT;
        static const StandardOptionDescriptor_t           PALOpponentKillPointsPer10KT;
        static const StandardOptionDescriptor_t           PALCombatPlanetScaling;
        static const StandardOptionDescriptor_t           PALCombatBaseScaling;
        static const StandardOptionDescriptor_t           PALShipCapturePer10Crew;
        static const StandardOptionDescriptor_t           PALRecyclingPer10KT;
        static const StandardOptionDescriptor_t           PALBoardingPartyPer10Crew;
        static const StandardOptionDescriptor_t           PALGroundAttackPer100Clans;
        static const StandardOptionDescriptor_t           PALGloryDevice;
        static const StandardOptionDescriptor_t           PALGloryDevicePer10KT;
        static const StandardOptionDescriptor_t           PALGloryDamagePer10KT;
        static const StandardOptionDescriptor_t           PALGloryKillPer10KT;
        static const StandardOptionDescriptor_t           PALImperialAssault;
        static const StandardOptionDescriptor_t           PALRGA;
        static const StandardOptionDescriptor_t           PALPillage;
        static const StandardOptionDescriptor_t           PALIncludesESB;
        static const StandardOptionDescriptor_t           FilterPlayerMessages;
        static const IntegerOptionDescriptor              AlternativeAntiCloak;
        static const StandardOptionDescriptor_t           AntiCloakImmunity;
        static const StandardOptionDescriptor_t           AllowMoreThan500Minefields;
        static const IntegerOptionDescriptor              NumMinefields;
        static const StandardOptionDescriptor_t           PALShipMinekillPer10KT;
        static const StandardOptionDescriptor_t           MaximumMinefieldsPerPlayer;
        static const IntegerOptionDescriptor              MineIdNeedsPermission;
        static const StringOptionDescriptor               BuildQueue;
        static const StandardOptionDescriptor_t           PBPCostPer100KT;
        static const StandardOptionDescriptor_t           PBPMinimumCost;
        static const StandardOptionDescriptor_t           PBPCloneCostRate;
        static const IntegerOptionDescriptor              AllowShipNames;
        static const IntegerOptionDescriptor              BuildPointReport;
        static const IntegerOptionDescriptor              AlternativeMinesDestroyMines;
        static const IntegerOptionDescriptor              NumShips;
        static const IntegerOptionDescriptor              ExtendedSensorSweep;
        static const StandardOptionDescriptor_t           ColonistCombatSurvivalRate;
        static const IntegerOptionDescriptor              NewNativesPerTurn;
        static const IntegerArrayOptionDescriptor<2>      NewNativesPopulationRange;
        static const IntegerArrayOptionDescriptor<9>      NewNativesRaceRate;
        static const IntegerArrayOptionDescriptor<9>      NewNativesGovernmentRate;
        static const StandardOptionDescriptor_t           PlayerSpecialMission;
        static const IntegerOptionDescriptor              TowedShipsCooperate;
        static const StandardOptionDescriptor_t           WrmScanRange;
        static const StandardOptionDescriptor_t           FuelUsagePerFightFor100KT;
        static const StandardOptionDescriptor_t           FuelUsagePerTurnFor100KT;
        static const IntegerOptionDescriptor              CPEnableEnemies;
        static const IntegerOptionDescriptor              CPEnableShow;
        static const IntegerOptionDescriptor              CPEnableRefit;
        static const IntegerOptionDescriptor              AllowIncompatibleConfiguration;
        static const CostArrayOptionDescriptor            FreeFighterCost;
        static const CostArrayOptionDescriptor            StarbaseCost;
        static const CostArrayOptionDescriptor            BaseFighterCost;
        static const CostArrayOptionDescriptor            ShipFighterCost;
        static const StandardOptionDescriptor_t           MaximumFightersOnBase;
        static const StandardOptionDescriptor_t           MaximumDefenseOnBase;
        static const IntegerOptionDescriptor              NumExperienceLevels;
        static const ExperienceOptionDescriptor_t         ExperienceLevels;
        static const StringOptionDescriptor               ExperienceLevelNames;
        static const IntegerOptionDescriptor              ExperienceLimit;
        static const StandardOptionDescriptor_t           EPRecrewScaling;
        static const IntegerOptionDescriptor              EPShipAging;
        static const IntegerOptionDescriptor              EPPlanetAging;
        static const IntegerOptionDescriptor              EPPlanetGovernment;
        static const IntegerOptionDescriptor              EPShipMovement100LY;
        static const IntegerOptionDescriptor              EPShipHyperjump;
        static const IntegerOptionDescriptor              EPShipChunnel;
        static const IntegerOptionDescriptor              EPShipIonStorm100MEV;
        static const IntegerOptionDescriptor              EPCombatKillScaling;
        static const IntegerOptionDescriptor              EPCombatDamageScaling;
        static const IntegerOptionDescriptor              EPShipAlchemy100KT;
        static const ExperienceOptionDescriptor_t         EPCombatBoostRate;
        static const ExperienceOptionDescriptor_t         EPCombatBoostLevel;
        static const StandardOptionDescriptor_t           EPTrainingScale;
        static const IntegerOptionDescriptor              ExactExperienceReports;
        static const ExperienceOptionDescriptor_t         EModBayRechargeRate;
        static const ExperienceOptionDescriptor_t         EModBayRechargeBonus;
        static const ExperienceOptionDescriptor_t         EModBeamRechargeRate;
        static const ExperienceOptionDescriptor_t         EModBeamRechargeBonus;
        static const ExperienceOptionDescriptor_t         EModTubeRechargeRate;
        static const ExperienceOptionDescriptor_t         EModBeamHitFighterCharge;
        static const ExperienceOptionDescriptor_t         EModTorpHitOdds;
        static const ExperienceOptionDescriptor_t         EModBeamHitOdds;
        static const ExperienceOptionDescriptor_t         EModBeamHitBonus;
        static const ExperienceOptionDescriptor_t         EModStrikesPerFighter;
        static const ExperienceOptionDescriptor_t         EModFighterBeamExplosive;
        static const ExperienceOptionDescriptor_t         EModFighterBeamKill;
        static const ExperienceOptionDescriptor_t         EModFighterMovementSpeed;
        static const ExperienceOptionDescriptor_t         EModMaxFightersLaunched;
        static const ExperienceOptionDescriptor_t         EModTorpHitBonus;
        static const ExperienceOptionDescriptor_t         EModTubeRechargeBonus;
        static const ExperienceOptionDescriptor_t         EModExtraFighterBays;
        static const ExperienceOptionDescriptor_t         EModEngineShieldBonusRate;
        static const ExperienceOptionDescriptor_t         EModShieldDamageScaling;
        static const ExperienceOptionDescriptor_t         EModShieldKillScaling;
        static const ExperienceOptionDescriptor_t         EModHullDamageScaling;
        static const ExperienceOptionDescriptor_t         EModCrewKillScaling;
        static const ExperienceOptionDescriptor_t         EModPlanetaryTorpsPerTube;
        static const ExperienceOptionDescriptor_t         EModMineHitOddsBonus;
        static const StringOptionDescriptor               GameName;
        static const StandardOptionDescriptor_t           BayRechargeRate;
        static const StandardOptionDescriptor_t           BayRechargeBonus;
        static const StandardOptionDescriptor_t           BeamRechargeRate;
        static const StandardOptionDescriptor_t           BeamRechargeBonus;
        static const StandardOptionDescriptor_t           TubeRechargeRate;
        static const StandardOptionDescriptor_t           BeamHitFighterCharge;
        static const StandardOptionDescriptor_t           BeamHitShipCharge;
        static const StandardOptionDescriptor_t           TorpFiringRange;
        static const StandardOptionDescriptor_t           BeamFiringRange;
        static const StandardOptionDescriptor_t           TorpHitOdds;
        static const StandardOptionDescriptor_t           BeamHitOdds;
        static const StandardOptionDescriptor_t           BeamHitBonus;
        static const StandardOptionDescriptor_t           StrikesPerFighter;
        static const StandardOptionDescriptor_t           FighterKillOdds;
        static const StandardOptionDescriptor_t           FighterBeamExplosive;
        static const StandardOptionDescriptor_t           FighterBeamKill;
        static const StandardOptionDescriptor_t           ShipMovementSpeed;
        static const StandardOptionDescriptor_t           FighterMovementSpeed;
        static const StandardOptionDescriptor_t           BayLaunchInterval;
        static const StandardOptionDescriptor_t           MaxFightersLaunched;
        static const IntegerOptionDescriptor              AllowAlternativeCombat;
        static const IntegerOptionDescriptor              StandoffDistance;
        static const IntegerOptionDescriptor              PlanetsHaveTubes;
        static const StandardOptionDescriptor_t           PlanetaryTorpsPerTube;
        static const IntegerOptionDescriptor              FireOnAttackFighters;
        static const StandardOptionDescriptor_t           TorpHitBonus;
        static const StandardOptionDescriptor_t           TubeRechargeBonus;
        static const StandardOptionDescriptor_t           ShieldDamageScaling;
        static const StandardOptionDescriptor_t           HullDamageScaling;
        static const StandardOptionDescriptor_t           CrewKillScaling;
        static const StandardOptionDescriptor_t           ShieldKillScaling;
        static const StandardOptionDescriptor_t           ExtraFighterBays;
        static const StandardOptionDescriptor_t           BeamHitFighterRange;
        static const StandardOptionDescriptor_t           FighterFiringRange;
        static const IntegerOptionDescriptor              AllowVPAFeatures;
        static const IntegerOptionDescriptor              MinimumHappiness;
        static const StandardOptionDescriptor_t           ColonistCombatCaptureRate;
        static const StandardOptionDescriptor_t           EPAcademyScale;
        static const StandardOptionDescriptor_t           UseBaseTorpsInCombat;
        static const StandardOptionDescriptor_t           BaseTechCost;
        static const StandardOptionDescriptor_t           EPShipBuild1000TorpUnits;
        static const StandardOptionDescriptor_t           EPShipBuild10Fighters;
        static const AliasOptionDescriptor                CPEnableRumor;
        static const AliasOptionDescriptor                RaceTaxRate;
        static const AliasOptionDescriptor                CPNumMinefields;
        static const AliasOptionDescriptor                NativeClansRange;
        static const AliasOptionDescriptor                NativeTypeFrequencies;
        static const AliasOptionDescriptor                NativeGovFrequencies;


        /** Default constructor.
            Makes a configuration containing all values at defaults (see setDefaultValues()). */
        HostConfiguration();

        /** Assign default values to all options.
            This populates the object with all configuration values, and gives them sensible default values. */
        void setDefaultValues();

        /** Assign dependant options.
            If an option has the default "same as other option", and has not been explicitly set in the config file,
            copy its value from the other option. */
        void setDependantOptions();

        /** Get player race number.
            \param player player number (normally [1,MAX_PLAYERS], but out-of-range values are handled)
            \return race number */
        int32_t getPlayerRaceNumber(int player) const;

        /** Get player mission number.
            \param player player number (normally [1,MAX_PLAYERS], but out-of-range values are handled)
            \return mission number */
        int32_t getPlayerMissionNumber(int player) const;

        /** Get experience level name.
            \param level level number (normally [0,NUM_EXPERIENCE_LEVELS], but out-of-range values are handled) */
        String_t getExperienceLevelName(int level, afl::string::Translator& tx) const;

        /** Get experience bonus.
            \param param descriptor of option to test
            \param level experience level (normally [0,MAX_EXPERIENCE_LEVELS], but out-of-range values are handled)
            \return Value */
        int32_t getExperienceBonus(const ExperienceOptionDescriptor_t& option, int level) const;

        /** Get set of all players of a particular race.
            \param race race to query */
        PlayerSet_t getPlayersOfRace(int race) const;

        /** Get set of all players where an option is enabled.
            \param opt boolean option descriptor */
        PlayerSet_t getPlayersWhereEnabled(const StandardOptionDescriptor_t& opt) const;

        /** Get set of all players where an option has a given scalar value.
            \param opt descriptor of option to query
            \param value Value to check for */
        PlayerSet_t getPlayersWhere(const StandardOptionDescriptor_t& opt, int value) const;

        /** Get set of all players where an option has a given cost value.
            \param opt descriptor of option to query
            \param value Value to check for */
        PlayerSet_t getPlayersWhere(const CostArrayOptionDescriptor& opt, const game::spec::Cost& value) const;
    };

} }

#endif
