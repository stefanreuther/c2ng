/**
  *  \file game/ref/historyshipselection.hpp
  *  \brief Class game::ref::HistoryShipSelection
  */
#ifndef C2NG_GAME_REF_HISTORYSHIPSELECTION_HPP
#define C2NG_GAME_REF_HISTORYSHIPSELECTION_HPP

#include "afl/bits/smallset.hpp"
#include "afl/string/translator.hpp"
#include "game/map/universe.hpp"
#include "game/ref/historyshiplist.hpp"
#include "game/session.hpp"
#include "game/teamsettings.hpp"
#include "game/turn.hpp"

namespace game { namespace ref {

    /** History ship selection.
        Describes a selection of history ships for display and allows populating a HistoryShipList from it.

        To use,
        - use setPosition() if desired
        - call getAvailableModes(), getInitialMode() to determine a mode
        - call setMode() to set the mode
        - call buildList() to build a result list

        This is a data class that doesn't keep and references and can be passed between threads. */
    class HistoryShipSelection {
     public:
        /** Filter mode. */
        enum Mode {
            AllShips,           ///< Show all ships.
            LocalShips,         ///< Show ships that are/were near a position.
            ExactShips,         ///< Show ships that are/were exactly at a position.
            ForeignShips,       ///< Show foreign ships (not mine).
            TeamShips,          ///< Show team ships (same team).
            EnemyShips,         ///< Show enemy ships (different team).
            OwnShips            ///< Show own ships (mine).
        };
        typedef afl::bits::SmallSet<Mode> Modes_t;

        /** Sort mode. */
        enum SortOrder {
            ById,               ///< Sort by ship Id.
            ByOwner,            ///< Sort by ship owner.
            ByHull,             ///< Sort by hull type.
            ByAge,              ///< Sort by age of scan.
            ByName              ///< Sort by name.
        };

        static const size_t ModeMax = static_cast<size_t>(OwnShips);
        static const size_t SortMax = static_cast<size_t>(ByName);

        /** Constructor.
            Initialize with default. */
        HistoryShipSelection();

        /** Destructor. */
        ~HistoryShipSelection();

        /** Set filter mode.
            \param m Mode */
        void setMode(Mode m);

        /** Get filter mode.
            \return mode */
        Mode getMode() const;

        /** Set sort order.
            \param o Order */
        void setSortOrder(SortOrder o);

        /** Get sort order.
            \return order */
        SortOrder getSortOrder() const;

        /** Set reference position.
            This enables the LocalShips/ExactShips modes.
            \param pos Position */
        void setPosition(game::map::Point pos);

        /** Build list of ships.
            \param [out] list    Result
            \param [in]  turn    Turn to work on. Contains turn number and actual ships.
            \param [in]  session Session. Needs all components (game: team settings; root: players for ByOwner; ship list: for ByHull) */
        void buildList(HistoryShipList& list, const Turn& turn, Session& session) const;

        /** Get available filter modes.
            Those depend on
            - whether a position has been set (setPosition: LocalShips/ExactShips)
            - whether teams are configured (EnemyShips/TeamShips)
            - whether appropriate ships actually exist
            The result can be empty if there aren't any ships at all; otherwise, the AllShips mode is always available.

            Setting a mode not contained in the result will produce an empty list,
            or a list identical to one also attainable with a different mode (i.e., when there are no teams, TeamShips and OwnShips are the same).

            \param univ  Universe
            \param mapConfig Map configuration
            \param teams Team settings
            \return Available modes */
        Modes_t getAvailableModes(const game::map::Universe& univ, const game::map::Configuration& mapConfig, const TeamSettings& teams) const;

        /** Get initial mode.
            Suggest an initial mode for the current situation.
            \param univ  Universe
            \param mapConfig Map configuration
            \param teams Team settings
            \return mode */
        Mode getInitialMode(const game::map::Universe& univ, const game::map::Configuration& mapConfig, const TeamSettings& teams) const;

        /** Get name of a given mode.
            \param mode Mode
            \param tx   Translator
            \return name */
        String_t getModeName(Mode mode, afl::string::Translator& tx) const;

        /** Get name of current mode.
            \param tx   Translator
            \return name */
        String_t getModeName(afl::string::Translator& tx) const;

        /** Get name of a given sort order.
            \param sort Sort order
            \param tx   Translator
            \return name */
        static String_t getSortOrderName(SortOrder sort, afl::string::Translator& tx);

        /** Get name of current sort order.
            \param tx   Translator
            \return name */
        String_t getSortOrderName(afl::string::Translator& tx) const;

     private:
        Mode m_mode;
        SortOrder m_sortOrder;
        bool m_positionValid;
        game::map::Point m_position;

        bool isInRange(int x, int y, const game::map::Configuration& mapConfig) const;
    };

} }

#endif
