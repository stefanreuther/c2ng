/**
  *  \file game/sim/configuration.hpp
  */
#ifndef C2NG_GAME_SIM_CONFIGURATION_HPP
#define C2NG_GAME_SIM_CONFIGURATION_HPP

#include "game/playerbitmatrix.hpp"
#include "game/teamsettings.hpp"
#include "game/config/hostconfiguration.hpp"

namespace game { namespace sim {

    class Configuration {
     public:
        /** Balancing mode. Various algorithms to correct the TVCR
            left/right imbalance. */
        enum BalancingMode {
            BalanceNone,            ///< No balancing.
            Balance360k,            ///< 360kt bonus (HOST).
            BalanceMasterAtArms     ///< Master at Arms proposal.
        };
        /** VCR mode. Which VCR is used to run the simulation. */
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

        Configuration();

        void loadDefaults(const TeamSettings& teams);
        void setMode(VcrMode mode, const TeamSettings& teams, const game::config::HostConfiguration& config);
        bool isExperienceEnabled(const game::config::HostConfiguration& config) const;

        void setEngineShieldBonus(int n);
        int  getEngineShieldBonus() const;
        void setScottyBonus(bool enable);
        bool hasScottyBonus() const;
        void setRandomLeftRight(bool enable);
        bool hasRandomLeftRight() const;
        void setHonorAlliances(bool enable);
        bool hasHonorAlliances() const;
        void setOnlyOneSimulation(bool enable);
        bool hasOnlyOneSimulation() const;
        void setSeedControl(bool enable);
        bool hasSeedControl() const;
        void setRandomizeFCodesOnEveryFight(bool enable);
        bool hasRandomizeFCodesOnEveryFight() const;
        void setBalancingMode(BalancingMode mode);
        BalancingMode getBalancingMode() const;
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
