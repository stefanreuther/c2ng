/**
  *  \file game/sim/configuration.cpp
  *  \brief Class game::sim::Configuration
  */

#include "game/sim/configuration.hpp"
#include "game/limits.hpp"

// Default constructor.
game::sim::Configuration::Configuration()
    : m_engineShieldBonus(0),
      m_scottyBonus(true),
      m_randomLeftRight(false),
      m_honorAlliances(true),
      m_onlyOneSimulation(false),
      m_seedControl(false),
      m_randomizeFCodesOnEveryFight(false),
      m_balancingMode(BalanceNone),
      m_vcrMode(VcrPHost4),
      m_allianceSettings(),
      m_enemySettings()
{
    // ex GSimOptions::GSimOptions
}

// Instead of loadDefaults() / InitSimConfig(), assign a default-initialized config and do setMode() / setModeFromHostVersion().

// Copy (parts) from another configuration.
void
game::sim::Configuration::copyFrom(const Configuration& other, Areas_t areas)
{
    if (areas.contains(MainArea)) {
        m_engineShieldBonus           = other.m_engineShieldBonus;
        m_scottyBonus                 = other.m_scottyBonus;
        m_randomLeftRight             = other.m_randomLeftRight;
        m_honorAlliances              = other.m_honorAlliances;
        m_onlyOneSimulation           = other.m_onlyOneSimulation;
        m_seedControl                 = other.m_seedControl;
        m_randomizeFCodesOnEveryFight = other.m_randomizeFCodesOnEveryFight;
        m_balancingMode               = other.m_balancingMode;
        m_vcrMode                     = other.m_vcrMode;
    }
    if (areas.contains(AllianceArea)) {
        m_allianceSettings            = other.m_allianceSettings;
    }
    if (areas.contains(EnemyArea)) {
        m_enemySettings               = other.m_enemySettings;
    }
}

// Set mode (host version).
void
game::sim::Configuration::setMode(VcrMode mode, int player, const game::config::HostConfiguration& config)
{
    // ex GSimOptions::setMode, ccsim.pas:InitSimConfig (part)
    m_engineShieldBonus = config[config.AllowEngineShieldBonus]() ? config[config.EngineShieldBonusRate](player) : 0;
    m_scottyBonus       = config[config.AllowFedCombatBonus]();
    m_vcrMode           = mode;
    switch (mode) {
     case VcrPHost2:
     case VcrPHost3:
     case VcrPHost4:
     case VcrFLAK:
        m_randomLeftRight = true;
        m_balancingMode = BalanceNone;
        break;

     case VcrHost:
     case VcrNuHost:
        m_randomLeftRight = false;
        m_balancingMode = Balance360k;
        break;
    }
}

// Set mode according to a host version.
void
game::sim::Configuration::setModeFromHostVersion(HostVersion host, int player, const game::config::HostConfiguration& config)
{
    // ex initSimulatorState (part)
    // This function is placed here, not in class HostVersion, to keep the number of dependencies of HostVersion low.
    VcrMode mode = VcrPHost4;
    switch (host.getKind()) {
     case HostVersion::Unknown:
     case HostVersion::Host:
     case HostVersion::SRace:
        mode = VcrHost;
        break;

     case HostVersion::PHost:
        if (host.getVersion() < MKVERSION(3,0,0)) {
            mode = VcrPHost2;
        } else if (host.getVersion() < MKVERSION(4,0,0)) {
            mode = VcrPHost3;
        } else {
            mode = VcrPHost4;
        }
        break;

     case HostVersion::NuHost:
        mode = VcrNuHost;
        break;
    }

    setMode(mode, player, config);
}

// Check enabled experience.
bool
game::sim::Configuration::isExperienceEnabled(const game::config::HostConfiguration& config) const
{
    // ex GSimOptions::isExperienceEnabled, ccsim.pas:IsExpGame
    switch (m_vcrMode) {
     case VcrPHost4:
     case VcrFLAK:
        return config[config.NumExperienceLevels]() > 0;

     case VcrPHost2:
     case VcrPHost3:
     case VcrNuHost:
     case VcrHost:
        return false;
    }
    return false;
}

// Set engine/shield bonus.
void
game::sim::Configuration::setEngineShieldBonus(int n)
{
    m_engineShieldBonus = n;
}

// Get engine/shield bonus.
int
game::sim::Configuration::getEngineShieldBonus() const
{
    return m_engineShieldBonus;
}

// Set scotty bonus.
void
game::sim::Configuration::setScottyBonus(bool enable)
{
    m_scottyBonus = enable;
}

// Check for scotty bonus.
bool
game::sim::Configuration::hasScottyBonus() const
{
    return m_scottyBonus;
}

// Set random left/right assignment.
void
game::sim::Configuration::setRandomLeftRight(bool enable)
{
    m_randomLeftRight = enable;
}

// Check for random left/right assignment.
bool
game::sim::Configuration::hasRandomLeftRight() const
{
    return m_randomLeftRight;
}

// Set whether alliances are honored.
void
game::sim::Configuration::setHonorAlliances(bool enable)
{
    m_honorAlliances = enable;
}

// Check whether alliances are honored.
bool
game::sim::Configuration::hasHonorAlliances() const
{
    return m_honorAlliances;
}

// Set limitation to one fight.
void
game::sim::Configuration::setOnlyOneSimulation(bool enable)
{
    m_onlyOneSimulation = enable;
    if (!m_onlyOneSimulation) {
        m_seedControl = false;
    }
}

// Check limitation to one fight.
bool
game::sim::Configuration::hasOnlyOneSimulation() const
{
    return m_onlyOneSimulation;
}

// Set seed control.
void
game::sim::Configuration::setSeedControl(bool enable)
{
    m_seedControl = enable;
    if (m_seedControl) {
        m_onlyOneSimulation = true;
    }
}

// Check for seed control.
bool
game::sim::Configuration::hasSeedControl() const
{
    return m_seedControl;
}

// Set whether friendly codes are randomized on every fight.
void
game::sim::Configuration::setRandomizeFCodesOnEveryFight(bool enable)
{
    m_randomizeFCodesOnEveryFight = enable;
}

// Check whether friendly codes are randomized on every fight.
bool
game::sim::Configuration::hasRandomizeFCodesOnEveryFight() const
{
    return m_randomizeFCodesOnEveryFight;
}

// Set balancing mode.
void
game::sim::Configuration::setBalancingMode(BalancingMode mode)
{
    m_balancingMode = mode;
}

// Get balancing mode.
game::sim::Configuration::BalancingMode
game::sim::Configuration::getBalancingMode() const
{
    return m_balancingMode;
}

// Get simulation mode (host version).
game::sim::Configuration::VcrMode
game::sim::Configuration::getMode() const
{
    return m_vcrMode;
}

// Check whether host version honors Alternative Combat settings.
bool
game::sim::Configuration::hasAlternativeCombat() const
{
    switch (getMode()) {
     case VcrHost:
        return false;
     case VcrPHost2:
     case VcrPHost3:
     case VcrPHost4:
     case VcrFLAK:
        return true;
     case VcrNuHost:
        return false;
    }
    return false;
}

game::PlayerBitMatrix&
game::sim::Configuration::allianceSettings()
{
    return m_allianceSettings;
}

const game::PlayerBitMatrix&
game::sim::Configuration::allianceSettings() const
{
    return m_allianceSettings;
}

game::PlayerBitMatrix&
game::sim::Configuration::enemySettings()
{
    return m_enemySettings;
}

const game::PlayerBitMatrix&
game::sim::Configuration::enemySettings() const
{
    return m_enemySettings;
}

String_t
game::sim::toString(Configuration::BalancingMode mode, afl::string::Translator& tx)
{
    switch (mode) {
     case Configuration::BalanceNone:         return tx("none");
     case Configuration::Balance360k:         return tx("360 kt (Host)");
     case Configuration::BalanceMasterAtArms: return tx("Master at Arms");
    }
    return String_t();
}

String_t
game::sim::toString(Configuration::VcrMode mode, afl::string::Translator& tx)
{
    switch (mode) {
     case Configuration::VcrHost:   return tx("Host");
     case Configuration::VcrPHost2: return tx("PHost 2");
     case Configuration::VcrPHost3: return tx("PHost 3");
     case Configuration::VcrPHost4: return tx("PHost 4");
     case Configuration::VcrFLAK:   return tx("FLAK");
     case Configuration::VcrNuHost: return tx("NuHost");
    }
    return String_t();
}

game::sim::Configuration::BalancingMode
game::sim::getNext(Configuration::BalancingMode mode)
{
    switch (mode) {
     case Configuration::BalanceNone:
     case Configuration::Balance360k:
        return static_cast<Configuration::BalancingMode>(mode + 1);
     case Configuration::BalanceMasterAtArms:
        return Configuration::BalanceNone;
    }
    return Configuration::BalanceNone;
}

game::sim::Configuration::VcrMode
game::sim::getNext(Configuration::VcrMode mode)
{
    switch (mode) {
     case Configuration::VcrHost:
     case Configuration::VcrPHost2:
     case Configuration::VcrPHost3:
     case Configuration::VcrPHost4:
     case Configuration::VcrFLAK:
        return static_cast<Configuration::VcrMode>(mode + 1);
     case Configuration::VcrNuHost:
        return Configuration::VcrHost;
    }
    return Configuration::VcrHost;
}
