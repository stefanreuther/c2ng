/**
  *  \file game/ref/sortbyfleet.cpp
  */

#include "game/ref/sortbyfleet.hpp"
#include "game/map/fleet.hpp"

game::ref::SortByFleet::SortByFleet(const game::map::Universe& univ, afl::string::Translator& tx, InterpreterInterface& interface)
    : m_universe(univ),
      m_translator(tx),
      m_interface(interface)
{ }

int
game::ref::SortByFleet::compare(const Reference& a, const Reference& b) const
{
    // ex sortByFleet
    return getFleetNumberKey(a) - getFleetNumberKey(b);
}

String_t
game::ref::SortByFleet::getClass(const Reference& a) const
{
    // ex diviFleet
    int key = getFleetNumberKey(a) >> 1;
    if (key != 0) {
        if (const game::map::Ship* pLeader = m_universe.ships().get(key)) {
            return game::map::Fleet::getTitle(*pLeader, m_translator, m_interface);
        } else {
            // This is an error: a fleet number that does not exist
            return m_translator.translateString("unknown");
        }
    } else {
        return m_translator.translateString("not in a fleet");
    }
}

int
game::ref::SortByFleet::getFleetNumberKey(const Reference& a) const
{
    int result = 0;
    if (a.getType() == Reference::Ship) {
        if (const game::map::Ship* pShip = m_universe.ships().get(a.getId())) {
            int fleetNumber = pShip->getFleetNumber();
            if (fleetNumber != 0) {
                if (pShip->isFleetLeader()) {
                    result = 2*fleetNumber;
                } else {
                    result = 2*fleetNumber + 1;
                }
            }
        }
    }
    return result;
}
