/**
  *  \file game/ref/sortbytowgroup.cpp
  */

#include "game/ref/sortbytowgroup.hpp"
#include "afl/string/format.hpp"
#include "game/spec/mission.hpp"
#include "game/map/playedshiptype.hpp"

namespace {
    game::Id_t getShipTowId(const game::map::Ship& ship)
    {
        // Only check current ships
        // FIXME: should be done generally; loading a history ship should clear the mission
        if (ship.getShipKind() != game::map::Ship::CurrentShip) {
            return 0;
        }

        // Check for Tow mission
        int mission;
        if (!ship.getMission().get(mission)) {
            return 0;
        }
        if (mission != game::spec::Mission::msn_Tow) {
            return 0;
        }

        // Result is tow parameter
        return ship.getMissionParameter(game::TowParameter).orElse(0);
    }
}


game::ref::SortByTowGroup::SortByTowGroup(const game::map::Universe& univ, afl::string::Translator& tx, InterpreterInterface& interface)
    : m_universe(univ),
      m_translator(tx),
      m_interface(interface)
{ }

int
game::ref::SortByTowGroup::compare(const Reference& a, const Reference& b) const
{
    return getTowGroupKey(a) - getTowGroupKey(b);
}

String_t
game::ref::SortByTowGroup::getClass(const Reference& a) const
{
    // ex diviTowGroup
    int key = getTowGroupKey(a) >> 1;
    if (const game::map::Ship* pShip = m_universe.ships().get(key)) {
        return afl::string::Format(m_translator.translateString("towing %s").c_str(), pShip->getName(game::map::Object::PlainName, m_translator, m_interface));
    } else {
        return m_translator.translateString("not in a tow group");
    }

}

int
game::ref::SortByTowGroup::getTowGroupKey(const Reference& a) const
{
    // ex sortByTowGroup
    if (a.getType() == Reference::Ship) {
        if (const game::map::Ship* pShip = m_universe.ships().get(a.getId())) {
            // Check whether we are towing someone.
            if (int towee = getShipTowId(*pShip)) {
                return 2*towee;
            }

            // Check if we are being towed
            for (int i = 1, n = m_universe.ships().size(); i <= n; ++i) {
                if (const game::map::Ship* pTower = m_universe.ships().get(i)) {
                    if (getShipTowId(*pTower) == a.getId()) {
                        return 2*a.getId() + 1;
                    }
                }
            }
        }
    }
    return 0;
}
