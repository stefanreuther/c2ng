/**
  *  \file game/map/viewport.hpp
  *  \brief Class game::map::Viewport
  */
#ifndef C2NG_GAME_MAP_VIEWPORT_HPP
#define C2NG_GAME_MAP_VIEWPORT_HPP

#include "afl/base/signal.hpp"
#include "afl/base/signalconnection.hpp"
#include "afl/base/uncopyable.hpp"
#include "afl/bits/smallset.hpp"
#include "game/config/hostconfiguration.hpp"
#include "game/interface/labelextra.hpp"
#include "game/interface/taskwaypoints.hpp"
#include "game/map/point.hpp"
#include "game/spec/shiplist.hpp"
#include "game/teamsettings.hpp"
#include "game/unitscoredefinitionlist.hpp"
#include "util/atomtable.hpp"

namespace game { namespace map {

    class Universe;
    class Configuration;

    /** Starchart viewport.
        Contains range and display options for a starchart rendering,
        and links to required objects. */
    class Viewport : afl::base::Uncopyable {
     public:
        /** Option. */
        enum Option {
            ShowIonStorms,      ///< Show ion storms.                         ex co_Ion = 1
            ShowMinefields,     ///< Show minefields.                         ex co_Mine = 2
            ShowUfos,           ///< Show Ufos.                               ex co_Ufo = 4
            ShowGrid,           ///< Show sector borders.                     ex co_Sectors = 8
            ShowBorders,        ///< Show map borders.                        ex co_Borders = 16
            ShowDrawings,       ///< Show user drawings.                      ex co_Drawings = 32
            ShowSelection,      ///< Show selections.                         ex co_Selection = 64
            ShowLabels,         ///< Show unit labels.                        ex co_Labels = 128
            ShowTrails,         ///< Show ship trails.                        ex co_Trails = 256
            ShowShipDots,       ///< Show ships as dots (default: triangles). ex co_ShipDots = 512 / co_NoTriangles
            ShowWarpWells,      ///< Show warp wells.                         ex co_WarpWells = 1024
            ShowMessages,       ///< Show message markers.                    ex co_Messages = 2048
            ShowMineDecay,      ///< Show minefields after decay.

            ShowOutsideGrid,    ///< Show outside grid. ex co_Sectors, inverted logic, "fill" option.

            FillIonStorms,      ///< Fill ion storms. ex co_Ion, "fill" option.
            FillMinefields,     ///< Fill minefields. ex co_Mine, "fill" option.
            FillUfos            ///< Fill Ufos.       ex co_Ufo, "fill" option.
        };
        typedef afl::bits::SmallSet<Option> Options_t;

        /** Constructor.
            All objects are read only, and must live longer than the Viewport.

            The Viewport object will not have a position range set; use setRange() to define one.

            @param univ                  Universe (non-const to access ObjectType::getObjectByIndex() which is non-const
            @param turnNumber            Turn number (for ship trails)
            @param teams                 Team settings (for player relations)
            @param labels                Optional LabelExtra. If not specified, labels will not be rendered (non-const to attach listeners)
            @param tasks                 Optional TaskWaypoints. If not specified, auto tasks will not be rendered.
            @param shipScoreDefinitions  Ship score definitions (for hull functions/gravitonic)
            @param shipList              Ship list (for hull functions/gravitonic)
            @param mapConfig             Map configuration
            @param config                Host configuration
            @param host                  Host version (for minefield decay) */
        Viewport(Universe& univ, int turnNumber, const TeamSettings& teams,
                 game::interface::LabelExtra* labels,
                 const game::interface::TaskWaypoints* tasks,
                 const UnitScoreDefinitionList& shipScoreDefinitions,
                 const game::spec::ShipList& shipList,
                 const Configuration& mapConfig,
                 const game::config::HostConfiguration& config,
                 HostVersion host);

        /** Destructor. */
        ~Viewport();

        /** Access Universe.
            @return universe */
        Universe& universe() const;

        /** Access team settings.
            @return team settings */
        const TeamSettings& teamSettings() const;

        /** Access LabelExtra.
            @return LabelExtra */
        const game::interface::LabelExtra* labels() const;

        /** Access TaskWaypoints.
            @return TaskWaypoints */
        const game::interface::TaskWaypoints* tasks() const;

        /** Get turn number.
            @return turn number */
        int getTurnNumber() const;

        /** Access ship score definitions.
            @return ship score definitions */
        const UnitScoreDefinitionList& shipScores() const;

        /** Access ship list.
            @return ship list */
        const game::spec::ShipList& shipList() const;

        /** Access map configuration.
            @return map configuration */
        const Configuration& mapConfiguration() const;

        /** Access host configuration.
            @return host configuration */
        const game::config::HostConfiguration& hostConfiguration() const;

        /** Access host version.
            @return host version */
        const HostVersion& hostVersion() const;

        /** Set position range.
            @param min Minimum (south/west)
            @param max Maximum (north/east) */
        void setRange(Point min, Point max);

        /** Get minimum (south/west) coordinate.
            @return coordinate */
        Point getMin() const;

        /** Get maximum (north/east) coordinate.
            @return coordinate */
        Point getMax() const;

        /** Set option.
            @param opt    Option
            @param enable Value of option */
        void setOption(Option opt, bool enable);

        /** Get all options.
            @return option set */
        Options_t getOptions() const;

        /** Set all options.
            @param opts Option set */
        void setOptions(Options_t opts);

        /** Check option value.
            @param opt Option to check
            @return value */
        bool hasOption(Option opt) const;

        /** Set drawing tag filter.
            Only drawings with this tag will be shown.
            @param tag Tag to show */
        void setDrawingTagFilter(util::Atom_t tag);

        /** Clear drawing tag filter.
            All drawings will be shown. */
        void clearDrawingTagFilter();

        /** Check whether drawing is visible.
            @param tag Drawing tag
            @return true if drawing shall be drawn */
        bool isDrawingTagVisible(util::Atom_t tag) const;

        /** Set ship trail Id.
            If nonzero, this ship's trail will be shown even if ShowTrails is off.
            @param id Ship Id */
        void setShipTrailId(Id_t id);

        /** Get ship trail Id.
            @return Id */
        Id_t getShipTrailId() const;

        /** Set ship Id for which not to show auto task.
            @param id Ship Id */
        void setShipIgnoreTaskId(Id_t id);

        /** Get ship Id for which not to show auto task.
            @return Ship Id */
        Id_t getShipIgnoreTaskId() const;

        /** Check whether circle is visible.
            @param origin Center
            @param radius Radius
            @return true if circle is visible */
        bool containsCircle(Point origin, int radius) const;

        /** Check whether rectangle is visible.
            @param a First point
            @param b Second point
            @return true if rectangle is visible */
        bool containsRectangle(Point a, Point b) const;

        /** Check whether line is visible.
            @param a First point
            @param b Second point
            @return true if line is visible */
        bool containsLine(Point a, Point b) const;

        /** Check whether text is visible.
            Because we do not know font metrics, this is just an estimate.
            @param origin Origin (center)
            @param text   Text
            @return true if text is visible */
        bool containsText(Point origin, const String_t& text) const;

        /** Signal: update.
            Emitted if any option changes that requires the starchart to be redrawn. */
        afl::base::Signal<void()> sig_update;

     private:
        void onChange();
        void onLabelChange(bool flag);

        Universe& m_universe;
        const TeamSettings& m_teamSettings;
        const game::interface::LabelExtra* m_labels;
        const game::interface::TaskWaypoints* m_tasks;
        const int m_turnNumber;
        const UnitScoreDefinitionList& m_shipScoreDefinitions;
        const game::spec::ShipList& m_shipList;
        const Configuration& m_mapConfig;
        const game::config::HostConfiguration& m_hostConfiguration;
        const HostVersion m_hostVersion;

        Point m_min;
        Point m_max;

        Options_t m_options;

        bool m_drawingTagFilterActive;
        util::Atom_t m_drawingTagFilter;
        Id_t m_shipTrailId;
        Id_t m_shipIgnoreTaskId;

        afl::base::SignalConnection conn_universeChange;
        afl::base::SignalConnection conn_teamChange;
        afl::base::SignalConnection conn_labelChange;
    };

} }

#endif
