/**
  *  \file game/spec/torpedolauncher.hpp
  *  \brief Class game::spec::TorpedoLauncher
  */
#ifndef C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP
#define C2NG_GAME_SPEC_TORPEDOLAUNCHER_HPP

#include "game/spec/weapon.hpp"
#include "game/spec/cost.hpp"
#include "game/hostversion.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace spec {

    /** A torpedo launcher.
        This class only holds data which it does not interpret or limit.

        This is the primary data class for torpedo systems;
        class Torpedo can be used to access a single torpedo instead of the launcher. */
    class TorpedoLauncher : public Weapon {
     public:
        /** Constructor.
            \param id torpedo Id */
        explicit TorpedoLauncher(int id);

        /** Get torpedo cost.
            \return cost */
        Cost& torpedoCost();

        /** Get torpedo cost.
            \return cost
            \overload */
        const Cost& torpedoCost() const;

        /** Get average recharge time.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param forPlayer Player to ask this question for
            \param host Host version
            \param config Host configuration
            \return average time between firing twice */
        int getRechargeTime(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get torpedo hit odds.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param forPlayer Player to ask this question for
            \param host Host version
            \param config Host configuration
            \return hit odds (percentage) */
        int getHitOdds(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get cost for a minefield.
            \param [in] forPlayer Player to ask this question for
            \param [in] numMines Number of mine units. Should be a large number to minimize rounding errors.
            \param [in] isWeb true for web mines, false for regular
            \param [in] config Host configuration
            \param [out] result Result
            \retval true result has been set
            \retval false result not available (infinite due to configuration) */
        bool getMinefieldCost(int forPlayer, int numMines, bool isWeb, const game::config::HostConfiguration& config, Cost& result) const;

     private:
        Cost m_torpedoCost;
    };

} }

#endif
