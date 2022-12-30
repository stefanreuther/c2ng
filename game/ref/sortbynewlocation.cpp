/**
  *  \file game/ref/sortbynewlocation.cpp
  */

#include "game/ref/sortbynewlocation.hpp"
#include "util/math.hpp"
#include "afl/string/format.hpp"
#include "game/ref/sortbylocation.hpp"

game::ref::SortByNewLocation::SortByNewLocation(const game::map::Universe& univ,
                                                const Game& game,
                                                const game::spec::ShipList& shipList,
                                                const Root& root,
                                                afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx),
      m_predictor()
{
    m_predictor.computeMovement(univ, game, shipList, root);
}

int
game::ref::SortByNewLocation::compare(const Reference& a, const Reference& b) const
{
    // ex sortByNewLocation
    return SortByLocation(m_universe, m_translator).comparePositions(getLocation(a), getLocation(b));
}

String_t
game::ref::SortByNewLocation::getClass(const Reference& a) const
{
    // ex diviNewLocation
    return SortByLocation(m_universe, m_translator).getClassFor(getLocation(a));
}

afl::base::Optional<game::map::Point>
game::ref::SortByNewLocation::getLocation(const Reference& a) const
{
    afl::base::Optional<game::map::Point> result;
    if (a.getType() == Reference::Ship) {
        // Try to resolve via predictor
        result = m_predictor.getShipPosition(a.getId());
    }

    if (!result.isValid()) {
        // Does not move, or we don't know how it moves
        if (const game::map::Object* mo = m_universe.getObject(a)) {
            result = mo->getPosition();
        }
    }

    return result;
}
