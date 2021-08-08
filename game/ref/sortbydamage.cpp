/**
  *  \file game/ref/sortbydamage.cpp
  */

#include "game/ref/sortbydamage.hpp"
#include "game/map/universe.hpp"
#include "game/map/ship.hpp"

game::ref::SortByDamage::SortByDamage(const game::map::Universe& univ)
    : m_universe(univ)
{ }

int
game::ref::SortByDamage::compare(const Reference& a, const Reference& b) const
{
    return getDamage(a) - getDamage(b);
}

String_t
game::ref::SortByDamage::getClass(const Reference& /*a*/) const
{
    return String_t();
}

int
game::ref::SortByDamage::getDamage(const Reference& a) const
{
    // ex sortByDamage, sort.pas:SortByDamage
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const game::map::Ship* pShip = m_universe.ships().get(a.getId())) {
            result = pShip->getDamage().orElse(0);
        }
    }
    return result;
}
