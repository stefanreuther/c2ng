/**
  *  \file game/ref/sortbyhullmass.cpp
  */

#include "game/ref/sortbyhullmass.hpp"
#include "game/spec/hull.hpp"

game::ref::SortByHullMass::SortByHullMass(const game::map::Universe& univ, const game::spec::ShipList& shipList)
    : m_universe(univ), m_shipList(shipList)
{ }

int
game::ref::SortByHullMass::compare(const Reference& a, const Reference& b) const
{
    // ex sortByHullMass
    return getHullMass(a) - getHullMass(b);
}

String_t
game::ref::SortByHullMass::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortByHullMass::getHullMass(const Reference& a) const
{
    if (const game::spec::Hull* pHull = m_shipList.hulls().get(getHullType(a))) {
        return pHull->getMass();
    } else {
        return 0;
    }
}

int
game::ref::SortByHullMass::getHullType(const Reference& a) const
{
    if (a.getType() == Reference::Hull) {
        return a.getId();
    } else if (a.getType() == Reference::Ship) {
        if (const game::map::Ship* pShip = m_universe.ships().get(a.getId())) {
            return pShip->getHull().orElse(0);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}
