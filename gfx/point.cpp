/**
  *  \file gfx/point.cpp
  */

#include <ostream>
#include "gfx/point.hpp"

std::ostream& operator<<(std::ostream& os, const gfx::Point& pt)
{
    return os << pt.getX() << "," << pt.getY();
}
