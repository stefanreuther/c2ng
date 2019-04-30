/**
  *  \file game/map/cursors.hpp
  */
#ifndef C2NG_GAME_MAP_CURSORS_HPP
#define C2NG_GAME_MAP_CURSORS_HPP

#include "game/map/simpleobjectcursor.hpp"
#include "game/map/location.hpp"

namespace game { namespace map {

    class Universe;

    class Cursors {
     public:
        Cursors();
        ~Cursors();

        void setUniverse(Universe* univ);

        SimpleObjectCursor& currentShip();
        SimpleObjectCursor& currentPlanet();
        SimpleObjectCursor& currentBase();
        SimpleObjectCursor& currentIonStorm();
        Location& location();

        ObjectCursor* getCursorByNumber(int nr);
        ObjectType* getTypeByNumber(int nr);

        static const int ShipScreen = 1;
        static const int PlanetScreen = 2;
        static const int BaseScreen = 3;
        static const int IonStorms = 31;

     private:
        Universe* m_pUniverse;
        SimpleObjectCursor m_currentShip;
        SimpleObjectCursor m_currentPlanet;
        SimpleObjectCursor m_currentBase;
        SimpleObjectCursor m_currentIonStorm;
        Location m_location;
    };

} }

#endif
