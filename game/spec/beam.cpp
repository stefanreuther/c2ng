/**
  *  \file game/spec/beam.cpp
  *  \brief Class game::spec::Beam
  */

#include "game/spec/beam.hpp"

using game::config::HostConfiguration;

// Constructor.
game::spec::Beam::Beam(int id)
    : Weapon(ComponentNameProvider::Beam, id)
{ }

int
game::spec::Beam::getRechargeTime(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (!host.isPHost()) {
        // Recharge 0.5 per tick, fire at 50 -> 100
        return 100;
    } else {
        // PHost: recharges by [0, (Kill+Damage) * BeamRechargeBonus + BeamRechargeRate)
        //   per tick, fires at BeamHitShipCharge
        //   -> 2*BeamHitShipCharge / ((Kill+Damage) * BRB + BRR)  */
        // @change PCC2 shows N=0 as does-not-recharge
        const int bang = getKillPower() + getDamagePower();
        const int rate = std::max(1, bang * config[HostConfiguration::BeamRechargeBonus](forPlayer) / 100 + config[HostConfiguration::BeamRechargeRate](forPlayer));

        return 2 * config[HostConfiguration::BeamHitShipCharge](forPlayer) / rate;
    }
}

int
game::spec::Beam::getHitOdds(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (!host.isPHost()) {
        // Beams always hit
        return 100;
    } else {
        const int bang = getKillPower() + getDamagePower();
        const int rate = bang * config[HostConfiguration::BeamHitBonus](forPlayer) / 100 + config[HostConfiguration::BeamHitOdds](forPlayer);
        return std::max(0, std::min(100, rate));
    }
}

int
game::spec::Beam::getNumMinesSwept(int forPlayer, bool isWeb, const game::config::HostConfiguration& config) const
{
    const int rate = config[isWeb ? HostConfiguration::WebMineSweepRate : HostConfiguration::MineSweepRate](forPlayer);
    return rate * getId() * getId();
}
