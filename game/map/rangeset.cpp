/**
  *  \file game/map/rangeset.cpp
  *  \brief Class game::map::RangeSet
  */

#include "game/map/rangeset.hpp"
#include "game/map/objecttype.hpp"
#include "game/map/object.hpp"

game::map::RangeSet::RangeSet()
    : m_points(),
      m_min(),
      m_max()
{ }

game::map::RangeSet::~RangeSet()
{ }

void
game::map::RangeSet::add(Point pt, int r)
{
    // ex GRangeSet::add(GPoint pt, int r)
    if (r > 0) {
        // Seed min, max
        if (m_points.empty()) {
            m_min = m_max = pt;
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
        m_min.setX(std::min(m_min.getX(), pt.getX() - r));
        m_min.setY(std::min(m_min.getY(), pt.getY() - r));
        m_max.setX(std::max(m_max.getX(), pt.getX() + r));
        m_max.setY(std::max(m_max.getY(), pt.getY() + r));
    }
}

void
game::map::RangeSet::addObjectType(ObjectType& type, PlayerSet_t playerLimit, bool markedOnly, int r)
{
    // ex GRangeSet::addObjectType
    for (int i = type.findNextIndex(0); i != 0; i = type.findNextIndex(i)) {
        if (const Object* mo = type.getObjectByIndex(i)) {
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

void
game::map::RangeSet::clear()
{
    // ex GRangeSet::clear
    m_points.clear();
    m_min = m_max = Point(0, 0);
}

bool
game::map::RangeSet::isEmpty() const
{
    // ex GRangeSet::isEmpty
    return m_points.empty();
}

game::map::Point
game::map::RangeSet::getMin() const
{
    // ex GRangeSet::getMinimum
    return m_min;
}

game::map::Point
game::map::RangeSet::getMax() const
{
    // ex GRangeSet::getMaximum
    return m_max;
}

game::map::RangeSet::Iterator_t
game::map::RangeSet::begin() const
{
    // ex GRangeSet::begin
    return m_points.begin();
}

game::map::RangeSet::Iterator_t
game::map::RangeSet::end() const
{
    // ex GRangeSet::end
    return m_points.end();
}
