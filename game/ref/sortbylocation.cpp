/**
  *  \file game/ref/sortbylocation.cpp
  */

#include "game/ref/sortbylocation.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"

game::ref::SortByLocation::SortByLocation(const game::map::Universe& univ, afl::string::Translator& tx)
    : m_universe(univ),
      m_translator(tx)
{ }

int
game::ref::SortByLocation::compare(const Reference& a, const Reference& b) const
{
    // ex sortByLocation
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
game::ref::SortByLocation::getClass(const Reference& a) const
{
    // ex diviLocation
    game::map::Point pt;
    if (getLocation(a, pt)) {
        return pt.toString();
    } else {
        return m_translator.translateString("not on map");
    }
}

bool
game::ref::SortByLocation::getLocation(const Reference& a, game::map::Point& out) const
{
    // FIXME: handle reference-to-position
    const game::map::Object* mo = m_universe.getObject(a);
    if (mo != 0) {
        return mo->getPosition(out);
    } else {
        return false;
    }
}
