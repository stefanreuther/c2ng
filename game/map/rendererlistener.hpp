/**
  *  \file game/map/rendererlistener.hpp
  */
#ifndef C2NG_GAME_MAP_RENDERERLISTENER_HPP
#define C2NG_GAME_MAP_RENDERERLISTENER_HPP

#include "game/map/point.hpp"
#include "afl/base/deletable.hpp"
#include "game/teamsettings.hpp"

namespace game { namespace map {

    class RendererListener : public afl::base::Deletable {
     public:
        static const int ripUnowned            = 1;     // Planet is unowned.
        static const int ripOwnPlanet          = 2;     // Planet is owned by us.
        static const int ripAlliedPlanet       = 4;     // Planet is owned by ally.
        static const int ripEnemyPlanet        = 8;     // Planet is owned by enemy.
        static const int ripHasBase            = 16;    // Planet has a base
        static const int ripOwnShips           = 32;    // Own ships in orbit
        static const int ripAlliedShips        = 64;    // Allied ships in orbit
        static const int ripEnemyShips         = 128;   // Enemy ships in orbit
        static const int ripGuessedAlliedShips = 256;   // Guessed allied ships in orbit
        static const int ripGuessedEnemyShips  = 512;   // Guessed enemy ships in orbit

        static const int risShowDot     = 1;    // Show dot for this ship
        static const int risShowIcon    = 2;    // Show icon for this ship
        static const int risFleetLeader = 4;    // Show as fleet leader
        static const int risAtPlanet    = 8;    // Ship is at planet; for placement of label (only set when label being used)

        static const int TrailFromPosition = 1; // If set, a (origin) is an actual position. If clear, it's just a heading it came from.
        static const int TrailToPosition   = 2; // If set, b (destination) is an actual position. If clear, it's just a heading it went to.

        enum Edge {
            North,
            East,
            South,
            West
        };

        typedef TeamSettings::Relation Relation_t;

        virtual void drawGridLine(Point a, Point b) = 0;
        virtual void drawBorderLine(Point a, Point b) = 0;
        virtual void drawBorderCircle(Point c, int radius) = 0;
        virtual void drawSelection(Point p) = 0;
        virtual void drawMessageMarker(Point a) = 0;
        virtual void drawPlanet(Point p, int id, int flags, String_t label) = 0;
        virtual void drawShip(Point p, int id, Relation_t rel, int flags, String_t label) = 0;
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel, bool filled) = 0;
        virtual void drawUfo(Point p, int id, int r, int colorCode, int speed, int heading, bool filled) = 0;
        virtual void drawUfoConnection(Point a, Point b, int colorCode) = 0;
        virtual void drawIonStorm(Point p, int r, int voltage, int speed, int heading, bool filled) = 0;
        virtual void drawUserCircle(Point pt, int r, int color) = 0;
        virtual void drawUserLine(Point a, Point b, int color) = 0;
        virtual void drawUserRectangle(Point a, Point b, int color) = 0;
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label) = 0;
        virtual void drawExplosion(Point p) = 0;
        virtual void drawShipTrail(Point a, Point b, Relation_t rel, int flags, int age) = 0;
        virtual void drawShipWaypoint(Point a, Point b, Relation_t rel) = 0;
        virtual void drawShipVector(Point a, Point b, Relation_t rel) = 0;
        virtual void drawWarpWellEdge(Point a, Edge e) = 0;
    };

} }

#endif
