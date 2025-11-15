/**
  *  \file game/map/cursors.hpp
  *  \brief Class game::map::Cursors
  */
#ifndef C2NG_GAME_MAP_CURSORS_HPP
#define C2NG_GAME_MAP_CURSORS_HPP

#include "game/map/location.hpp"
#include "game/map/simpleobjectcursor.hpp"
#include "game/reference.hpp"

namespace game { namespace map {

    class Universe;
    class Configuration;

    /** Cursors.
        Aggregates object cursors and object types for all user-visible object selections.
        The cursors drive UI object selection, and the "Iterator" script functionality.
        The object types represent UI object sets; not every one has an associated cursor. */
    class Cursors {
     public:
        /** Default constructor. */
        Cursors();

        /** Destructor. */
        ~Cursors();

        /** Set universe.
            Makes the cursors drive the given universe, with the given map configuration.
            Setting parameters to null makes the cursors report no current object.

            @param univ       Universe
            @param mapConfig  Map configuration */
        void setUniverse(Universe* univ, const Configuration* mapConfig);

        /** Access ship cursor (F1/ship screen).
            @return cursor */
        ObjectCursor& currentShip();

        /** Access history ship cursor (F6/history screen).
            @return cursor */
        ObjectCursor& currentHistoryShip();

        /** Access planet cursor (F2/planet screen).
            @return cursor */
        ObjectCursor& currentPlanet();

        /** Access starbaes cursor (F3/starbase screen).
            @return cursor */
        ObjectCursor& currentBase();

        /** Access fleet cursor (F10/fleet screen).
            @return cursor */
        ObjectCursor& currentFleet();

        /** Access Ufo cursor.
            @return cursor */
        ObjectCursor& currentUfo();

        /** Access ion storm cursor.
            @return cursor */
        ObjectCursor& currentIonStorm();

        /** Access minefield cursor.
            @return cursor */
        ObjectCursor& currentMinefield();

        /** Access map location.
            @return location */
        Location& location();

        /** Get object cursor, given a type number.
            Implements the mapping required for the "Iterator" script functionality.
            @param nr Number
            @return cursor; null if nr is not known
            @see game::interface::makeIteratorValue */
        ObjectCursor* getCursorByNumber(int nr);

        /** Get object type, given a type number.
            Implements the mapping required for the "Iterator" script functionality.
            @param nr Number
            @return object type; null if nr is not known
            @see game::interface::makeIteratorValue */
        ObjectType* getTypeByNumber(int nr);

        /** Get reference type, given a type number.
            If getCursorByNumber()/getTypeByNumber() produce objects of a given type,
            returns a Reference::Type suitable to form references to those objects.

            @param nr Number
            @return reference type; Reference::Null if nr is not known */
        static Reference::Type getReferenceTypeByNumber(int nr);

        /*
         *  Symbolic names for cursor numbers
         */

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

inline game::map::ObjectCursor&
game::map::Cursors::currentShip()
{
    return m_currentShip;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentHistoryShip()
{
    return m_currentHistoryShip;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentPlanet()
{
    return m_currentPlanet;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentBase()
{
    return m_currentBase;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentFleet()
{
    return m_currentFleet;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentUfo()
{
    return m_currentUfo;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentIonStorm()
{
    return m_currentIonStorm;
}

inline game::map::ObjectCursor&
game::map::Cursors::currentMinefield()
{
    return m_currentMinefield;
}

inline game::map::Location&
game::map::Cursors::location()
{
    return m_location;
}

#endif
