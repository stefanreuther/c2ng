/**
  *  \file game/sim/configuration.cpp
  *  \brief Class game::sim::Configuration
  */

#include "game/sim/configuration.hpp"
#include "game/limits.hpp"

// Default constructor.
game::sim::Configuration::Configuration()
    : m_allianceSettings(),
      m_enemySettings(),
      m_engineShieldBonus(0),
      m_scottyBonus(true),
      m_randomLeftRight(false),
      m_honorAlliances(true),
      m_onlyOneSimulation(false),
      m_seedControl(false),
      m_randomizeFCodesOnEveryFight(false),
      m_balancingMode(BalanceNone),
      m_vcrMode(VcrPHost4)
{
    // ex GSimOptions::GSimOptions
}

// Load defaults.
void
game::sim::Configuration::loadDefaults(const TeamSettings& teams)
{
    // FIXME: this function has interesting semantics - do we need it?
    // ex GSimOptions::loadDefaults (sort-of). Original loadDefaults also implies setMode()
    m_allianceSettings.clear();
    m_enemySettings.clear();
    m_honorAlliances = true;
    m_onlyOneSimulation = false;
    m_seedControl = false;
    m_randomizeFCodesOnEveryFight = false;
    // m_seed = 0;

    // Initialize alliances from teams:
    for (int a = 1; a <= MAX_PLAYERS; ++a) {
        for (int b = 1; b <= MAX_PLAYERS; ++b) {
            if (a != b && teams.getPlayerTeam(a) != 0 && teams.getPlayerTeam(a) == teams.getPlayerTeam(b)) {
                m_allianceSettings.set(a, b, true);
            }
        }
    }
}

// Set mode (host version).
void
game::sim::Configuration::setMode(VcrMode mode, const TeamSettings& teams, const game::config::HostConfiguration& config)
{
    // ex GSimOptions::setMode
    m_engineShieldBonus = config[config.AllowEngineShieldBonus]() ? config[config.EngineShieldBonusRate](teams.getViewpointPlayer()) : 0;
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

// Check enabled experience.
bool
game::sim::Configuration::isExperienceEnabled(const game::config::HostConfiguration& config) const
{
    // ex GSimOptions::isExperienceEnabled
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
