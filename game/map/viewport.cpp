/**
  *  \file game/map/viewport.cpp
  */

#include "game/map/viewport.hpp"
#include "game/map/universe.hpp"
#include "util/math.hpp"

game::map::Viewport::Viewport(Universe& univ, int turnNumber, TeamSettings& teams,
                              const UnitScoreDefinitionList& shipScoreDefinitions,
                              const game::spec::ShipList& shipList,
                              const game::config::HostConfiguration& config)
    : m_universe(univ),
      m_teamSettings(teams),
      m_turnNumber(turnNumber),
      m_shipScoreDefinitions(shipScoreDefinitions),
      m_shipList(shipList),
      m_hostConfiguration(config),
      m_min(),
      m_max(),
      m_options(Options_t::allUpTo(FillUfos) - ShowShipDots),
      m_drawingTagFilterActive(false),
      m_drawingTagFilter(),
      m_shipTrailId(),
      conn_universeChange(univ.sig_universeChange.add(this, &Viewport::onChange)),
      conn_teamChange(univ.sig_universeChange.add(this, &Viewport::onChange))
{ }

game::map::Viewport::~Viewport()
{ }

game::map::Universe&
game::map::Viewport::universe() const
{
    return m_universe;
}

game::TeamSettings&
game::map::Viewport::teamSettings() const
{
    return m_teamSettings;
}

int
game::map::Viewport::getTurnNumber() const
{
    return m_turnNumber;
}

const game::UnitScoreDefinitionList&
game::map::Viewport::shipScores() const
{
    return m_shipScoreDefinitions;
}

const game::spec::ShipList&
game::map::Viewport::shipList() const
{
    return m_shipList;
}

const game::config::HostConfiguration&
game::map::Viewport::hostConfiguration() const
{
    return m_hostConfiguration;
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
        setOptions(m_options + opt);
    } else {
        setOptions(m_options - opt);
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
    if (opts != m_options) {
        m_options = opts;
        onChange();
    }
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

void
game::map::Viewport::setDrawingTagFilter(util::Atom_t tag)
{
    if (!m_drawingTagFilterActive || m_drawingTagFilter != tag) {
        m_drawingTagFilter = tag;
        m_drawingTagFilterActive = true;
        onChange();
    }
}

void
game::map::Viewport::clearDrawingTagFilter()
{
    if (m_drawingTagFilterActive) {
        m_drawingTagFilter = 0;
        m_drawingTagFilterActive = false;
        onChange();
    }
}

bool
game::map::Viewport::isDrawingTagVisible(util::Atom_t tag) const
{
    return !m_drawingTagFilterActive || tag == m_drawingTagFilter;
}

void
game::map::Viewport::setShipTrailId(Id_t id)
{
    if (m_shipTrailId != id) {
        m_shipTrailId = id;
        onChange();
    }
}

game::Id_t
game::map::Viewport::getShipTrailId() const
{
    return m_shipTrailId;
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

bool
game::map::Viewport::containsText(Point origin, const String_t& text) const
{
    if (text.empty()) {
        return false;
    } else {
        const int ASSUMED_HEIGHT = 20;
        return std::max(origin.getY() - ASSUMED_HEIGHT, m_min.getY())
            <= std::min(origin.getY() + ASSUMED_HEIGHT, m_max.getY());
    }
}

void
game::map::Viewport::onChange()
{
    sig_update.raise();
}
