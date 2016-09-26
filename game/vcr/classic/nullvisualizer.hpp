/**
  *  \file game/vcr/classic/nullvisualizer.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_NULLVISUALIZER_HPP
#define C2NG_GAME_VCR_CLASSIC_NULLVISUALIZER_HPP

#include "game/vcr/classic/visualizer.hpp"

namespace game { namespace vcr { namespace classic {

// /** \class VcrNullVisualizer
//     \brief Null Visualizer

//     This VCR visualizer does nothing. It is to be used for
//     simulations, etc. */
    class NullVisualizer : public Visualizer {
     public:
        NullVisualizer();

        ~NullVisualizer();

        void startFighter(Side side, int track);
        void landFighter(Side side, int track);
        void killFighter(Side side, int track);

        void fireBeam(Side side, int track, int target, int hit, int damage, int kill);
        void fireTorpedo(Side side, int hit, int launcher);

        void updateBeam(Side side, int id);
        void updateLauncher(Side side, int id);

        void killObject(Side side);

        // void redraw();
        // void init();
    };

} } }

#endif
