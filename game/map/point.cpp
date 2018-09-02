/**
  *  \file game/point.cpp
  */

#include "game/map/point.hpp"
#include "afl/string/parse.hpp"
#include "util/math.hpp"

game::map::Point::Point()
    : m_x(0), m_y(0)
{ }

game::map::Point::Point(int x, int y)
    : m_x(x), m_y(y)
{ }

int
game::map::Point::getX() const
{
    return m_x;
}

int
game::map::Point::getY() const
{
    return m_y;
}

void
game::map::Point::setX(int x)
{
    m_x = x;
}

void
game::map::Point::setY(int y)
{
    m_y = y;
}

void
game::map::Point::addX(int dx)
{
    m_x += dx;
}

void
game::map::Point::addY(int dy)
{
    m_y += dy;
}

bool
game::map::Point::operator==(Point rhs) const
{
    return m_x == rhs.m_x && m_y == rhs.m_y;
}

bool
game::map::Point::operator!=(Point rhs) const
{
    return m_x != rhs.m_x || m_y != rhs.m_y;
}

game::map::Point
game::map::Point::operator+(Point other) const
{
    return Point(m_x + other.m_x, m_y + other.m_y);
}

game::map::Point
game::map::Point::operator-(Point other) const
{
    return Point(m_x - other.m_x, m_y - other.m_y);
}

game::map::Point&
game::map::Point::operator+=(Point other)
{
    m_x += other.m_x; m_y += other.m_y;
    return *this;
}

game::map::Point&
game::map::Point::operator-=(Point other)
{
    m_x -= other.m_x;
    m_y -= other.m_y;
    return *this;
}

/** Set this point by parsing a coordinate pair.
    \param str Coordinate
    \return true on success */
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

int
game::map::Point::compare(const Point& other) const
{
    int result = util::compare3(m_y, other.m_y);
    if (result == 0) {
        result = util::compare3(m_x, other.m_x);        
    }
    return result;
}

long
game::map::Point::getSquaredRawDistance(Point other) const
{
    // ex game/coord.h:distanceSquaredRaw
    long dx = other.m_x - m_x;
    long dy = other.m_y - m_y;
    return dx*dx + dy*dy;
}

bool
game::map::Point::isCloserThan(Point other, long distance) const
{
    return getSquaredRawDistance(other) < distance*distance;
}
