/**
  *  \file game/map/viewport.cpp
  */

#include "game/map/viewport.hpp"
#include "game/map/universe.hpp"
#include "util/math.hpp"

game::map::Viewport::Viewport(Universe& univ, TeamSettings& teams)
    : m_universe(univ),
      m_teamSettings(teams),
      m_min(),
      m_max(),
      m_options(Options_t::allUpTo(ShowWarpWells)),
      conn_universeChange(univ.sig_universeChange.add(this, &Viewport::onChange)),
      conn_teamChange(univ.sig_universeChange.add(this, &Viewport::onChange))
{ }

game::map::Viewport::~Viewport()
{ }

game::map::Universe&
game::map::Viewport::universe()
{
    return m_universe;
}

game::TeamSettings&
game::map::Viewport::teamSettings()
{
    return m_teamSettings;
}

void
game::map::Viewport::setRange(Point min, Point max)
{
    if (min != m_min || max != m_max) {
        m_min = min;
        m_max = max;
        onChange();
    }
}

void
game::map::Viewport::setOption(Option opt, bool enable)
{
    if (enable) {
        m_options += opt;
    } else {
        m_options -= opt;
    }
}

game::map::Viewport::Options_t
game::map::Viewport::getOptions() const
{
    return m_options;
}

void
game::map::Viewport::setOptions(Options_t opts)
{
    m_options = opts;
}

game::map::Point
game::map::Viewport::getMin() const
{
    return m_min;
}

game::map::Point
game::map::Viewport::getMax() const
{
    return m_max;
}

bool
game::map::Viewport::hasOption(Option opt) const
{
    return m_options.contains(opt);
}

bool
game::map::Viewport::containsCircle(Point origin, int radius) const
{
    // FIXME: make this method smarter
    return containsRectangle(origin - Point(radius, radius), origin + Point(radius, radius));
}

bool
game::map::Viewport::containsRectangle(Point a, Point b) const
{
    int minX = std::min(a.getX(), b.getX());
    int maxX = std::max(a.getX(), b.getX());
    int minY = std::min(a.getY(), b.getY());
    int maxY = std::max(a.getY(), b.getY());

    return std::max(minX, m_min.getX()) <= std::min(maxX, m_max.getX())
        && std::max(minY, m_min.getY()) <= std::min(maxY, m_max.getY());
}

bool
game::map::Viewport::containsLine(Point a, Point b) const
{
    // FIXME: make this method smarter
    return containsRectangle(a, b);
}

void
game::map::Viewport::onChange()
{
    sig_update.raise();
}
