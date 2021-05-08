/**
  *  \file u/t_game_vcr_flak_environment.cpp
  *  \brief Test for game::vcr::flak::Environment
  */

#include "game/vcr/flak/environment.hpp"

#include "t_game_vcr_flak.hpp"

/** Interface test. */
void
TestGameVcrFlakEnvironment::testInterface()
{
    class Tester : public game::vcr::flak::Environment {
     public:
        virtual int getConfiguration(ScalarOption /*index*/) const
            { return 0; }
        virtual int getConfiguration(ArrayOption /*index*/, int /*player*/) const
            { return 0; }
        virtual int getExperienceConfiguration(ExperienceOption /*index*/, int /*level*/, int /*player*/) const
            { return 0; }
        virtual int getBeamKillPower(int /*type*/) const
            { return 0; }
        virtual int getBeamDamagePower(int /*type*/) const
            { return 0; }
        virtual int getTorpedoKillPower(int /*type*/) const
            { return 0; }
        virtual int getTorpedoDamagePower(int /*type*/) const
            { return 0; }
        virtual int getPlayerRaceNumber(int /*player*/) const
            { return 0; }
    };
    Tester t;
}

