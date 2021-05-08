/**
  *  \file game/vcr/flak/environment.hpp
  *  \brief Interface game::vcr::flak::Environment
  */
#ifndef C2NG_GAME_VCR_FLAK_ENVIRONMENT_HPP
#define C2NG_GAME_VCR_FLAK_ENVIRONMENT_HPP

namespace game { namespace vcr { namespace flak {

    /** Environment abstraction.

        While the FLAK core was originally written to fit into the PCC2 kernel,
        care has been taken to isolate all environment access to ease replacing the actual back-end.
        For the original FLAK implementation, this was obtained by a set of compile-time defines.
        For the c2ng implementation, environment adaption happens through this class. */
    class Environment {
     public:
        /** Scalar configuration options (integer/bool). */
        enum ScalarOption {
            AllowAlternativeCombat,
            FireOnAttackFighters,
            StandoffDistance
        };

        /** Array-ized configuration options (per-player). */
        enum ArrayOption {
            BayLaunchInterval,
            BeamFiringRange,
            BeamHitShipCharge,
            FighterFiringRange,
            FighterKillOdds,
            ShipMovementSpeed,
            TorpFiringRange
        };

        /** Experience-modified configuration options (per-player and per-level). */
        enum ExperienceOption {
            BayRechargeBonus,
            BayRechargeRate,
            BeamHitBonus,
            BeamHitFighterCharge,
            BeamHitOdds,
            BeamRechargeBonus,
            BeamRechargeRate,
            CrewKillScaling,
            FighterBeamExplosive,
            FighterBeamKill,
            FighterMovementSpeed,
            HullDamageScaling,
            ShieldDamageScaling,
            ShieldKillScaling,
            StrikesPerFighter,
            TorpHitBonus,
            TorpHitOdds,
            TubeRechargeBonus,
            TubeRechargeRate
        };

        /** Get configuration scalar.
            \param index Option
            \return value */
        virtual int getConfiguration(ScalarOption index) const = 0;

        /** Get configuration array value.
            \param index Option
            \param player Player
            \return player's value */
        virtual int getConfiguration(ArrayOption index, int player) const = 0;

        /** Get experience configuration.
            Look up player's base option, add option modificator according to level.
            \param index   Option
            \param level   Experience level
            \param player  Player
            \return computed value */
        virtual int getExperienceConfiguration(ExperienceOption index, int level, int player) const = 0;

        /** Get beam kill power.
            \param type Type
            \return kill power
            \see game::spec::Beam::getKillPower(); PDK: BeamKillPower() */
        virtual int getBeamKillPower(int type) const = 0;

        /** Get beam damage power.
            \param type Type
            \return damage power
            \see game::spec::Beam::getDamagePower(); PDK: BeamDestructivePower() */
        virtual int getBeamDamagePower(int type) const = 0;

        /** Get torpedo kill power.
            \param type Type
            \return kill power
            \see game::spec::TorpedoLauncher::getKillPower(); PDK: TorpKillPower() */
        virtual int getTorpedoKillPower(int type) const = 0;

        /** Get beam damage power.
            \param type Type
            \return damage power
            \see game::spec::TorpedoLauncher::getDamagePower(); PDK: TorpDestructivePower() */
        virtual int getTorpedoDamagePower(int type) const = 0;

        /** Get player race number.
            \param player Player number
            \return race number
            \see game::config::HostConfiguration::getPlayerRaceNumber() */
        virtual int getPlayerRaceNumber(int player) const = 0;
    };

} } }

#endif
