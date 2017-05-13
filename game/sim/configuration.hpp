/**
  *  \file game/sim/configuration.hpp
  *  \brief Class game::sim::Configuration
  */
#ifndef C2NG_GAME_SIM_CONFIGURATION_HPP
#define C2NG_GAME_SIM_CONFIGURATION_HPP

#include "game/playerbitmatrix.hpp"
#include "game/teamsettings.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace sim {

    /** Simulator configuration. */
    class Configuration {
     public:
        /** Balancing mode.
            Various algorithms to correct the TVCR left/right imbalance. */
        enum BalancingMode {
            BalanceNone,            ///< No balancing.
            Balance360k,            ///< 360kt bonus (HOST).
            BalanceMasterAtArms     ///< Master at Arms proposal.
        };
        /** VCR mode.
            Which VCR is used to run the simulation. */
        enum VcrMode {
            VcrHost,                ///< HOST.
            VcrPHost2,              ///< PHost 2.x.
            VcrPHost3,              ///< PHost 3.x.
            VcrPHost4,              ///< PHost 4.x.
            VcrFLAK,                ///< FLAK.
            VcrNuHost               ///< NuHost.
        };

        // FIXME: needed?
        // enum {
        //     BalanceMAX = BalanceMasterAtArms,
        //     VcrMAX = VcrNuHost
        // };

        /** Default constructor. */
        Configuration();

        /** Load defaults.
            \param teams Teams (to initialize alliances from) */
        void loadDefaults(const TeamSettings& teams);

        /** Set mode (host version).
            Sets other parameters (ES-Bonus, Scotty bonus, left/right, balancing) to mode/configuration dependant defaults.
            \param mode Mode
            \param teams Team settings (for viewpoint player)
            \param config Host configuration */
        void setMode(VcrMode mode, const TeamSettings& teams, const game::config::HostConfiguration& config);

        /** Check enabled experience.
            \param config Host configuration
            \return true if experience enabled */
        bool isExperienceEnabled(const game::config::HostConfiguration& config) const;

        /** Set engine/shield bonus.
            \param n Bonus (percentage) */
        void setEngineShieldBonus(int n);

        /** Get engine/shield bonus.
            \return bonus (percentage) */
        int getEngineShieldBonus() const;

        /** Set scotty bonus.
            \param enable flag */
        void setScottyBonus(bool enable);

        /** Check for scotty bonus.
            \return true if enabled */
        bool hasScottyBonus() const;

        /** Set random left/right assignment.
            \param enable flag */
        void setRandomLeftRight(bool enable);

        /** Check for random left/right assignment.
            \return true if enabled */
        bool hasRandomLeftRight() const;

        /** Set whether alliances are honored.
            \param enable flag */
        void setHonorAlliances(bool enable);

        /** Check whether alliances are honored.
            \return true if enabled */
        bool hasHonorAlliances() const;

        /** Set limitation to one fight.
            \param enable flag */
        void setOnlyOneSimulation(bool enable);

        /** Check limitation to one fight.
            \return true if enabled */
        bool hasOnlyOneSimulation() const;

        /** Set seed control.
            \param enable flag */
        void setSeedControl(bool enable);

        /** Check for seed control.
            \return true if enabled */
        bool hasSeedControl() const;

        /** Set whether friendly codes are randomized on every fight.
            \param enable flag */
        void setRandomizeFCodesOnEveryFight(bool enable);

        /** Check whether friendly codes are randomized on every fight.
            \return true if enabled */
        bool hasRandomizeFCodesOnEveryFight() const;

        /** Set balancing mode.
            \param mode Mode */
        void setBalancingMode(BalancingMode mode);

        /** Get balancing mode.
            \return mode */
        BalancingMode getBalancingMode() const;

        /** Get simulation mode (host version).
            \return mode */
        VcrMode getMode() const;

     private:
        PlayerBitMatrix m_allianceSettings;             // ex alliance_settings
        PlayerBitMatrix m_enemySettings;                // ex enemy_settings
        int            m_engineShieldBonus;             // ex engine_shield_bonus;
        bool           m_scottyBonus;                   // ex scotty_bonus;
        bool           m_randomLeftRight;               // ex random_left_right;
        bool           m_honorAlliances;                // ex honor_alliances;
        bool           m_onlyOneSimulation;             // ex only_one_simulation;
        bool           m_seedControl;                   // ex seed_control;
        bool           m_randomizeFCodesOnEveryFight;   // ex randomize_fcodes_on_every_fight;
        BalancingMode  m_balancingMode;                 // ex balancing_mode;
        VcrMode        m_vcrMode;                       // ex vcr_mode;
        // int            m_seed; // FIXME: needed?
    };

} }

#endif
