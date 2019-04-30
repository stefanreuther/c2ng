/**
  *  \file gfx/point.cpp
  *  \brief Class gfx::Point
  */

#include <ostream>
#include <algorithm>
#include "gfx/point.hpp"

gfx::Point&
gfx::Point::extendRight(Point other)
{
    m_x += other.m_x;
    m_y = std::max(m_y, other.m_y);
    return *this;
}

gfx::Point&
gfx::Point::extendBelow(Point other)
{
    m_x = std::max(m_x, other.m_x);
    m_y += other.m_y;
    return *this;
}

std::ostream& operator<<(std::ostream& os, const gfx::Point& pt)
{
    return os << pt.getX() << "," << pt.getY();
}
