/**
  *  \file game/sim/configuration.cpp
  */

#include "game/sim/configuration.hpp"
#include "game/limits.hpp"

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

void
game::sim::Configuration::loadDefaults(const TeamSettings& teams)
{
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

void
game::sim::Configuration::setEngineShieldBonus(int n)
{
    m_engineShieldBonus = n;
}

int
game::sim::Configuration::getEngineShieldBonus() const
{
    return m_engineShieldBonus;
}

void
game::sim::Configuration::setScottyBonus(bool enable)
{
    m_scottyBonus = enable;
}

bool
game::sim::Configuration::hasScottyBonus() const
{
    return m_scottyBonus;
}

void
game::sim::Configuration::setRandomLeftRight(bool enable)
{
    m_randomLeftRight = enable;
}

bool
game::sim::Configuration::hasRandomLeftRight() const
{
    return m_randomLeftRight;
}

void
game::sim::Configuration::setHonorAlliances(bool enable)
{
    m_honorAlliances = enable;
}

bool
game::sim::Configuration::hasHonorAlliances() const
{
    return m_honorAlliances;
}

void
game::sim::Configuration::setOnlyOneSimulation(bool enable)
{
    m_onlyOneSimulation = enable;
}

bool
game::sim::Configuration::hasOnlyOneSimulation() const
{
    return m_onlyOneSimulation;
}

void
game::sim::Configuration::setSeedControl(bool enable)
{
    m_seedControl = enable;
}

bool
game::sim::Configuration::hasSeedControl() const
{
    return m_seedControl;
}

void
game::sim::Configuration::setRandomizeFCodesOnEveryFight(bool enable)
{
    m_randomizeFCodesOnEveryFight = enable;
}

bool
game::sim::Configuration::hasRandomizeFCodesOnEveryFight() const
{
    return m_randomizeFCodesOnEveryFight;
}

void
game::sim::Configuration::setBalancingMode(BalancingMode mode)
{
    m_balancingMode = mode;
}

game::sim::Configuration::BalancingMode
game::sim::Configuration::getBalancingMode() const
{
    return m_balancingMode;
}

game::sim::Configuration::VcrMode
game::sim::Configuration::getMode() const
{
    return m_vcrMode;
}
