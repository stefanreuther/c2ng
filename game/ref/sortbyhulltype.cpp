/**
  *  \file game/ref/sortbyhulltype.cpp
  */

#include "game/ref/sortbyhulltype.hpp"
#include "game/map/ship.hpp"
#include "game/spec/hull.hpp"
#include "game/ref/sortbyhullmass.hpp"

game::ref::SortByHullType::SortByHullType(const game::map::Universe& univ, const game::spec::ShipList& shipList, afl::string::Translator& tx)
    : m_universe(univ),
      m_shipList(shipList),
      m_translator(tx)
{ }

int
game::ref::SortByHullType::compare(const Reference& a, const Reference& b) const
{
    // ex sortByHull
    return getHullType(a) - getHullType(b);
}

String_t
game::ref::SortByHullType::getClass(const Reference& a) const
{
    // ex diviHull
    if (const game::spec::Hull* pHull = m_shipList.hulls().get(getHullType(a))) {
        return pHull->getName(m_shipList.componentNamer());
    } else {
        return m_translator.translateString("unknown");
    }
}

int
game::ref::SortByHullType::getHullType(const Reference& a) const
{
    return SortByHullMass(m_universe, m_shipList).getHullType(a);
}
