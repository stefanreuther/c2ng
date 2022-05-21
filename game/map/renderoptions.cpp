/**
  *  \file game/map/renderoptions.cpp
  *  \brief Class game::map::RenderOptions
  */

#include "game/map/renderoptions.hpp"

using game::config::UserConfiguration;

game::map::RenderOptions::RenderOptions()
    : m_show(defaults()),
      m_fill(defaults() & tristate())
{ }

game::map::RenderOptions::RenderOptions(Options_t show, Options_t fill)
    : m_show(show),
      m_fill(fill)
{ }

void
game::map::RenderOptions::toggleOptions(Options_t which)
{
    // ex GChartOptions::toggleOptions

    // First, tristate options. We have three states:
    //    show   fill
    //      0      ?
    //      1      0
    //      1      1
    Options_t tristate_opts   = which & tristate();
    Options_t shown_tristate  = tristate_opts & m_show;        /* shown filled or shown empty */
    Options_t hidden_tristate = tristate_opts - m_show;        /* hidden */
    Options_t empty_tristate  = shown_tristate - m_fill;       /* shown filled */
    Options_t filled_tristate = shown_tristate & m_fill;       /* shown empty */

    m_show |= hidden_tristate;                                 /* show hidden */
    m_fill -= hidden_tristate;

    m_fill |= empty_tristate;                                  /* fill empty */

    m_show -= filled_tristate;                                 /* hide filled */
    m_fill -= filled_tristate;

    // Then, normal options
    Options_t boolean_opts = which - tristate();
    m_show ^= boolean_opts;
}

void
game::map::RenderOptions::copyOptions(const RenderOptions& opts, Options_t which)
{
    // ex GChartOptions::copyValues
    m_show =  (opts.m_show & which) | (m_show - which);
    m_fill = ((opts.m_fill & which) | (m_fill - which)) & tristate();
}

void
game::map::RenderOptions::setOptions(Options_t which)
{
    // ex GChartOptions::setOption
    m_show = (m_show | which);
    m_fill = (m_fill - which);
}

game::map::RenderOptions::Value
game::map::RenderOptions::getOption(Option which) const
{
    if (m_show.contains(which)) {
        if ((m_fill & tristate()).contains(which)) {
            return Filled;
        } else {
            return Enabled;
        }
    } else {
        return Disabled;
    }
}

game::map::Viewport::Options_t
game::map::RenderOptions::getViewportOptions() const
{
    Viewport::Options_t result;

    // Show
    if (m_show.contains(ShowIonStorms)) {
        result += Viewport::ShowIonStorms;
    }
    if (m_show.contains(ShowMinefields)) {
        result += Viewport::ShowMinefields;
    }
    if (m_show.contains(ShowUfos)) {
        result += Viewport::ShowUfos;
    }
    if (m_show.contains(ShowGrid)) {
        result += Viewport::ShowGrid;
    }
    if (m_show.contains(ShowBorders)) {
        result += Viewport::ShowBorders;
    }
    if (m_show.contains(ShowDrawings)) {
        result += Viewport::ShowDrawings;
    }
    if (m_show.contains(ShowSelection)) {
        result += Viewport::ShowSelection;
    }
    if (m_show.contains(ShowLabels)) {
        result += Viewport::ShowLabels;
    }
    if (m_show.contains(ShowTrails)) {
        result += Viewport::ShowTrails;
    }
    if (m_show.contains(ShowShipDots)) {
        result += Viewport::ShowShipDots;
    }
    if (m_show.contains(ShowWarpWells)) {
        result += Viewport::ShowWarpWells;
    }
    if (m_show.contains(ShowMessages)) {
        result += Viewport::ShowMessages;
    }
    if (m_show.contains(ShowMineDecay)) {
        result += Viewport::ShowMineDecay;
    }

    // Fill
    if (m_show.contains(ShowGrid) && !m_fill.contains(ShowGrid)) {
        result += Viewport::ShowOutsideGrid;
    }
    if (m_fill.contains(ShowIonStorms)) {
        result += Viewport::FillIonStorms;
    }
    if (m_fill.contains(ShowMinefields)) {
        result += Viewport::FillMinefields;
    }
    if (m_fill.contains(ShowUfos)) {
        result += Viewport::FillUfos;
    }

    return result;
}

void
game::map::RenderOptions::storeToConfiguration(game::config::UserConfiguration& config, Area area) const
{
    config[UserConfiguration::ChartRenderOptions[area][0]].set(m_show.toInteger());
    config[UserConfiguration::ChartRenderOptions[area][1]].set(m_fill.toInteger());
}

game::map::RenderOptions
game::map::RenderOptions::fromConfiguration(const game::config::UserConfiguration& config, Area area)
{
    return RenderOptions(Options_t::fromInteger(config[UserConfiguration::ChartRenderOptions[area][0]]()),
                         Options_t::fromInteger(config[UserConfiguration::ChartRenderOptions[area][1]]()));
}

game::map::RenderOptions::Options_t
game::map::RenderOptions::getOptionFromKey(util::Key_t key)
{
    // ex GChartOptions::toggleOptionKey
    switch (key) {
     case 'm':
        return Options_t(ShowMinefields);
     case 'a':
        return Options_t(ShowShipDots);
     case 'd':
        return Options_t(ShowLabels);
     case 'i':
        return Options_t(ShowIonStorms);
     case 'v':
        return Options_t(ShowTrails);
     case 't':
        return Options_t(ShowSelection);
     case 's':
        return Options_t(ShowGrid);
     case 'b':
        return Options_t(ShowBorders);
     case 'u':
        return Options_t(ShowUfos);
     case 'p':
        return Options_t(ShowDrawings);
     case 'w':
        return Options_t(ShowWarpWells);
     case 'n':
        return Options_t(ShowMessages);
     case 'y':
        return Options_t(ShowMineDecay);
     default:
        return Options_t();
    }
}
