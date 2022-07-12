/**
  *  \file game/spec/fighter.cpp
  *  \brief Class game::spec::Fighter
  */

#include "game/spec/fighter.hpp"
#include "afl/string/format.hpp"

const int32_t game::spec::Fighter::MAX_INTERVAL;

// Constructor.
game::spec::Fighter::Fighter(int id,
                             const game::config::HostConfiguration& config,
                             const PlayerList& players,
                             afl::string::Translator& tx)
    : Weapon(ComponentNameProvider::Fighter, id)
{
    // ex GFighter::GFighter(int id)
    setKillPower(config[config.FighterBeamKill](id));
    setDamagePower(config[config.FighterBeamExplosive](id));
    cost() = config[config.BaseFighterCost](id);
    setName(afl::string::Format(tx("%s fighter"), players.getPlayerName(id, Player::AdjectiveName, tx)));
    setMass(1);
    setTechLevel(1);
    setShortName(tx("Ftr"));
}

game::spec::Fighter::Range_t
game::spec::Fighter::getRechargeTime(const HostVersion& host, const game::config::HostConfiguration& config) const
{
    /* Recharge time for PHost: Actual recharge rate is BayRechargeRate + nbays*BayRechargeBonus.
       We cannot model the bonus, but we know that the actual rate includes minimum one bonus.
       We can add a relation sign to help the user.

       Given a rate, the average recharge per tick is (rate-1)/2, making the average time
       2000/(rate-1).

       Recharge time for THost: a fighter is launched if rand(1..20) <= nbays. That is, a one-bay
       ship launches at a rate of 1/20, a two-bay ship at 2/20, a three-bay ship at 3/20, etc.,
       making the per-bay launch rate 1/20, and the recharge time 20. */
    if (host.isPHost()) {
        // Config values
        const int bonus = config[config.BayRechargeBonus](getId());
        const int nomRate = config[config.BayRechargeRate](getId());
        const int limit = config[config.BayLaunchInterval](getId());

        // Effective rates for 1..20 bays
        const int minRate = std::max(1, nomRate + bonus);
        const int maxRate = std::max(1, nomRate + 20*bonus);

        // Intervals
        const int maxInterval = std::max(limit, 2000 / (minRate+1));
        const int minInterval = std::max(limit, 2000 / (maxRate+1));
        return Range_t(minInterval, maxInterval);
    } else {
        return Range_t::fromValue(20);
    }
}

game::spec::Fighter::Range_t
game::spec::Fighter::getNumStrikes(const HostVersion& host, const game::config::HostConfiguration& config) const
{
    /* Strikes for PHost: this is explicit in the StrikesPerFighter. We do not model
       the speeds here; they can become significant, but usually aren't.

       Strikes for THost: fighters fire in [-19,+19] around the enemy, a 39 ly range.
       Fighters move at speed 4, units at speed 1, giving a difference speed of 4 or 5.
       This allows for 7 to 10 strikes. */
    if (host.isPHost()) {
        return Range_t::fromValue(config[config.StrikesPerFighter](getId()));
    } else {
        return Range_t(7, 10);
    }
}
