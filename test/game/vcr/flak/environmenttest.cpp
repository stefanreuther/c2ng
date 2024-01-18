/**
  *  \file test/game/vcr/flak/environmenttest.cpp
  *  \brief Test for game::vcr::flak::Environment
  */

#include "game/vcr/flak/environment.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.vcr.flak.Environment")
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
