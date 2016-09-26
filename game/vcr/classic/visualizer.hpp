/**
  *  \file game/vcr/classic/visualizer.hpp
  */
#ifndef C2NG_GAME_VCR_CLASSIC_VISUALIZER_HPP
#define C2NG_GAME_VCR_CLASSIC_VISUALIZER_HPP

#include "game/vcr/classic/types.hpp"
#include "afl/base/deletable.hpp"

namespace game { namespace vcr { namespace classic {

// /*! \class VcrVisualizer

//     The visualizer's methods are called by a VCR player to do actual
//     screen display. Not all events generate visualizer callbacks, hence
//     the visualisation code must also poll for changes (fighter positions,
//     damage/kill values, ...). It can use the accessor interface of
//     VcrPlayer for that. */
    /// Abstract VCR Visualizer.
    class Visualizer {
        // protected:
        //    VcrPlayer* ply;
     public:
        virtual void startFighter(Side side, int track) = 0;
        virtual void landFighter(Side side, int track) = 0;
        virtual void killFighter(Side side, int track) = 0;

        /* hit >= 0: hit, hit < 0: miss */
        /* track >= 0: fighter fires; < 0: beam fires; track = -1-beam nr. */
        virtual void fireBeam(Side side, int track, int target, int hit, int damage, int kill) = 0;
        virtual void fireTorpedo(Side side, int hit, int launcher) = 0;

        virtual void updateBeam(Side side, int id) = 0;
        virtual void updateLauncher(Side side, int id) = 0;

        virtual void killObject(Side side) = 0;

        // FIXME: needed?
        // virtual void redraw() = 0;
        // virtual void init() = 0;
    };

} } }


#endif
