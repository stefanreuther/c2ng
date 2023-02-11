/**
  *  \file game/map/rendererlistener.hpp
  *  \brief Interface game::map::RendererListener
  */
#ifndef C2NG_GAME_MAP_RENDERERLISTENER_HPP
#define C2NG_GAME_MAP_RENDERERLISTENER_HPP

#include "game/map/point.hpp"
#include "afl/base/deletable.hpp"
#include "game/teamsettings.hpp"

namespace game { namespace map {

    /** Renderer listener.
        Receives calls from a Renderer, to draw a map.
        The call sequence represents a Z order hierarchy, i.e. later calls draw "over" earlier calls;
        see Renderer class description.

        All calls use game coordinates.
        The transformation to possible graphics coordinates happens in the receiver.

        Calls do no longer contain object references to game data. */
    class RendererListener : public afl::base::Deletable {
     public:
        // Planet flags
        static const int ripUnowned            = 1;     ///< Planet is unowned.
        static const int ripOwnPlanet          = 2;     ///< Planet is owned by us.
        static const int ripAlliedPlanet       = 4;     ///< Planet is owned by ally.
        static const int ripEnemyPlanet        = 8;     ///< Planet is owned by enemy.
        static const int ripHasBase            = 16;    ///< Planet has a base.
        static const int ripOwnShips           = 32;    ///< Own ships in orbit.
        static const int ripAlliedShips        = 64;    ///< Allied ships in orbit.
        static const int ripEnemyShips         = 128;   ///< Enemy ships in orbit.
        static const int ripGuessedAlliedShips = 256;   ///< Guessed allied ships in orbit.
        static const int ripGuessedEnemyShips  = 512;   ///< Guessed enemy ships in orbit.

        // Ship flags
        static const int risShowDot     = 1;    ///< Show dot for this ship.
        static const int risShowIcon    = 2;    ///< Show icon for this ship.
        static const int risFleetLeader = 4;    ///< Show as fleet leader.
        static const int risAtPlanet    = 8;    ///< Ship is at planet; for placement of label (only set when label being used).

        // Ship trail flags
        static const int TrailFromPosition = 1; ///< If set, a (origin) is an actual position. If clear, it's just a heading it came from.
        static const int TrailToPosition   = 2; ///< If set, b (destination) is an actual position. If clear, it's just a heading it went to.

        /** Warp well edge. */
        enum Edge {
            North,
            East,
            South,
            West
        };

        typedef TeamSettings::Relation Relation_t;

        /** Draw a grid line (sector grid).
            @param a First point
            @param b Second point */
        virtual void drawGridLine(Point a, Point b) = 0;

        /** Draw a border line (rectangular map image boundary).
            If the map is rectangular, this is called for every edge of the rectangle.
            @param a First point
            @param b Second point */
        virtual void drawBorderLine(Point a, Point b) = 0;

        /** Draw a border circle (circular map image boundary).
            @param c Center point
            @param radius Radius */
        virtual void drawBorderCircle(Point c, int radius) = 0;

        /** Draw selection marker.
            Marks that there is at least one marked object at the given position.
            @param p Position */
        virtual void drawSelection(Point p) = 0;

        /** Draw message marker.
            Marks that there is at least one object that has messages at the given position.
            @param p Position */
        virtual void drawMessageMarker(Point p) = 0;

        /** Draw planet.
            @param p     Position
            @param id    Planet Id
            @param flags Flags; reports status of planet and units in orbit; combination of ripXxx flags
            @param label Label */
        virtual void drawPlanet(Point p, int id, int flags, String_t label) = 0;

        /** Draw ship.
            Note that ships might be drawn twice with different flags.
            @param p     Position
            @param id    Ship Id
            @param rel   Ownership relation
            @param flags Flags; combination of risXxx flags
            @param label Label */
        virtual void drawShip(Point p, int id, Relation_t rel, int flags, String_t label) = 0;

        /** Draw minefield.
            @param p      Center position
            @param id     Id
            @param r      Radius
            @param isWeb  true for web mines, false for regular mines
            @param rel    Ownership relation
            @param filled true if minefields should be drawn filled, false if empty */
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled) = 0;

        /** Draw Ufo.
            @param p         Center position
            @param id        Id
            @param r         Radius
            @param colorCode Color code (VGA color, [1,15])
            @param speed     Speed (warp factor)
            @param heading   Heading (degrees)
            @param filled    true if Ufos should be drawn filled, false if empty */
        virtual void drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled) = 0;

        /** Draw Ufo connection.
            If Ufos represent connected wormholes, this draws the connection between them.
            @param a         First Ufo center
            @param b         Second Ufo center
            @param colorCode Color code (VGA color, [1,15]) */
        virtual void drawUfoConnection(Point a, Point b, int colorCode) = 0;

        /** Draw Ion storm.
            @param p       Center position
            @param r       Radius
            @param voltage Voltage (determines color)
            @param speed   Speed (warp factor)
            @param heading Heading (degrees)
            @param filled  true if storms should be drawn filled, false if empty */
        virtual void drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled) = 0;

        /** Draw circle drawing.
            @param pt     Center position
            @param r      Radius
            @param color  Color [1,NUM_USER_COLORS] */
        virtual void drawUserCircle(Point pt, int r, int color) = 0;

        /** Draw line drawing.
            @param a      First point
            @param b      Second point
            @param color  Color [1,NUM_USER_COLORS] */
        virtual void drawUserLine(Point a, Point b, int color) = 0;

        /** Draw rectangle drawing.
            @param a      First point
            @param b      Second point
            @param color  Color [1,NUM_USER_COLORS] */
        virtual void drawUserRectangle(Point a, Point b, int color) = 0;

        /** Draw marker drawing.
            @param pt     Position
            @param shape  Shape [0,NUM_USER_MARKERS]
            @param color  Color [1,NUM_USER_COLORS]
            @param label  Label */
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label) = 0;

        /** Draw explosion marker.
            @param p Position */
        virtual void drawExplosion(Point p) = 0;

        /** Draw ship trail.
            @param a     First (older, from) position
            @param b     Second (newer, to) position
            @param rel   Ownership relation
            @param flags Flags, combination of TrailFromPosition/TrailToPosition, defines which of the positions is an actual scanned position
            @param age   Age of this trail (0=current) */
        virtual void drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age) = 0;

        /** Draw ship waypoint (planned movement order).
            @param a     First (current) position
            @param b     Second (waypoint) position
            @param rel   Ownership relation */
        virtual void drawShipWaypoint(Point a, Point b, Relation_t rel) = 0;

        /** Draw ship vector (scanned movement).
            @param a     First (current) position
            @param b     Second (waypoint) position
            @param rel   Ownership relation */
        virtual void drawShipVector(Point a, Point b, Relation_t rel) = 0;

        /** Draw warp well edge.
            Called repeatedly to draw warp well boundaries around planets.
            For this function, positions need to be treated as squares, we're drawing the sides of the square.
            @param a     Position
            @param e     Edge to draw */
        virtual void drawWarpWellEdge(Point a, Edge e) = 0;
    };

} }

#endif
