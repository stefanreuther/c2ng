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

        typedef TeamSettings::Relation Relation_t;

        virtual void drawGridLine(Point a, Point b) = 0;
        virtual void drawBorderLine(Point a, Point b) = 0;
        virtual void drawSelection(Point p) = 0;
        virtual void drawPlanet(Point p, int id, int flags) = 0;
        virtual void drawShip(Point p, int id, Relation_t rel) = 0;
        virtual void drawFleetLeader(Point pt, int id, Relation_t rel) = 0;
        virtual void drawMinefield(Point p, int id, int r, bool isWeb, Relation_t rel) = 0;
        virtual void drawUserCircle(Point pt, int r, int color) = 0;
        virtual void drawUserLine(Point a, Point b, int color) = 0;
        virtual void drawUserRectangle(Point a, Point b, int color) = 0;
        virtual void drawUserMarker(Point pt, int shape, int color, String_t label) = 0;
    };

} }

#endif
