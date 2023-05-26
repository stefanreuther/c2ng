/**
  *  \file ui/layout/info.cpp
  *  \brief Class ui::layout::Info
  */

#include "ui/layout/info.hpp"

// General constructor.
ui::layout::Info::Info(gfx::Point prefSize, Growth growth) throw()
    : m_preferredSize(prefSize),
      m_growth(growth)
{ }

// Fixed-size constructor.
ui::layout::Info::Info(gfx::Point fixedSize) throw()
    : m_preferredSize(fixedSize),
      m_growth(Fixed)
{ }

// No-layout/invisible constructor.
ui::layout::Info::Info() throw()
    : m_preferredSize(),
      m_growth(NoLayout)
{ }

// Check for horizontal growth.
bool
ui::layout::Info::isGrowHorizontal(Growth g)
{
    return g == GrowHorizontal || g == GrowBoth;
}

// Check for vertical growth.
bool
ui::layout::Info::isGrowVertical(Growth g)
{
    return g == GrowVertical || g == GrowBoth;
}

// Check for ignored widget.
bool
ui::layout::Info::isIgnored(Growth g)
{
    return g == NoLayout;
}

// Make growth behaviour from parameters.
ui::layout::Info::Growth
ui::layout::Info::makeGrowthBehaviour(bool h, bool v, bool ignore)
{
    return ignore
        ? NoLayout
        : h
        ? (v
           ? GrowBoth
           : GrowHorizontal)
        : (v
           ? GrowVertical
           : Fixed);
}

// Combine two growth behaviours with "And".
ui::layout::Info::Growth
ui::layout::Info::andGrowthBehaviour(Growth a, Growth b)
{
    return isIgnored(a)
        ? b
        : isIgnored(b)
        ? a
        : makeGrowthBehaviour(isGrowHorizontal(a) & isGrowHorizontal(b),
                              isGrowVertical(a)   & isGrowVertical(b),
                              false);
}
