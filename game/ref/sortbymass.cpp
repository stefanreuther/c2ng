/**
  *  \file game/ref/sortbymass.cpp
  */

#include "game/ref/sortbymass.hpp"

game::ref::SortByMass::SortByMass(const game::map::Universe& univ, const game::spec::ShipList& shipList)
    : m_universe(univ),
      m_shipList(shipList)
{ }

int
game::ref::SortByMass::compare(const Reference& a, const Reference& b) const
{
    // ex sortByMass
    return getMass(a) - getMass(b);
}

String_t
game::ref::SortByMass::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortByMass::getMass(const Reference& a) const
{
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const game::map::Ship* pShip = m_universe.ships().get(a.getId())) {
            result = pShip->getMass(m_shipList).orElse(0);
        }
    }
    return result;
}
