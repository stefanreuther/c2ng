/**
  *  \file game/vcr/flak/gameenvironment.hpp
  *  \brief Class game::vcr::flak::GameEnvironment
  */
#ifndef C2NG_GAME_VCR_FLAK_GAMEENVIRONMENT_HPP
#define C2NG_GAME_VCR_FLAK_GAMEENVIRONMENT_HPP

#include "game/config/hostconfiguration.hpp"
#include "game/spec/componentvector.hpp"
#include "game/vcr/flak/environment.hpp"

namespace game { namespace vcr { namespace flak {

    /** Environment instance for c2ng.
        Implements the FLAK environment using a HostConfiguration object and BeamVector_t/TorpedoVector_t. */
    class GameEnvironment : public Environment {
     public:
        /** Constructor.
            Note that this class stores references to its parameters, so they must live sufficiently long.
            \param config  Host configuration
            \param beams   Beam weapons
            \param torps   Torpedo launchers */
        GameEnvironment(const game::config::HostConfiguration& config, const game::spec::BeamVector_t& beams, const game::spec::TorpedoVector_t& torps);

        // Environment:
        virtual int getConfiguration(ScalarOption index) const;
        virtual int getConfiguration(ArrayOption index, int player) const;
        virtual int getExperienceConfiguration(ExperienceOption index, int level, int player) const;
        virtual int getBeamKillPower(int type) const;
        virtual int getBeamDamagePower(int type) const;
        virtual int getTorpedoKillPower(int type) const;
        virtual int getTorpedoDamagePower(int type) const;
        virtual int getPlayerRaceNumber(int player) const;

     private:
        const game::config::HostConfiguration& m_config;
        const game::spec::BeamVector_t& m_beams;
        const game::spec::TorpedoVector_t& m_torpedos;
    };

} } }

#endif
