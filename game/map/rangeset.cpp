/**
  *  \file game/map/rangeset.cpp
  */

#include "game/map/rangeset.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/mapobject.hpp"

game::map::RangeSet::RangeSet()
    : m_points(),
      m_minimum(),
      m_maximum()
{ }

game::map::RangeSet::~RangeSet()
{ }

// /** Add a single range.
//     \param pt Center
//     \param r Radius */
void
game::map::RangeSet::add(Point pt, int r)
{
    // ex GRangeSet::add(GPoint pt, int r)
    if (r > 0) {
        // Seed min, max
        if (m_points.empty()) {
            m_minimum = m_maximum = pt;
        }

        // Include point in map
        PointMap_t::iterator it = m_points.find(pt);
        if (it != m_points.end()) {
            if (r > it->second) {
                it->second = r;
            }
        } else {
            m_points.insert(std::make_pair(pt, r));
        }

        // Update min, max
        m_minimum.setX(std::min(m_minimum.getX(), pt.getX() - r));
        m_minimum.setY(std::min(m_minimum.getY(), pt.getY() - r));
        m_maximum.setX(std::max(m_maximum.getX(), pt.getX() + r));
        m_maximum.setY(std::max(m_maximum.getY(), pt.getY() + r));
    }
}

// /** Add object type.
//     \param type Type to iterate through
//     \param playerLimit Only include these players
//     \param markedOnly true to only include marked objects
//     \param r Radius */
void
game::map::RangeSet::addObjectType(ObjectType& type, PlayerSet_t playerLimit, bool markedOnly, int r)
{
    // ex GRangeSet::addObjectType
    for (int i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
        if (MapObject* mo = dynamic_cast<MapObject*>(type.getObjectByIndex(i))) {
            int owner;
            if (mo->getOwner(owner) && playerLimit.contains(owner) && (!markedOnly || mo->isMarked())) {
                Point pt;
                if (mo->getPosition(pt)) {
                    add(pt, r);
                }
            }
        }
    }
}

// /** Clear. */
void
game::map::RangeSet::clear()
{
    // ex GRangeSet::clear
    m_points.clear();
    m_minimum = m_maximum = Point(0, 0);
}

// /** Check for emptiness. */
bool
game::map::RangeSet::isEmpty() const
{
    // ex GRangeSet::isEmpty
    return m_points.empty();
}

// /** Get minimum point of bounding box. */
game::map::Point
game::map::RangeSet::getMinimum() const
{
    // ex GRangeSet::getMinimum
    return m_minimum;
}

// /** Get maximum point of bounding box. */
game::map::Point
game::map::RangeSet::getMaximum() const
{
    // ex GRangeSet::getMaximum
    return m_maximum;
}

// /** Get begin iterator. */
game::map::RangeSet::Iterator_t
game::map::RangeSet::begin() const
{
    // ex GRangeSet::begin
    return m_points.begin();
}

// /** Get end iterator. */
game::map::RangeSet::Iterator_t
game::map::RangeSet::end() const
{
    // ex GRangeSet::end
    return m_points.end();
}
