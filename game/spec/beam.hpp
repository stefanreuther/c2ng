/**
  *  \file game/spec/beam.hpp
  *  \brief Class game::spec::Beam
  */
#ifndef C2NG_GAME_SPEC_BEAM_HPP
#define C2NG_GAME_SPEC_BEAM_HPP

#include "game/spec/weapon.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/hostversion.hpp"

namespace game { namespace spec {

    /** A beam weapon.
        This class only holds data which it does not interpret or limit. */
    class Beam : public Weapon {
     public:
        /** Constructor.
            \param beam Id */
        explicit Beam(int id);

        /** Get average recharge time.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param forPlayer Player to ask this question for
            \param host Host version
            \param config Host configuration
            \return average time between firing twice */
        int getRechargeTime(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get beam hit odds.
            This is an estimation for spec displays.
            Combat algorithms will implement this internally.
            \param forPlayer Player to ask this question for
            \param host Host version
            \param config Host configuration
            \return hit odds (percentage) */
        int getHitOdds(int forPlayer, const HostVersion& host, const game::config::HostConfiguration& config) const;

        /** Get number of mines swept.
            \param forPlayer Player to ask this question for
            \param isWeb true for web mines
            \param config Host configuration
            \return number of mines swept by one beam */
        int getNumMinesSwept(int forPlayer, bool isWeb, const game::config::HostConfiguration& config) const;
    };

} }

#endif
