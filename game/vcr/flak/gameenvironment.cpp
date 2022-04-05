/**
  *  \file game/vcr/flak/gameenvironment.cpp
  *  \brief Class game::vcr::flak::GameEnvironment
  */

#include "game/vcr/flak/gameenvironment.hpp"
#include "game/spec/beam.hpp"
#include "game/spec/torpedolauncher.hpp"

using game::config::HostConfiguration;

game::vcr::flak::GameEnvironment::GameEnvironment(const game::config::HostConfiguration& config, const game::spec::BeamVector_t& beams, const game::spec::TorpedoVector_t& torps)
    : m_config(config),
      m_beams(beams),
      m_torpedos(torps)
{
    // Cache getPlayerRaceNumber() in a local array.
    // getPlayerRaceNumber() is called by the algorithm in main loops.
    // It uses dynamic_cast to type-check the ConfigurationOption, which makes it expensive.
    // This optimisation saves about 7% total runtime.
    // (c2simtool -G testgames/pl13 --run 500 --mode=flak --seed=1 11carriers.ccb, 5.7 -> 5.3 seconds)
    for (int i = 1; i <= MAX_PLAYERS; ++i) {
        m_playerRace[i-1] = config.getPlayerRaceNumber(i);
    }
}

int
game::vcr::flak::GameEnvironment::getConfiguration(ScalarOption index) const
{
    // ex flak/flak-port.h:CONFIG_SCALAR
    const game::config::IntegerOptionDescriptor* desc = 0;
    switch (index) {
     case AllowAlternativeCombat:  desc = &HostConfiguration::AllowAlternativeCombat; break;
     case FireOnAttackFighters:    desc = &HostConfiguration::FireOnAttackFighters;   break;
     case StandoffDistance:        desc = &HostConfiguration::StandoffDistance;       break;
    }
    if (desc != 0) {
        return m_config[*desc]();
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getConfiguration(ArrayOption index, int player) const
{
    // ex flak/flak-port.h:CONFIG_ARRAY
    const HostConfiguration::StandardOptionDescriptor_t* desc = 0;
    switch (index) {
     case BayLaunchInterval:  desc = &HostConfiguration::BayLaunchInterval;  break;
     case BeamFiringRange:    desc = &HostConfiguration::BeamFiringRange;    break;
     case BeamHitShipCharge:  desc = &HostConfiguration::BeamHitShipCharge;  break;
     case FighterFiringRange: desc = &HostConfiguration::FighterFiringRange; break;
     case FighterKillOdds:    desc = &HostConfiguration::FighterKillOdds;    break;
     case ShipMovementSpeed:  desc = &HostConfiguration::ShipMovementSpeed;  break;
     case TorpFiringRange:    desc = &HostConfiguration::TorpFiringRange;    break;
    }
    if (desc != 0) {
        return m_config[*desc](player);
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getExperienceConfiguration(ExperienceOption index, int level, int player) const
{
    // ex flak/flak-port.h:CONFIG_EX
    const HostConfiguration::StandardOptionDescriptor_t* sd = 0;
    const HostConfiguration::ExperienceOptionDescriptor_t* ed = 0;
    switch (index) {
     case BayRechargeBonus:     sd = &HostConfiguration::BayRechargeBonus;     ed = &HostConfiguration::EModBayRechargeBonus;     break;
     case BayRechargeRate:      sd = &HostConfiguration::BayRechargeRate;      ed = &HostConfiguration::EModBayRechargeRate;      break;
     case BeamHitBonus:         sd = &HostConfiguration::BeamHitBonus;         ed = &HostConfiguration::EModBeamHitBonus;         break;
     case BeamHitFighterCharge: sd = &HostConfiguration::BeamHitFighterCharge; ed = &HostConfiguration::EModBeamHitFighterCharge; break;
     case BeamHitOdds:          sd = &HostConfiguration::BeamHitOdds;          ed = &HostConfiguration::EModBeamHitOdds;          break;
     case BeamRechargeBonus:    sd = &HostConfiguration::BeamRechargeBonus;    ed = &HostConfiguration::EModBeamRechargeBonus;    break;
     case BeamRechargeRate:     sd = &HostConfiguration::BeamRechargeRate;     ed = &HostConfiguration::EModBeamRechargeRate;     break;
     case CrewKillScaling:      sd = &HostConfiguration::CrewKillScaling;      ed = &HostConfiguration::EModCrewKillScaling;      break;
     case FighterBeamExplosive: sd = &HostConfiguration::FighterBeamExplosive; ed = &HostConfiguration::EModFighterBeamExplosive; break;
     case FighterBeamKill:      sd = &HostConfiguration::FighterBeamKill;      ed = &HostConfiguration::EModFighterBeamKill;      break;
     case FighterMovementSpeed: sd = &HostConfiguration::FighterMovementSpeed; ed = &HostConfiguration::EModFighterMovementSpeed; break;
     case HullDamageScaling:    sd = &HostConfiguration::HullDamageScaling;    ed = &HostConfiguration::EModHullDamageScaling;    break;
     case ShieldDamageScaling:  sd = &HostConfiguration::ShieldDamageScaling;  ed = &HostConfiguration::EModShieldDamageScaling;  break;
     case ShieldKillScaling:    sd = &HostConfiguration::ShieldKillScaling;    ed = &HostConfiguration::EModShieldKillScaling;    break;
     case StrikesPerFighter:    sd = &HostConfiguration::StrikesPerFighter;    ed = &HostConfiguration::EModStrikesPerFighter;    break;
     case TorpHitBonus:         sd = &HostConfiguration::TorpHitBonus;         ed = &HostConfiguration::EModTorpHitBonus;         break;
     case TorpHitOdds:          sd = &HostConfiguration::TorpHitOdds;          ed = &HostConfiguration::EModTorpHitOdds;          break;
     case TubeRechargeBonus:    sd = &HostConfiguration::TubeRechargeBonus;    ed = &HostConfiguration::EModTubeRechargeBonus;    break;
     case TubeRechargeRate:     sd = &HostConfiguration::TubeRechargeRate;     ed = &HostConfiguration::EModTubeRechargeRate;     break;
    }
    if (sd != 0 && ed != 0) {
        return m_config[*sd](player) + m_config.getExperienceBonus(*ed, level);
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getBeamKillPower(int type) const
{
    if (const game::spec::Beam* b = m_beams.get(type)) {
        return b->getKillPower();
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getBeamDamagePower(int type) const
{
    if (const game::spec::Beam* b = m_beams.get(type)) {
        return b->getDamagePower();
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getTorpedoKillPower(int type) const
{
    if (const game::spec::TorpedoLauncher* tl = m_torpedos.get(type)) {
        return tl->getKillPower();
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getTorpedoDamagePower(int type) const
{
    if (const game::spec::TorpedoLauncher* tl = m_torpedos.get(type)) {
        return tl->getDamagePower();
    } else {
        return 0;
    }
}

int
game::vcr::flak::GameEnvironment::getPlayerRaceNumber(int player) const
{
    if (player > 0 && player <= MAX_PLAYERS) {
        return m_playerRace[player-1];
    } else {
        return player;
    }
}
