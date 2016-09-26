/**
  *  \file game/map/viewport.hpp
  */
#ifndef C2NG_GAME_MAP_VIEWPORT_HPP
#define C2NG_GAME_MAP_VIEWPORT_HPP

#include "game/teamsettings.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "game/map/point.hpp"
#include "afl/bits/smallset.hpp"

namespace game { namespace map {

    class Universe;

    class Viewport : afl::base::Uncopyable {
     public:
        enum Option {
            ShowIonStorms,      // co_Ion       = 1,
            ShowMinefields,     // co_Mine      = 2,
            ShowUfos,           // co_Ufo       = 4,
            ShowGrid,           // co_Sectors   = 8,
            ShowBorders,        // co_Borders   = 16,
            ShowDrawings,       // co_Drawings  = 32,
            ShowSelection,      // co_Selection = 64,
            ShowLabels,         // co_Labels    = 128,
            ShowTrails,         // co_Trails    = 256,
            // ShowShipDots,       // co_ShipDots  = 512,    // called co_NoTriangles in PCC 1.x
            ShowWarpWells       // co_WarpWells = 1024; xref all()
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        Viewport(Universe& univ, TeamSettings& teams);
        ~Viewport();

        Universe& universe();
        TeamSettings& teamSettings();

        void setRange(Point min, Point max);
        void setOption(Option opt, bool enable);

        Options_t getOptions() const;
        void setOptions(Options_t opts);

        Point getMin() const;
        Point getMax() const;
        bool hasOption(Option opt) const;

        bool containsCircle(Point origin, int radius) const;
        bool containsRectangle(Point a, Point b) const;
        bool containsLine(Point a, Point b) const;

        afl::base::Signal<void()> sig_update;

     private:
        void onChange();

        Universe& m_universe;
        TeamSettings& m_teamSettings;

        Point m_min;
        Point m_max;

        Options_t m_options;

        afl::base::SignalConnection conn_universeChange;
        afl::base::SignalConnection conn_teamChange;
    };

} }

#endif
