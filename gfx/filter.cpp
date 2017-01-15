/**
  *  \file gfx/filter.cpp
  */

#include "gfx/filter.hpp"

gfx::Filter::Filter(Canvas& parent)
    : Canvas(),
      m_parent(parent)
{ }

gfx::Filter::~Filter()
{ }

void
gfx::Filter::getPixels(Point pt, afl::base::Memory<Color_t> colors)
{
    m_parent.getPixels(pt, colors);
}

gfx::Point
gfx::Filter::getSize()
{
    return m_parent.getSize();
}

int
gfx::Filter::getBitsPerPixel()
{
    return m_parent.getBitsPerPixel();
}

void
gfx::Filter::setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
{
    m_parent.setPalette(start, colorDefinitions, colorHandles);
}

void
gfx::Filter::decodeColors(afl::base::Memory<const Color_t> colorHandles, afl::base::Memory<ColorQuad_t> colorDefinitions)
{
    m_parent.decodeColors(colorHandles, colorDefinitions);
}

void
gfx::Filter::encodeColors(afl::base::Memory<const ColorQuad_t> colorDefinitions, afl::base::Memory<Color_t> colorHandles)
{
    m_parent.encodeColors(colorDefinitions, colorHandles);
}

afl::base::Ref<gfx::Canvas>
gfx::Filter::convertCanvas(afl::base::Ref<Canvas> orig)
{
    return orig;
}

gfx::Canvas&
gfx::Filter::parent()
{
    return m_parent;
}
