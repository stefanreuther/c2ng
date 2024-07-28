/**
  *  \file gfx/point.cpp
  *  \brief Class gfx::Point
  */

#include <algorithm>
#include "gfx/point.hpp"
#include "afl/string/format.hpp"

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
String_t
gfx::makePrintable(const Point& pt)
{
    return afl::string::Format("%d,%d", pt.getX(), pt.getY());
}
