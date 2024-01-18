/**
  *  \file test/game/vcr/classic/visualizertest.cpp
  *  \brief Test for game::vcr::classic::Visualizer
  */

#include "game/vcr/classic/visualizer.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("game.vcr.classic.Visualizer")
{
    using game::vcr::classic::Side;
    using game::vcr::classic::Algorithm;
    class Tester : public game::vcr::classic::Visualizer {
     public:
        virtual void startFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
            { }
        virtual void landFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
            { }
        virtual void killFighter(Algorithm& /*algo*/, Side /*side*/, int /*track*/)
            { }
        virtual void fireBeam(Algorithm& /*algo*/, Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int /*damage*/, int /*kill*/)
            { }
        virtual void fireTorpedo(Algorithm& /*algo*/, Side /*side*/, int /*hit*/, int /*launcher*/)
            { }
        virtual void updateBeam(Algorithm& /*algo*/, Side /*side*/, int /*id*/)
            { }
        virtual void updateLauncher(Algorithm& /*algo*/, Side /*side*/, int /*id*/)
            { }
        virtual void killObject(Algorithm& /*algo*/, Side /*side*/)
            { }
    };
    Tester t;
}
