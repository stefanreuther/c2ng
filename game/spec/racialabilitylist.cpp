/**
  *  \file game/spec/racialabilitylist.cpp
  *  \brief Class game::spec::RacialAbilityList
  *
  *  As of 20200430, this is an initial draft. Potential evolution:
  *  - FIXME: how do we give names to most options without breaking the translations?
  *  - FIXME: more pictures
  *  - FIXME: name the increased/decreased parameters to tell whether more or less is better
  *  - FIXME: more categories, e.g. Ship>Missions, Minefields>Laying, Minefields>Sweeping, etc.
  *
  *  FIXME: consider how robust we need to be against config changes.
  *  Hullfunc racial abilities are frozen by the time ShipList::findRacialAbilities() is called,
  *  and thus cannot provide live reaction to config changes.
  *  The config abilities code can probably be made entirely robust.
  */

#include "game/spec/racialabilitylist.hpp"
#include "afl/string/format.hpp"
#include "game/playerarray.hpp"
#include "util/translation.hpp"
#include "game/config/costarrayoption.hpp"

/* Marker macro for ability names. grep for it. */
#define ABILITY_PICTURE(x) x

namespace {
    using afl::string::Format;
    using game::PlayerSet_t;
    using game::config::HostConfiguration;

    /* Numeric ranges for the unique Ids.
       These have no real-world equivalent, but better should not clash. */
    const uint32_t FIRST_CONFIG = 1;
    const uint32_t FIRST_HULL_FUNCTION = 100000;

    /* Placeholders for explanations/pictures to be filled in */
    const char*const NO_EXPLANATION = "";
    const char*const NO_PICTURE = "";

    String_t formatRelation(int myValue, int refValue)
    {
        // Check even multipliers
        if (refValue != 0 && (myValue % refValue) == 0) {
            return Format("%dx", myValue / refValue);
        }

        // Check even percentage
        if (refValue != 0) {
            int percent = 100*myValue / refValue;
            if (myValue == percent*refValue / 100) {
                return Format("%d%%", percent);
            }
        }

        // Just produce standard
        return Format("%d", myValue);
    }
}

/*
 *  ConfigBuilder - Helper class to build config-based abilities
 */

class game::spec::RacialAbilityList::ConfigBuilder {
 public:
    ConfigBuilder(Abilities_t& data, const HostConfiguration& config, util::NumberFormatter fmt, afl::string::Translator& tx);

    void add(Category cat, String_t name, const String_t& explanation, const char* pictureName, PlayerSet_t players);
    void addRateConfiguration(Category cat, const HostConfiguration::StandardOptionDescriptor_t& opt, PlayerSet_t limit, String_t name);
    void addCostConfiguration(Category cat, const game::config::CostArrayOptionDescriptor& opt, PlayerSet_t limit, String_t name);
    void addAnonymousRateConfiguration(Category cat, const HostConfiguration::StandardOptionDescriptor_t& opt, PlayerSet_t limit);
    void addTraitors(int race, int percent);
    void addPlayerSpecialMissions();
    void addPlayerRacialAbilities();
    void addEconomyAbilities();
    void addShipAbilities();
    void addShipBuildingAbilities();
    void addMinefieldAbilities();
    void addSensorAbilities();
    void addCombatAbilities();

 private:
    Abilities_t& m_data;
    const HostConfiguration& m_config;
    util::NumberFormatter m_numberFormatter;
    afl::string::Translator& m_translator;
    uint32_t m_uniqueId;
};


game::spec::RacialAbilityList::ConfigBuilder::ConfigBuilder(Abilities_t& data, const HostConfiguration& config, util::NumberFormatter fmt, afl::string::Translator& tx)
    : m_data(data),
      m_config(config),
      m_numberFormatter(fmt),
      m_translator(tx),
      m_uniqueId(FIRST_CONFIG)
{ }

void
game::spec::RacialAbilityList::ConfigBuilder::add(Category cat, String_t name, const String_t& explanation, const char* pictureName, PlayerSet_t players)
{
    // As of 20200430, this combination of parameter types generates smallest object code; 3x String_t generates 5k more
    m_data.push_back(Ability(FromConfiguration, cat, m_uniqueId++, -1, name, explanation, pictureName, players));
}

/* Add a configuration option as an ability.
   For example, given the value "1,20,1,10,1,1,1,1,1", this will identify "1" as the most common value, and generate abilities for "20" and "10".
   No ability is generated if all values are identical (typical case if an arrayized option is set to a single value),
   or too many different values appear and it's not clear which one is the common value.

   \param cat    Category for result
   \param opt    Option descriptor
   \param limit  Only consider these players. For example, HissEffectRate only affects players that have PlayerSpecialMission=2,
                 and a nonstandard HissEffectRate shall not create an ability for other players.
   \param name   Name of ability (used with "Increased" or "Decreased") */
void
game::spec::RacialAbilityList::ConfigBuilder::addRateConfiguration(Category cat, const HostConfiguration::StandardOptionDescriptor_t& opt, PlayerSet_t limit, String_t name)
{
    const HostConfiguration::StandardOption_t& optVal = m_config[opt];

    // Count frequencies
    PlayerArray<int> freq;

    for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
        if (limit.contains(pl)) {
            int existing = 1;
            while (existing < pl && (freq.get(existing) == 0 || optVal(existing) != optVal(pl))) {
                ++existing;
            }
            freq.set(existing, freq.get(existing) + 1);
        }
    }

    // Find most common values
    int mostCommon = 0, total = 0;
    for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
        if (freq.get(pl) != 0) {
            ++total;
            if (mostCommon == 0 || (freq.get(pl) > freq.get(mostCommon))) {
                mostCommon = pl;
            }
        }
    }

    if (total > 4) {
        // More than 4 values.
        // FIXME: should we create abilities here?
    } else if (total > 1) {
        // Try to build relations
        for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
            if (pl != mostCommon && freq.get(pl) != 0) {
                int myValue = optVal(pl);
                int refValue = optVal(mostCommon);
                String_t label = (myValue > refValue
                                  ? (refValue == 0
                                     ? N_("%s (%s)")
                                     : N_("Increased %s (%s)"))
                                  : N_("Reduced %s (%s)"));
                add(cat,
                    Format(m_translator(label), name, formatRelation(myValue, refValue)),
                    Format(m_translator("%s = %d (instead of %d)"), opt.m_name, myValue, refValue),
                    NO_PICTURE,
                    m_config.getPlayersWhere(opt, myValue) & limit);
            }
        }
    } else {
        // Only one value; no ability
    }
}

/* Same as addRateConfiguration, but for a CostArrayOption. */
void
game::spec::RacialAbilityList::ConfigBuilder::addCostConfiguration(Category cat, const game::config::CostArrayOptionDescriptor& opt, PlayerSet_t limit, String_t name)
{
    const game::config::CostArrayOption& optVal = m_config[opt];

    // Count frequencies
    PlayerArray<int> freq;

    for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
        if (limit.contains(pl)) {
            int existing = 1;
            while (existing < pl && (freq.get(existing) == 0 || optVal(existing) != optVal(pl))) {
                ++existing;
            }
            freq.set(existing, freq.get(existing) + 1);
        }
    }

    // Find most common values
    int mostCommon = 0, total = 0;
    for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
        if (freq.get(pl) != 0) {
            ++total;
            if (mostCommon == 0 || (freq.get(pl) > freq.get(mostCommon))) {
                mostCommon = pl;
            }
        }
    }

    if (total > 4) {
        // More than 4 values.
        // FIXME: should we create abilities here?
    } else if (total > 1) {
        // Try to build relations
        for (int pl = 1; pl <= MAX_PLAYERS; ++pl) {
            if (pl != mostCommon && freq.get(pl) != 0) {
                game::spec::Cost myValue = optVal(pl);
                game::spec::Cost refValue = optVal(mostCommon);
                String_t label = (refValue.isEnoughFor(myValue)
                                  ? N_("Reduced %s")
                                  : (myValue.isEnoughFor(refValue)
                                     ? N_("Increased %s")
                                     : N_("Modified %s")));
                add(cat,
                    Format(m_translator(label), name),
                    Format(m_translator("%s = %d (instead of %d)"), opt.m_name, myValue.format(m_translator, m_numberFormatter), refValue.format(m_translator, m_numberFormatter)),
                    NO_PICTURE,
                    m_config.getPlayersWhere(opt, myValue) & limit);
            }
        }
    } else {
        // Only one value; no ability
    }
}

/* Add anonymous config option.
   Like addRateConfiguration, but uses the option's name as label. */
void
game::spec::RacialAbilityList::ConfigBuilder::addAnonymousRateConfiguration(Category cat, const HostConfiguration::StandardOptionDescriptor_t& opt, PlayerSet_t limit)
{
    addRateConfiguration(cat, opt, limit, opt.m_name);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addTraitors(int race, int percent)
{
    add(Combat,
        Format(m_translator("%d%% traitors when boarded"), percent),
        m_translator("Number of crewmen that defect when the ship is boarded"),
        ABILITY_PICTURE("surrender"),
        m_config.getPlayersOfRace(race));
}

/* Add abilities derived from PlayerSpecialMission.
   This affects missions and closely related options. */
void
game::spec::RacialAbilityList::ConfigBuilder::addPlayerSpecialMissions()
{
    afl::string::Translator& tx = m_translator;
    const String_t explanation = tx("This ability defines the meaning of ships' mission 9.");

    // Fed
    if (m_config[HostConfiguration::AllowSuperRefit]()) {
        add(Ship, tx("Super Refit mission"), explanation, ABILITY_PICTURE("superrefit"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 1));
    }

    // Lizard
    if (m_config[HostConfiguration::AllowHiss]()) {
        PlayerSet_t hissers = m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 2);
        add(Ship, tx("Hiss mission"), explanation, ABILITY_PICTURE("hiss"), hissers);
        addRateConfiguration(Ship, HostConfiguration::HissEffectRate, hissers, tx("Hiss effect"));
        // MaxShipsHissing is not arrayized
    }

    // Bird
    add(Ship, tx("Super Spy mission"), explanation, ABILITY_PICTURE("superspy"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 3));

    // Klingon
    add(Ship, tx("Pillage mission"), explanation, ABILITY_PICTURE("pillage"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 4));
    // CumulativePillaging is not arrayized

    // Privateer
    PlayerSet_t robbers = m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 5);
    add(Ship, tx("Rob mission"), explanation, ABILITY_PICTURE("robship"), robbers);
    addRateConfiguration(Ship, HostConfiguration::RobFailureOdds, robbers, tx("Rob failure odds"));

    // Cyborg
    add(Ship, tx("Self Repair mission"), explanation, ABILITY_PICTURE("selfrepair"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 6));

    // Crystal
    if (m_config[HostConfiguration::AllowWebMines]()) {
        add(Ship, tx("Lay Web Mines mission"), explanation, ABILITY_PICTURE("webmines"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 7));
        // UnitsPerWebRate -> addMinefieldAbilities
    }

    // Empire
    add(Ship, tx("Dark Sense mission"), explanation, ABILITY_PICTURE("darksense"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 8));

    // Bot/Colony
    PlayerSet_t buildEnabled = m_config.getPlayersWhereEnabled(HostConfiguration::AllowBuildFighters);
    PlayerSet_t buildMission = (m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 9) | m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 11))
        & buildEnabled;

    add(Ship, tx("Build Fighters mission"), explanation, ABILITY_PICTURE("buildfighters"), buildMission);
    add(Ship, tx("Build Fighters on ships"), tx("Ships can build fighters using friendly code \"lfm\""), ABILITY_PICTURE("buildfighters"), buildEnabled);

    // Rebel
    add(Ship, tx("Rebel Ground Attack mission"), explanation, ABILITY_PICTURE("rga"), m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 10));
}

/* Add racial abilities derived from PlayerRace. */
void
game::spec::RacialAbilityList::ConfigBuilder::addPlayerRacialAbilities()
{
    // Fed
    if (m_config[HostConfiguration::AllowFedCombatBonus]()) {
        add(Combat, m_translator("Crew bonus"), NO_EXPLANATION, ABILITY_PICTURE("fedcrewbonus"), m_config.getPlayersOfRace(1));
    }

    // Lizard
    add(Combat, m_translator("150% damage limit"), NO_EXPLANATION, ABILITY_PICTURE("lizarddamagelimit"), m_config.getPlayersOfRace(2));

    // Bird
    add(Ship, m_translator("Immune to planet attacks if out of fuel"), NO_EXPLANATION, ABILITY_PICTURE("nofuelplanetimmunity"), m_config.getPlayersOfRace(3));

    // Privateer: 3x kill
    // FIXME: in Nu, this is a configurable race ability
    add(Combat, m_translator("Triple Beam Kill"), NO_EXPLANATION, ABILITY_PICTURE("triplebeamkill"), m_config.getPlayersOfRace(5));

    // Borg
    PlayerSet_t borgs = m_config.getPlayersOfRace(6);
    add(Ship, m_translator("Gather debris"), NO_EXPLANATION, ABILITY_PICTURE("gatherdebris"), borgs);
    add(Economy, m_translator("Assimilate natives"), NO_EXPLANATION, ABILITY_PICTURE("assimilate"), borgs);
    addRateConfiguration(Economy, HostConfiguration::BorgAssimilationRate, borgs, m_translator("Assimilation rate"));

    // Fighter sweeping (mostly Colonies)
    // FIXME: it seems a good idea to optimize here (only one entry if fighterSweepers=webSweepers,
    // but this fails because fighterSweepers by default includes player 12+ as at this time we don't know
    // that we have only 11.
    PlayerSet_t fighterSweepers = m_config.getPlayersWhereEnabled(HostConfiguration::FighterSweepRate);
    PlayerSet_t webSweepers = m_config[HostConfiguration::AllowColoniesSweepWebs]()
        ? m_config.getPlayersOfRace(11) & fighterSweepers
        : PlayerSet_t();
    add(Minefield, m_translator("Sweep regular mine fields with fighters"), NO_EXPLANATION, ABILITY_PICTURE("fightersweep"), fighterSweepers);
    add(Minefield, m_translator("Sweep web mine fields with fighters"), NO_EXPLANATION, ABILITY_PICTURE("fightersweep"), webSweepers);
    addRateConfiguration(Minefield, HostConfiguration::FighterSweepRate, fighterSweepers, m_translator("Fighter sweep rate"));
    addRateConfiguration(Minefield, HostConfiguration::FighterSweepRange, fighterSweepers, m_translator("Fighter sweep range"));

    // Traitors
    addTraitors(1, 90);
    addTraitors(5, 100);
    addTraitors(8, 40);
    addTraitors(11, 70);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addEconomyAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);

    // Production
    addAnonymousRateConfiguration(Economy, HostConfiguration::MaxPlanetaryIncome, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::RaceMiningRate, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::StructureDecayPerTurn, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::ProductionRate, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::ColonistTaxRate, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::NativeTaxRate, ALL);

    // Population
    addAnonymousRateConfiguration(Economy, HostConfiguration::RaceGrowthRate, ALL);
    if (m_config[HostConfiguration::ClimateLimitsPopulation]()) {
        addAnonymousRateConfiguration(Economy, HostConfiguration::ClimateDeathRate, ALL);
    }
    add(Economy, m_translator("Overpopulation will eat supplies"), NO_EXPLANATION, NO_PICTURE, m_config.getPlayersWhereEnabled(HostConfiguration::AllowEatingSupplies));
    if (m_config[HostConfiguration::CrystalsPreferDeserts]()) {
        add(Economy, m_translator("Prefers hot planets"), NO_EXPLANATION, NO_PICTURE, m_config.getPlayersOfRace(7));
    }

    // Starbases
    addAnonymousRateConfiguration(Economy, HostConfiguration::RecycleRate, ALL);
    addAnonymousRateConfiguration(Economy, HostConfiguration::FreeFighters, ALL);
    addCostConfiguration(Economy, HostConfiguration::FreeFighterCost, m_config.getPlayersWhereEnabled(HostConfiguration::FreeFighters), HostConfiguration::FreeFighterCost.m_name);
    addCostConfiguration(Economy, HostConfiguration::StarbaseCost, ALL, HostConfiguration::StarbaseCost.m_name);
    addAnonymousRateConfiguration(Economy, HostConfiguration::BaseTechCost, ALL);
    addCostConfiguration(Economy, HostConfiguration::BaseFighterCost, ALL, HostConfiguration::BaseFighterCost.m_name);
    addCostConfiguration(Economy, HostConfiguration::ShipFighterCost, ALL, HostConfiguration::ShipFighterCost.m_name);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addShipAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);

    // Cloak
    addAnonymousRateConfiguration(Ship, HostConfiguration::CloakFailureRate, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::CloakFuelBurn, ALL);
    // addAnonymousRateConfiguration(Ship, HostConfiguration::DamageLevelForCloakFail, ALL);

    // Terraforming
    if (m_config[HostConfiguration::AllowScienceMissions]()) {
        addAnonymousRateConfiguration(Ship, HostConfiguration::TerraformRate, ALL);
    }

    // Towing
    addAnonymousRateConfiguration(Ship, HostConfiguration::TowStrengthEngineScale, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::TowStrengthDistanceScale, ALL);

    // Fuel usage
    addAnonymousRateConfiguration(Ship, HostConfiguration::FuelUsagePerFightFor100KT, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::FuelUsagePerTurnFor100KT, ALL);

    // Experience
    addAnonymousRateConfiguration(Ship, HostConfiguration::EPRecrewScaling, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::EPTrainingScale, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::EPAcademyScale, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::EPShipBuild1000TorpUnits, ALL);
    addAnonymousRateConfiguration(Ship, HostConfiguration::EPShipBuild10Fighters, ALL);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addShipBuildingAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);

    // Cloning
    // FIXME: ShipCloneCostRate only for PHost; replace by a can-clone-ships ability for Host
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::ShipCloneCostRate, ALL);

    // Build queue
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::SBQBuildPALBoost, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::SBQNewBuildPALBoost, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::SBQPointsForAging, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::SBQBuildChangePenalty, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::SBQBoostExpX100, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::BuildChangeRelativePenalty, ALL);

    // PBP
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PBPCostPer100KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PBPMinimumCost, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PBPCloneCostRate, ALL);

    // PAL
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALDecayPerTurn, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALPlayerRate, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALCombatAggressor, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALAggressorPointsPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALOpponentPointsPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALAggressorKillPointsPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALOpponentKillPointsPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALCombatPlanetScaling, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALCombatBaseScaling, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALShipCapturePer10Crew, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALRecyclingPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALBoardingPartyPer10Crew, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALGroundAttackPer100Clans, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALGloryDevice, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALGloryDevicePer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALGloryDamagePer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALGloryKillPer10KT, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALImperialAssault, ALL);
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALRGA, m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 10));
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALPillage, m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 4));
    add(ShipBuilding, HostConfiguration::PALIncludesESB.m_name, NO_EXPLANATION, NO_PICTURE, m_config.getPlayersWhereEnabled(HostConfiguration::PALIncludesESB));
    addAnonymousRateConfiguration(ShipBuilding, HostConfiguration::PALShipMinekillPer10KT, ALL);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addMinefieldAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);
    const PlayerSet_t THOLIANS = m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 7);
    bool hasWebs = m_config[HostConfiguration::AllowWebMines]();

    // Laying
    // add(Minefield, m_translator("Can lay minefields"), NO_EXPLANATION, NO_PICTURE, m_config.getPlayersWhereEnabled(HostConfiguration::AllowMinefields));
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MaximumMinefieldRadius, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MaximumWebMinefieldRadius, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MaximumMinefieldsPerPlayer, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::UnitsPerTorpRate, ALL);
    if (hasWebs) {
        addAnonymousRateConfiguration(Minefield, HostConfiguration::UnitsPerWebRate, THOLIANS);
    }

    // Sweeping
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineSweepRate, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineSweepRate, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineSweepRange, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineSweepRange, ALL);

    // Movement
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineHitOdds, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineHitOdds, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineHitOddsWhenCloakedX10, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineOddsWarpBonusX100, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::CloakMineOddsWarpBonusX100, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineOddsWarpBonusX100, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineTravelSafeWarp, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::CloakedMineTravelSafeWarp, ALL);
    addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineTravelSafeWarp, ALL);

    // Decay
    addAnonymousRateConfiguration(Minefield, HostConfiguration::MineDecayRate, ALL);
    if (hasWebs) {
        addAnonymousRateConfiguration(Minefield, HostConfiguration::WebMineDecayRate, THOLIANS);
    }
}


void
game::spec::RacialAbilityList::ConfigBuilder::addSensorAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);

    addAnonymousRateConfiguration(Sensor, HostConfiguration::ScanRange, ALL);
    addAnonymousRateConfiguration(Sensor, HostConfiguration::SensorRange, ALL);
    addAnonymousRateConfiguration(Sensor, HostConfiguration::DarkSenseRange, m_config.getPlayersWhere(HostConfiguration::PlayerSpecialMission, 8));
    addAnonymousRateConfiguration(Sensor, HostConfiguration::MineScanRange, ALL);
    addAnonymousRateConfiguration(Sensor, HostConfiguration::WrmScanRange, ALL);
}

void
game::spec::RacialAbilityList::ConfigBuilder::addCombatAbilities()
{
    const PlayerSet_t ALL = PlayerSet_t::allUpTo(MAX_PLAYERS);

    // Ground combat
    addRateConfiguration(Combat, HostConfiguration::GroundKillFactor, ALL, m_translator("Ground attack strength"));
    addRateConfiguration(Combat, HostConfiguration::GroundDefenseFactor, ALL, m_translator("Ground defense strength"));

    // General
    if (m_config[HostConfiguration::AllowEngineShieldBonus]()) {
        addRateConfiguration(Combat, HostConfiguration::EngineShieldBonusRate, ALL, m_translator("Engine/Shield bonus rate"));
    }
    addAnonymousRateConfiguration(Combat, HostConfiguration::ColonistCombatCaptureRate, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::ColonistCombatSurvivalRate, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::ShipMovementSpeed, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::MaximumFightersOnBase, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::MaximumDefenseOnBase, ALL);

    // Weapon formulas
    addAnonymousRateConfiguration(Combat, HostConfiguration::CrewKillScaling, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::HullDamageScaling, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::ShieldDamageScaling, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::ShieldKillScaling, ALL);

    // Beams
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamFiringRange, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamHitBonus, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamHitFighterCharge, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamHitFighterRange, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamHitOdds, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamHitShipCharge, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamRechargeBonus, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BeamRechargeRate, ALL);

    // Fighters
    addAnonymousRateConfiguration(Combat, HostConfiguration::BayLaunchInterval, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BayRechargeBonus, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::BayRechargeRate, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::ExtraFighterBays, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::FighterBeamExplosive, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::FighterBeamKill, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::FighterFiringRange, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::FighterKillOdds, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::FighterMovementSpeed, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::MaxFightersLaunched, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::StrikesPerFighter, ALL);

    // Torpedoes
    add(Combat, m_translator("Use starbase torpedoes in combat"), NO_EXPLANATION, NO_PICTURE, m_config.getPlayersWhereEnabled(HostConfiguration::UseBaseTorpsInCombat));
    addAnonymousRateConfiguration(Combat, HostConfiguration::PlanetaryTorpsPerTube, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::TorpFiringRange, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::TorpHitBonus, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::TorpHitOdds, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::TubeRechargeBonus, ALL);
    addAnonymousRateConfiguration(Combat, HostConfiguration::TubeRechargeRate, ALL);
}



/*
 *  RacialAbilityList
 */

// Constructor.
game::spec::RacialAbilityList::RacialAbilityList()
    : m_data()
{ }

// Destructor.
game::spec::RacialAbilityList::~RacialAbilityList()
{ }

// Add abilities derived from ship list.
void
game::spec::RacialAbilityList::addShipRacialAbilities(const ShipList& shipList)
{
    const HullFunctionAssignmentList& abilities = shipList.racialAbilities();
    uint32_t uniqueId = FIRST_HULL_FUNCTION;
    for (size_t i = 0, n = abilities.getNumEntries(); i < n; ++i) {
        if (const HullFunctionAssignmentList::Entry* p = abilities.getEntryByIndex(i)) {
            HullFunction fcn;
            if (shipList.modifiedHullFunctions().getFunctionDefinition(p->m_function, fcn)) {
                if (const BasicHullFunction* basic = shipList.basicHullFunctions().getFunctionById(fcn.getBasicFunctionId())) {
                    m_data.push_back(Ability(FromHullFunction, Ship, uniqueId, fcn.getBasicFunctionId(), basic->getDescription(), basic->getExplanation(), basic->getPictureName(), p->m_addedPlayers));
                }
            }
        }
        ++uniqueId;
    }
}

// Add abilities derived from configuration.
void
game::spec::RacialAbilityList::addConfigRacialAbilities(const game::config::HostConfiguration& config, util::NumberFormatter fmt, afl::string::Translator& tx)
{
    ConfigBuilder b(m_data, config, fmt, tx);

    b.addPlayerSpecialMissions();
    b.addPlayerRacialAbilities();
    b.addEconomyAbilities();
    b.addShipAbilities();
    b.addShipBuildingAbilities();
    b.addMinefieldAbilities();
    b.addSensorAbilities();
    b.addCombatAbilities();
}

// Filter players.
void
game::spec::RacialAbilityList::filterPlayers(PlayerSet_t players)
{
    Abilities_t result;
    for (Iterator_t i = begin(), e = end(); i != e; ++i) {
        if (i->players.containsAnyOf(players)) {
            result.push_back(*i);
        }
    }
    result.swap(m_data);
}

// Get number of abilities.
size_t
game::spec::RacialAbilityList::size() const
{
    return m_data.size();
}

// Get ability by index.
const game::spec::RacialAbilityList::Ability*
game::spec::RacialAbilityList::get(size_t index) const
{
    if (index < m_data.size()) {
        return &m_data[index];
    } else {
        return 0;
    }
}

// Get iterator to beginning.
game::spec::RacialAbilityList::Iterator_t
game::spec::RacialAbilityList::begin() const
{
    return m_data.begin();
}

// Get iterator to one-past-end.
game::spec::RacialAbilityList::Iterator_t
game::spec::RacialAbilityList::end() const
{
    return m_data.end();
}

String_t
game::spec::toString(RacialAbilityList::Category cat, afl::string::Translator& tx)
{
    switch (cat) {
     case RacialAbilityList::Combat:       return tx("Combat");
     case RacialAbilityList::Economy:      return tx("Economy");
     case RacialAbilityList::Minefield:    return tx("Minefields");
     case RacialAbilityList::Sensor:       return tx("Sensors");
     case RacialAbilityList::Ship:         return tx("Ships");
     case RacialAbilityList::ShipBuilding: return tx("Ship building");
    }
    return String_t();
}

String_t
game::spec::toString(RacialAbilityList::Origin origin, afl::string::Translator& tx)
{
    switch (origin) {
     case RacialAbilityList::FromHullFunction:  return tx("Ship functions");
     case RacialAbilityList::FromConfiguration: return tx("Host configuration");
    }
    return String_t();
}
