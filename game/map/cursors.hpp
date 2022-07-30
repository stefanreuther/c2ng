/**
  *  \file game/map/cursors.hpp
  */
#ifndef C2NG_GAME_MAP_CURSORS_HPP
#define C2NG_GAME_MAP_CURSORS_HPP

#include "game/map/simpleobjectcursor.hpp"
#include "game/map/location.hpp"

namespace game { namespace map {

    class Universe;
    class Configuration;

    class Cursors {
     public:
        Cursors();
        ~Cursors();

        void setUniverse(Universe* univ, const Configuration* mapConfig);

        SimpleObjectCursor& currentShip();
        SimpleObjectCursor& currentHistoryShip();
        SimpleObjectCursor& currentPlanet();
        SimpleObjectCursor& currentBase();
        SimpleObjectCursor& currentFleet();
        SimpleObjectCursor& currentUfo();
        SimpleObjectCursor& currentIonStorm();
        SimpleObjectCursor& currentMinefield();
        Location& location();

        ObjectCursor* getCursorByNumber(int nr);
        ObjectType* getTypeByNumber(int nr);

        static const int ShipScreen = 1;
        static const int PlanetScreen = 2;
        static const int BaseScreen = 3;
        static const int HistoryScreen = 6;
        static const int FleetScreen = 10;
        static const int AllShips = 21;
        static const int AllPlanets = 22;
        static const int Ufos = 30;
        static const int IonStorms = 31;
        static const int Minefields = 32;

     private:
        Universe* m_pUniverse;
        SimpleObjectCursor m_currentShip;
        SimpleObjectCursor m_currentHistoryShip;
        SimpleObjectCursor m_currentPlanet;
        SimpleObjectCursor m_currentBase;
        SimpleObjectCursor m_currentFleet;
        SimpleObjectCursor m_currentUfo;
        SimpleObjectCursor m_currentIonStorm;
        SimpleObjectCursor m_currentMinefield;
        Location m_location;
    };

} }

#endif
