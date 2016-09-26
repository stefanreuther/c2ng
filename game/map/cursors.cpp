/**
  *  \file game/map/cursors.cpp
  */

#include "game/map/cursors.hpp"
#include "game/map/universe.hpp"

game::map::Cursors::Cursors()
    : m_pUniverse(0),
      m_currentShip(),
      m_currentPlanet(),
      m_currentBase(),
      m_currentIonStorm()
{ }

game::map::Cursors::~Cursors()
{ }

void
game::map::Cursors::setUniverse(Universe* univ)
{
    m_pUniverse = univ;
    if (univ != 0) {
        m_currentShip.setObjectType(&univ->playedShips());
        m_currentPlanet.setObjectType(&univ->playedPlanets());
        m_currentBase.setObjectType(&univ->playedBases());
        m_currentIonStorm.setObjectType(&univ->ionStormType());
    } else {
        m_currentShip.setObjectType(0);
        m_currentPlanet.setObjectType(0);
        m_currentBase.setObjectType(0);
        m_currentIonStorm.setObjectType(0);
    }
}

game::map::SimpleObjectCursor&
game::map::Cursors::currentShip()
{
    return m_currentShip;
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
game::map::Cursors::currentIonStorm()
{
    return m_currentIonStorm;
}

game::map::ObjectCursor*
game::map::Cursors::getCursorByNumber(int nr)
{
    // ex int/if/iterif.h:getObjectSelectionFromIteratorId
    switch (nr) {
     case ShipScreen:   return &m_currentShip;   break;
     case PlanetScreen: return &m_currentPlanet; break;
     case BaseScreen:   return &m_currentBase;   break;
     // case 10: return &current_fleet_selection;
     // case 30: return &current_ufo_selection;
     case IonStorms:    return &m_currentIonStorm; break;
     // case 32: return &current_minefield_selection;
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
            // case 10: return &current_fleet_type;
            // case 21: return &current_anyship_type;
            // case 22: return &current_anyplanet_type;
            // case 30: return &current_ufo_type;
         case IonStorms:     return &m_pUniverse->ionStormType();
            // case 32: return &current_minefield_type;
        }
    }
    return 0;
}
