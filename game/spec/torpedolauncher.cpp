/**
  *  \file game/spec/torpedolauncher.cpp
  *  \brief Class game::spec::TorpedoLauncher
  */

#include "game/spec/torpedolauncher.hpp"

using game::config::HostConfiguration;

game::spec::TorpedoLauncher::TorpedoLauncher(int id)
    : Weapon(ComponentNameProvider::Torpedo, id),
      m_torpedoCost(),
      m_firingRangeBonus(0)
{ }

game::spec::Cost&
game::spec::TorpedoLauncher::torpedoCost()
{
    return m_torpedoCost;
}

const game::spec::Cost&
game::spec::TorpedoLauncher::torpedoCost() const
{
    return m_torpedoCost;
}

void
game::spec::TorpedoLauncher::setFiringRangeBonus(int n)
{
    m_firingRangeBonus = n;
}

int
game::spec::TorpedoLauncher::getFiringRangeBonus() const
{
    return m_firingRangeBonus;
}

int
game::spec::TorpedoLauncher::getRechargeTime(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (!host.isPHost()) {
        // Torpedo recharge times for THost. I won't use a huge FP formula when a
        // 10-byte table also does :-) This table is based upon the following:
        // - Facts:
        //   . torps fire at charge 41 charge in any case
        //   . between 31 and 40, the chance that it fires is PROB = (type-1)/17
        //     i.e. zero for type 1, 1/17 for type 2, ... 9/17 for type 10
        // - Hence, the chance that a torp fires after 31+x is
        //     (1-PROB)^x
        // - We want the time x where 50% of all torps fire:
        //     0.5     = exp(ln(1-PROB)*x)
        //     ln(0.5) = ln(1-PROB) * x
        //     x       = ln(0.5) / ln(1-PROB)
        //             = ln(0.5) / ln(1-(tech-1)/17)
        //   For tech \in [1,10] this yields
        //     INF, 11.4, 5.5, 3.6, 2.6, 2.0, 1.6, 1.3, 1.1, 0.9
        //   Add 31, trim to 41, voila.
        const int MAX = 10;
        static const int8_t recharge_times[MAX] = {
           41, 41, 36, 34, 33, 33, 32, 32, 32, 31
        };

        int index = std::max(0, std::min(MAX, getId()) - 1);
        return recharge_times[index];
    } else {
        // Torpedo recharge times for PHost.
        // - torpedoes fire at 1000
        // - recharge is random in [0, N) with N = (bonus * boom / 100) + rate)
        //   (plus experience effects, times Nu recharge rate; not relevant here)
        // - at least 1
        // Therefore, recharge is N/2, or time is 1000/(N/2) = 2000/N.
        // @change PCC2 shows N=0 as does-not-recharge.
        const int bang = getKillPower() + getDamagePower();
        const int rate = std::max(1, bang * config[HostConfiguration::TubeRechargeBonus](forPlayer) / 100 + config[HostConfiguration::TubeRechargeRate](forPlayer));
        return 2000 / rate;
    }
}

int
game::spec::TorpedoLauncher::getHitOdds(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const
{
    if (!host.isPHost()) {
        // Default TorpMissRate=35, which is nominally a 65% hit rate.
        // Actual rate is 66.6% due to the random number distribution (80/120).
        return 66;
    } else {
        const int bang = getKillPower() + getDamagePower();
        const int rate = bang * config[HostConfiguration::TorpHitBonus](forPlayer) / 100 + config[HostConfiguration::TorpHitOdds](forPlayer);
        return std::max(0, std::min(100, rate));
    }
}

bool
game::spec::TorpedoLauncher::getMinefieldCost(int forPlayer, int numMines, bool isWeb, const game::config::HostConfiguration& config, Cost& result) const
{
    const int rate = config[isWeb ? HostConfiguration::UnitsPerWebRate : HostConfiguration::UnitsPerTorpRate](forPlayer) * getId() * getId();
    if (rate <= 0) {
        return false;
    }

    result = torpedoCost();
    result *= numMines * 100;
    result /= rate;
    return true;
}
