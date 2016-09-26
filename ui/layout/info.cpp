/**
  *  \file ui/layout/info.cpp
  */

#include "ui/layout/info.hpp"

ui::layout::Info::Info(gfx::Point minSize, gfx::Point prefSize, Growth growth)
    : m_minSize(minSize),
      m_preferredSize(prefSize),
      m_growth(growth)
{ }

ui::layout::Info::Info(gfx::Point fixedSize)
    : m_minSize(fixedSize),
      m_preferredSize(fixedSize),
      m_growth(Fixed)
{ }

ui::layout::Info::Info()
    : m_minSize(),
      m_preferredSize(),
      m_growth(NoLayout)
{ }

gfx::Point
ui::layout::Info::getMinSize() const
{
    return m_minSize;
}

gfx::Point
ui::layout::Info::getPreferredSize() const
{
    return m_preferredSize;
}

ui::layout::Info::Growth
ui::layout::Info::getGrowthBehaviour() const
{
    return m_growth;
}

bool
ui::layout::Info::isGrowHorizontal() const
{
    return m_growth == GrowHorizontal || m_growth == GrowBoth;
}

bool
ui::layout::Info::isGrowVertical() const
{
    return m_growth == GrowVertical || m_growth == GrowBoth;
}

bool
ui::layout::Info::isIgnored() const
{
    return m_growth == NoLayout;
}

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
