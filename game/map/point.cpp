/**
  *  \file game/point.cpp
  *  \brief Class game::map::Point
  */

#include "game/map/point.hpp"
#include "afl/string/format.hpp"
#include "afl/string/parse.hpp"
#include "util/math.hpp"

// Get component.
int
game::map::Point::get(Component c) const
{
    switch (c) {
     case X: return m_x;
     case Y: return m_y;
    }
    return 0;
}

// Set component.
void
game::map::Point::set(Component c, int v)
{
    switch (c) {
     case X: m_x = v; break;
     case Y: m_y = v; break;
    }
}

// Parse coordinates.
bool
game::map::Point::parseCoordinates(const String_t& str)
{
    // ex GPoint::parseCoordinates
    int x, y;
    String_t::size_type p = str.find(',');
    if (p == str.npos) {
        return false;
    }

    if (!afl::string::strToInteger(str.substr(0, p), x) || !afl::string::strToInteger(str.substr(p+1), y)) {
        return false;
    }

    m_x = x;
    m_y = y;
    return true;
}

String_t
game::map::Point::toString() const
{
    return afl::string::Format("(%d,%d)", m_x, m_y);
}

// Three-way comparison.
int
game::map::Point::compare(const Point& other) const
{
    int result = util::compare3(m_y, other.m_y);
    if (result == 0) {
        result = util::compare3(m_x, other.m_x);
    }
    return result;
}

// Get squared distance to another point.
long
game::map::Point::getSquaredRawDistance(Point other) const
{
    // ex game/coord.h:distanceSquaredRaw
    long dx = other.m_x - m_x;
    long dy = other.m_y - m_y;
    return dx*dx + dy*dy;
}

// Check distance.
bool
game::map::Point::isCloserThan(Point other, long distance) const
{
    return getSquaredRawDistance(other) < distance*distance;
}
