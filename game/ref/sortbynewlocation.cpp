/**
  *  \file game/ref/sortbynewlocation.cpp
  */

#include "game/ref/sortbynewlocation.hpp"
#include "util/math.hpp"
#include "afl/string/format.hpp"

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
    // FIXME: duplicate to SortByLocation::compare
    game::map::Point pa, pb;
    bool oka = getLocation(a, pa);
    bool okb = getLocation(b, pb);
    int result = util::compare3(oka, okb);
    if (result == 0) {
        result = pa.compare(pb);
    }
    return result;
}

String_t
game::ref::SortByNewLocation::getClass(const Reference& a) const
{
    // ex diviNewLocation
    // FIXME: duplicate to SortByLocation::getClass
    game::map::Point pt;
    if (getLocation(a, pt)) {
        return pt.toString();
    } else {
        return m_translator.translateString("not on map");
    }
}

bool
game::ref::SortByNewLocation::getLocation(const Reference& a, game::map::Point& out) const
{
    if (a.getType() == Reference::Ship && m_predictor.getShipPosition(a.getId(), out)) {
        // ok, resolved by predictor
        return true;
    } else {
        // does not move, or we don't know how it moves.
        const game::map::Object* mo = m_universe.getObject(a);
        if (mo != 0) {
            return mo->getPosition().get(out);
        } else {
            return false;
        }
    }
}
