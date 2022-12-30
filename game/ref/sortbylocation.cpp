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
    // ex sortByLocation, sort.pas:SortByLocation
    return comparePositions(getLocation(a), getLocation(b));
}

String_t
game::ref::SortByLocation::getClass(const Reference& a) const
{
    // ex diviLocation
    return getClassFor(getLocation(a));
}

String_t
game::ref::SortByLocation::getClassFor(afl::base::Optional<game::map::Point> pt) const
{
    if (const game::map::Point* p = pt.get()) {
        return p->toString();
    } else {
        return m_translator("not on map");
    }
}

int
game::ref::SortByLocation::comparePositions(afl::base::Optional<game::map::Point> a, afl::base::Optional<game::map::Point> b) const
{
    game::map::Point pa, pb;
    bool oka = a.get(pa);
    bool okb = b.get(pb);
    int result = util::compare3(oka, okb);
    if (result == 0) {
        result = pa.compare(pb);
    }
    return result;
}

afl::base::Optional<game::map::Point>
game::ref::SortByLocation::getLocation(const Reference& a) const
{
    const game::map::Object* mo = m_universe.getObject(a);
    if (mo != 0) {
        // It's a map object
        return mo->getPosition();
    } else {
        // Might be a position
        return a.getPosition();
    }
}
