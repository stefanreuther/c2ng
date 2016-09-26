/**
  *  \file u/t_game_vcr_classic_visualizer.cpp
  *  \brief Test for game::vcr::classic::Visualizer
  */

#include "game/vcr/classic/visualizer.hpp"

#include "t_game_vcr_classic.hpp"

/** Interface test. */
void
TestGameVcrClassicVisualizer::testIt()
{
    using game::vcr::classic::Side;
    class Tester : public game::vcr::classic::Visualizer {
     public:
        virtual void startFighter(Side /*side*/, int /*track*/)
            { }
        virtual void landFighter(Side /*side*/, int /*track*/)
            { }
        virtual void killFighter(Side /*side*/, int /*track*/)
            { }
        virtual void fireBeam(Side /*side*/, int /*track*/, int /*target*/, int /*hit*/, int /*damage*/, int /*kill*/)
            { }
        virtual void fireTorpedo(Side /*side*/, int /*hit*/, int /*launcher*/)
            { }
        virtual void updateBeam(Side /*side*/, int /*id*/)
            { }
        virtual void updateLauncher(Side /*side*/, int /*id*/)
            { }
        virtual void killObject(Side /*side*/)
            { }
    };
    Tester t;
}

