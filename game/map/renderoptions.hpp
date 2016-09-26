/**
  *  \file game/map/renderoptions.hpp
  */
#ifndef C2NG_GAME_MAP_RENDEROPTIONS_HPP
#define C2NG_GAME_MAP_RENDEROPTIONS_HPP

#include "afl/bits/smallset.hpp"
#include "afl/base/signal.hpp"

namespace game { namespace map {

    class RenderOptions {
     public:
        enum Option {
            ShowIonStorms,      // co_Ion       = 1,
            ShowMinefields,     // co_Mine      = 2,
            ShowUfos,           // co_Ufo       = 4,
            ShowSectors,        // co_Sectors   = 8,
            ShowBorders,        // co_Borders   = 16,
            ShowDrawings,       // co_Drawings  = 32,
            ShowSelection,      // co_Selection = 64,
            ShowLabels,         // co_Labels    = 128,
            ShowTrails,         // co_Trails    = 256,
            ShowShipDots,       // co_ShipDots  = 512,    // called co_NoTriangles in PCC 1.x
            ShowWarpWells       // co_WarpWells = 1024; xref all()
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        RenderOptions();

        void toggleOptions(Options_t which);
        void copyFrom(const RenderOptions& opts);
        void copyValues(const RenderOptions& opts, Options_t mask);
        void setOption(Options_t which);

        afl::base::Signal<void()> sig_change;

     private:
        Options_t m_show;
        Options_t m_fill;

        static Options_t all();
        static Options_t tristate();
        static Options_t defaults();
    };

} }

#endif
