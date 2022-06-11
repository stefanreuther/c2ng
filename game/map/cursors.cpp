/**
  *  \file game/map/cursors.cpp
  */

#include "game/map/cursors.hpp"
#include "game/map/universe.hpp"

game::map::Cursors::Cursors()
    : m_pUniverse(0),
      m_currentShip(),
      m_currentHistoryShip(),
      m_currentPlanet(),
      m_currentBase(),
      m_currentFleet(),
      m_currentUfo(),
      m_currentIonStorm(),
      m_currentMinefield(),
      m_location()
{ }

game::map::Cursors::~Cursors()
{ }

void
game::map::Cursors::setUniverse(Universe* univ, const Configuration* mapConfig)
{
    m_pUniverse = univ;
    if (univ != 0) {
        m_currentShip.setObjectType(&univ->playedShips());
        m_currentHistoryShip.setObjectType(&univ->historyShips());
        m_currentPlanet.setObjectType(&univ->playedPlanets());
        m_currentBase.setObjectType(&univ->playedBases());
        m_currentFleet.setObjectType(&univ->fleets());
        m_currentUfo.setObjectType(&univ->ufos());
        m_currentIonStorm.setObjectType(&univ->ionStormType());
        m_currentMinefield.setObjectType(&univ->minefields());
    } else {
        m_currentShip.setObjectType(0);
        m_currentHistoryShip.setObjectType(0);
        m_currentPlanet.setObjectType(0);
        m_currentBase.setObjectType(0);
        m_currentFleet.setObjectType(0);
        m_currentUfo.setObjectType(0);
        m_currentIonStorm.setObjectType(0);
        m_currentMinefield.setObjectType(0);
    }
    m_location.setUniverse(univ, mapConfig);
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentShip()
{
    return m_currentShip;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentHistoryShip()
{
    return m_currentHistoryShip;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentPlanet()
{
    return m_currentPlanet;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentBase()
{
    return m_currentBase;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentFleet()
{
    return m_currentFleet;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentUfo()
{
    return m_currentUfo;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentIonStorm()
{
    return m_currentIonStorm;
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentMinefield()
{
    return m_currentMinefield;
}

game::map::Location&
game::map::Cursors::location()
{
    return m_location;
}

game::map::ObjectCursor*
game::map::Cursors::getCursorByNumber(int nr)
{
    // ex int/if/iterif.h:getObjectSelectionFromIteratorId
    switch (nr) {
     case ShipScreen:    return &m_currentShip;
     case PlanetScreen:  return &m_currentPlanet;
     case BaseScreen:    return &m_currentBase;
     case HistoryScreen: return &m_currentHistoryShip;
     case FleetScreen:   return &m_currentFleet;
     case Ufos:          return &m_currentUfo;
     case IonStorms:     return &m_currentIonStorm;
     case Minefields:    return &m_currentMinefield;
    }
    return 0;
}

game::map::ObjectType*
game::map::Cursors::getTypeByNumber(int nr)
{
    // ex int/if/iterif.h:getObjectTypeFromIteratorId
    // FIXME: here?
    if (m_pUniverse != 0) {
        switch (nr) {
         case ShipScreen:    return &m_pUniverse->playedShips();
         case PlanetScreen:  return &m_pUniverse->playedPlanets();
         case BaseScreen:    return &m_pUniverse->playedBases();
         case HistoryScreen: return &m_pUniverse->historyShips();
         case FleetScreen:   return &m_pUniverse->fleets();
            // case 21: return &current_anyship_type;
            // case 22: return &current_anyplanet_type;
         case Ufos:          return &m_pUniverse->ufos();
         case IonStorms:     return &m_pUniverse->ionStormType();
         case Minefields:    return &m_pUniverse->minefields();
        }
    }
    return 0;
}
