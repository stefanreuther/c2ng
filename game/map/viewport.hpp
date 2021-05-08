/**
  *  \file game/map/viewport.hpp
  */
#ifndef C2NG_GAME_MAP_VIEWPORT_HPP
#define C2NG_GAME_MAP_VIEWPORT_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/bits/smallset.hpp"
#include "game/map/point.hpp"
#include "game/teamsettings.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "game/spec/shiplist.hpp"
#include "game/config/hostconfiguration.hpp"
#include "util/atomtable.hpp"

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
            ShowShipDots,       // co_ShipDots  = 512,    // called co_NoTriangles in PCC 1.x
            ShowWarpWells,      // co_WarpWells = 1024,
            ShowMessages,       // co_Messages  = 2048

            ShowOutsideGrid,    // [fill] co_Sectors // inverted logic!

            FillIonStorms,      // [fill] co_Ion
            FillMinefields,     // [fill] co_Mine
            FillUfos            // [fill] co_Ufo
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        Viewport(Universe& univ, int turnNumber, TeamSettings& teams,
                 const UnitScoreDefinitionList& shipScoreDefinitions,
                 const game::spec::ShipList& shipList,
                 const game::config::HostConfiguration& config);
        ~Viewport();

        Universe& universe() const;
        TeamSettings& teamSettings() const;
        int getTurnNumber() const;
        const UnitScoreDefinitionList& shipScores() const;
        const game::spec::ShipList& shipList() const;
        const game::config::HostConfiguration& hostConfiguration() const;

        void setRange(Point min, Point max);
        void setOption(Option opt, bool enable);

        Options_t getOptions() const;
        void setOptions(Options_t opts);

        Point getMin() const;
        Point getMax() const;
        bool hasOption(Option opt) const;

        void setDrawingTagFilter(util::Atom_t tag);
        void clearDrawingTagFilter();
        bool isDrawingTagVisible(util::Atom_t tag) const;

        void setShipTrailId(Id_t id);
        Id_t getShipTrailId() const;

        bool containsCircle(Point origin, int radius) const;
        bool containsRectangle(Point a, Point b) const;
        bool containsLine(Point a, Point b) const;
        bool containsText(Point origin, const String_t& text) const;

        afl::base::Signal<void()> sig_update;

     private:
        void onChange();

        Universe& m_universe;
        TeamSettings& m_teamSettings;
        int m_turnNumber;
        const UnitScoreDefinitionList& m_shipScoreDefinitions;
        const game::spec::ShipList& m_shipList;
        const game::config::HostConfiguration& m_hostConfiguration;

        Point m_min;
        Point m_max;

        Options_t m_options;

        bool m_drawingTagFilterActive;
        util::Atom_t m_drawingTagFilter;
        Id_t m_shipTrailId;

        afl::base::SignalConnection conn_universeChange;
        afl::base::SignalConnection conn_teamChange;
    };

} }

#endif
