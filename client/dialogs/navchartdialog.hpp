/**
  *  \file client/dialogs/navchartdialog.hpp
  */
#ifndef C2NG_CLIENT_DIALOGS_NAVCHARTDIALOG_HPP
#define C2NG_CLIENT_DIALOGS_NAVCHARTDIALOG_HPP

#include "client/si/outputstate.hpp"
#include "client/si/userside.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "ui/root.hpp"
#include "util/requestsender.hpp"

namespace client { namespace dialogs {

    /** Navigation chart dialog status.
        Used to pass initial status into doNavigationChart, and to receive updated status. */
    struct NavChartState {
        // General config
        String_t title;                                   ///< Window title.
        game::map::Point center;                          ///< Center of map.
        game::map::Point origin;                          ///< Origin (=position of ship). Used for distance computations.
        game::map::Point target;                          ///< Scanner target.
        bool hyperjumping;                                ///< true if ship is hyperjumping.
        int speed;                                        ///< Speed (light-years per turn).
        game::Id_t shipId;                                ///< Initiating ship Id. Used for chunnel building.

        // Mostly runtime only
        bool chunnelMode;                                 ///< True if we are building a chunnel. May be active only if acceptChunnel is set.

        // Possible results
        bool acceptLocation;                              ///< If true, dialog can produce a NavChartResult::Location result.
        bool acceptShip;                                  ///< If true, dialog can produce a NavChartResult::Ship result.
        bool acceptChunnel;                               ///< If true, dialog can produce a NavChartResult::Chunnel result.

        // Ship limits
        bool requireOwnShip;
        game::Id_t excludeShip;

        NavChartState()
            : title(), center(), origin(), target(), hyperjumping(false), speed(0), chunnelMode(false),
              acceptLocation(false), acceptShip(false), acceptChunnel(false), requireOwnShip(false), excludeShip(0)
            { }
    };

    /** Result of navigation chart dialog.
        The dialog produces one such structure as output. */
    struct NavChartResult {
        // Dialog result
        enum Result {
            Canceled,                ///< Dialog was cancelled.
            Location,                ///< User selected a map location.
            Ship,                    ///< User selected a ship.
            Chunnel                  ///< User asked to set up a chunnel.
        };
        Result result;               ///< Result (always set).
        game::map::Point position;   ///< Location (always set unless result is Canceled).
        game::Id_t shipId;           ///< Ship Id (set for Ship, Chunnel).

        // Script-side result
        client::si::OutputState outputState;

        NavChartResult()
            : result(Canceled), position(), shipId(0), outputState()
            { }
    };


    /** Show navigation chart widget.
        \param result     [out] dialog result placed here
        \param in         [in/out] initial/updated dialog state
        \param us         [in] UserSide
        \param root       [in] UI root
        \param tx         [in] Translator
        \see NavChartState, NavChartResult */
    void doNavigationChart(NavChartResult& result,
                           NavChartState& in,
                           client::si::UserSide& us,
                           ui::Root& root,
                           afl::string::Translator& tx);

} }

#endif
