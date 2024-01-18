/**
  *  \file test/game/vcr/classic/algorithmtest.cpp
  *  \brief Test for game::vcr::classic::Algorithm
  */

#include "game/vcr/classic/algorithm.hpp"

#include "afl/test/testrunner.hpp"
#include "game/vcr/classic/nullvisualizer.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.vcr.classic.Algorithm")
{
    using game::vcr::Object;
    using game::vcr::classic::Side;
    class Tester : public game::vcr::classic::Algorithm {
     public:
        Tester(game::vcr::classic::Visualizer& vis)
            : Algorithm(vis)
            { }
        virtual bool checkBattle(Object& /*left*/, Object& /*right*/, uint16_t& /*seed*/)
            { return false; }
        virtual void initBattle(const Object& /*left*/, const Object& /*right*/, uint16_t /*seed*/)
            { }
        virtual void doneBattle(Object& /*left*/, Object& /*right*/)
            { }
        virtual bool setCapabilities(uint16_t /*cap*/)
            { return false; }
        virtual bool playCycle()
            { return false; }
        virtual void playFastForward()
            { }
        virtual int getBeamStatus(Side /*side*/, int /*id*/)
            { return 0; }
        virtual int getLauncherStatus(Side /*side*/, int /*id*/)
            { return 0; }
        virtual int getNumTorpedoes(Side /*side*/)
            { return 0; }
        virtual int getNumFighters(Side /*side*/)
            { return 0; }
        virtual int getShield(Side /*side*/)
            { return 0; }
        virtual int getDamage(Side /*side*/)
            { return 0; }
        virtual int getCrew(Side /*side*/)
            { return 0; }
        virtual int getFighterX(Side /*side*/, int /*id*/)
            { return 0; }
        virtual game::vcr::classic::FighterStatus getFighterStatus(Side /*side*/, int /*id*/)
            { return game::vcr::classic::FighterIdle; }
        virtual int getObjectX(Side /*side*/)
            { return 0; }
        virtual int32_t getDistance()
            { return 0; }
        virtual game::vcr::classic::StatusToken* createStatusToken()
            { return 0; }
        virtual void restoreStatus(const game::vcr::classic::StatusToken& /*token*/)
            { }
        virtual game::vcr::classic::Time_t getTime()
            { return game::vcr::classic::Time_t(); }
        virtual game::vcr::classic::BattleResult_t getResult()
            { return game::vcr::classic::BattleResult_t(); }
        virtual game::vcr::Statistic getStatistic(Side /*side*/)
            { return game::vcr::Statistic(); }
    };
    game::vcr::classic::NullVisualizer vis;
    Tester t(vis);
}
