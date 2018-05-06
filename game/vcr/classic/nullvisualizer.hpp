/**
  *  \file game/vcr/classic/nullvisualizer.hpp
  *  \brief Class game::vcr::classic::NullVisualizer
  */
#ifndef C2NG_GAME_VCR_CLASSIC_NULLVISUALIZER_HPP
#define C2NG_GAME_VCR_CLASSIC_NULLVISUALIZER_HPP

#include "game/vcr/classic/visualizer.hpp"

namespace game { namespace vcr { namespace classic {

    /** Null Visualizer.
        Ignores all calls. */
    class NullVisualizer : public Visualizer {
     public:
        /** Constructor. */
        NullVisualizer();

        /** Destructor. */
        ~NullVisualizer();

        // Visualizer methods:
        void startFighter(Algorithm& algo, Side side, int track);
        void landFighter(Algorithm& algo, Side side, int track);
        void killFighter(Algorithm& algo, Side side, int track);

        void fireBeam(Algorithm& algo, Side side, int track, int target, int hit, int damage, int kill);
        void fireTorpedo(Algorithm& algo, Side side, int hit, int launcher);

        void updateBeam(Algorithm& algo, Side side, int id);
        void updateLauncher(Algorithm& algo, Side side, int id);

        void killObject(Algorithm& algo, Side side);
    };

} } }

#endif
