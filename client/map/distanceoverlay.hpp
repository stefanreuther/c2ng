/**
  *  \file client/map/distanceoverlay.hpp
  *  \brief Class client::map::DistanceOverlay
  */
#ifndef C2NG_CLIENT_MAP_DISTANCEOVERLAY_HPP
#define C2NG_CLIENT_MAP_DISTANCEOVERLAY_HPP

#include "afl/base/signalconnection.hpp"
#include "client/map/overlay.hpp"
#include "game/map/point.hpp"
#include "game/session.hpp"
#include "game/types.hpp"
#include "util/requestreceiver.hpp"

namespace client { namespace map {

    class Screen;
    class Location;

    /** Map overlay for distance mode.
        Computes the distance from a given location and shows it on the map. */
    class DistanceOverlay : public Overlay {
     public:
        /** Constructor.
            \param parent   Parent screen
            \param loc      Map Location object
            \param origin   Origin of measurement
            \param shipId   Ship to show information for; can be 0 */
        DistanceOverlay(Screen& parent, Location& loc, game::map::Point origin, game::Id_t shipId);
        ~DistanceOverlay();

        // Overlay:
        virtual void drawBefore(gfx::Canvas& can, const Renderer& ren);
        virtual void drawAfter(gfx::Canvas& can, const Renderer& ren);
        virtual bool drawCursor(gfx::Canvas& can, const Renderer& ren);
        virtual bool handleKey(util::Key_t key, int prefix, const Renderer& ren);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons, const Renderer& ren);

     private:
        enum Mode {
            DistanceMode,        ///< Not locked at a ship.
            WaypointMode,        ///< Origin is a ship we play, waypoint change possible.
            ForeignMode,         ///< Origin is a ship we don't play.
            OtherMode            ///< We have a ship but it's not at origin.
        };

        enum State {
            Idle,                ///< Status is current.
            Requesting,          ///< New request has been made.
            Retriggered          ///< New request has been made, but already obsolete.
        };

        struct Status {
            Mode mode;
            String_t distanceInfo;            ///< Distance.
            String_t flightInfo;              ///< ETAs, angle.
            String_t shipName;                ///< Name.
            String_t shipInfo;                ///< Real ETA.
            String_t fuelInfo;                ///< Fuel usage.
            uint8_t shipColor;                ///< Color for shipInfo line.
            uint8_t fuelColor;                ///< Color for fuel line.
            game::map::Point shipWaypoint;    ///< Ship waypoint if mode is WaypointMode.
            Status()
                : mode(), distanceInfo(), flightInfo(), shipInfo(), fuelInfo(), shipColor(), fuelColor(), shipWaypoint()
                { }
        };

        Screen& m_parent;
        Location& m_location;
        game::map::Point m_origin;
        game::Id_t m_shipId;
        Status m_status;
        bool m_first;
        State m_state;

        util::RequestReceiver<DistanceOverlay> m_reply;
        afl::base::SignalConnection conn_positionChange;

        void onPositionChange(game::map::Point pt);
        void maybeRequestStatus();
        void setStatus(Status st);
        void requestStatus();

        void swapEnds();
        void setWaypoint();

        static void buildStatus(Status& out, game::Session& session, game::map::Point origin, game::map::Point target, game::Id_t shipId);
    };

} }

#endif
