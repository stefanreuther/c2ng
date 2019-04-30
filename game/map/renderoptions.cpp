/**
  *  \file game/map/renderoptions.cpp
  */

#include "game/map/renderoptions.hpp"

game::map::RenderOptions::RenderOptions()
    : m_show(defaults()),
      m_fill(defaults() - tristate())
{ }

// /** Toggle an option (or set of options).
//     \param which_ones options to toggle, set of co_XXX */
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

    m_show |= hidden_tristate;                             /* show hidden */
    m_fill -= hidden_tristate;

    m_fill |= empty_tristate;                              /* fill empty */

    m_show -= filled_tristate;                            /* hide filled */
    m_fill -= filled_tristate;

    // Then, normal options
    Options_t boolean_opts = which - tristate();
    m_show ^= boolean_opts;

    sig_change.raise();
}

// /** Copy from another option set. */
void
game::map::RenderOptions::copyFrom(const RenderOptions& opts)
{
    // ex copyFrom
    if (m_fill != opts.m_fill || m_show != opts.m_show) {
        m_fill = opts.m_fill;
        m_show = opts.m_show;
        sig_change.raise();
    }
}

// /** Copy values from another option set.
//     \param opts Option set to copy from
//     \param mask Mask describing options to copy */
void
game::map::RenderOptions::copyValues(const RenderOptions& opts, Options_t mask)
{
    // ex GChartOptions::copyValues
    const Options_t newShow =  (opts.m_show & mask) | (m_show - mask);
    const Options_t newFill = ((opts.m_fill & mask) | (m_fill - mask)) & tristate();
    if (m_show != newShow || m_fill != newFill) {
        m_show = newShow;
        m_fill = newFill;
        sig_change.raise();
    }
}

// /** Set option. Set all options in \c mask to "enabled, not filled".
//     This is a state all options allow.
//     \param mask Options to set */
void
game::map::RenderOptions::setOption(Options_t which)
{
    // ex GChartOptions::setOption
    const Options_t newShow = (m_show | which);
    const Options_t newFill = (m_fill - which);
    if (m_show != newShow || m_fill != newFill) {
        m_show = newShow;
        m_fill = newFill;
        sig_change.raise();
    }
}

inline game::map::RenderOptions::Options_t
game::map::RenderOptions::all()
{
    return Options_t::allUpTo(ShowWarpWells);
}

inline game::map::RenderOptions::Options_t
game::map::RenderOptions::tristate()
{
    // ex co_Tristate
    return Options_t() + ShowIonStorms + ShowMinefields + ShowUfos + ShowSectors;
}

inline game::map::RenderOptions::Options_t
game::map::RenderOptions::defaults()
{
    // ex co_Default
    return all() - ShowTrails - ShowWarpWells;
}
