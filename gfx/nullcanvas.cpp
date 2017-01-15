/**
  *  \file gfx/nullcanvas.cpp
  */

#include "gfx/nullcanvas.hpp"

gfx::NullCanvas::~NullCanvas()
{ }

void
gfx::NullCanvas::drawHLine(const Point& /*pt*/, int /*npix*/, Color_t /*color*/, LinePattern_t /*pat*/, Alpha_t /*alpha*/)
{ }

void
gfx::NullCanvas::drawVLine(const Point& /*pt*/, int /*npix*/, Color_t /*color*/, LinePattern_t /*pat*/, Alpha_t /*alpha*/)
{ }

void
gfx::NullCanvas::drawPixel(const Point& /*pt*/, Color_t /*color*/, Alpha_t /*alpha*/)
{ }

void
gfx::NullCanvas::drawPixels(const Point& /*pt*/, afl::base::Memory<const Color_t> /*colors*/, Alpha_t /*alpha*/)
{ }

void
gfx::NullCanvas::drawBar(Rectangle /*rect*/, Color_t /*color*/, Color_t /*bg*/, const FillPattern& /*pat*/, Alpha_t /*alpha*/)
{ }

void
gfx::NullCanvas::blit(const Point& /*pt*/, Canvas& /*src*/, Rectangle /*rect*/)
{ }

void
gfx::NullCanvas::blitPattern(Rectangle /*rect*/, const Point& /*pt*/, int /*bytesPerLine*/, const uint8_t* /*data*/, Color_t /*color*/, Color_t /*bg*/, Alpha_t /*alpha*/)
{ }

gfx::Rectangle
gfx::NullCanvas::computeClipRect(Rectangle r)
{
    // tell them we clip away everything
    return Rectangle(r.getTopLeft(), Point());
}

void
gfx::NullCanvas::getPixels(Point /*pt*/, afl::base::Memory<Color_t> colors)
{
    colors.fill(0);
}

gfx::Point
gfx::NullCanvas::getSize()
{
    return Point(1, 1);
}

int
gfx::NullCanvas::getBitsPerPixel()
{
    return 1;
}

bool
gfx::NullCanvas::isVisible(Rectangle /*r*/)
{
    return false;
}

bool
gfx::NullCanvas::isClipped(Rectangle /*r*/)
{
    return true;
}

void
gfx::NullCanvas::setPalette(Color_t start, afl::base::Memory<const ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<Color_t> colorHandles)
{
    while (Color_t* c = colorHandles.eat()) {
        *c = start++;
    }
}

void
gfx::NullCanvas::decodeColors(afl::base::Memory<const Color_t> /*colorHandles*/, afl::base::Memory<ColorQuad_t> colorDefinitions)
{
    colorDefinitions.fill(COLORQUAD_FROM_RGBA(0,0,0,0));
}

void
gfx::NullCanvas::encodeColors(afl::base::Memory<const ColorQuad_t> /*colorDefinitions*/, afl::base::Memory<Color_t> colorHandles)
{
    colorHandles.fill(0);
}

afl::base::Ref<gfx::Canvas>
gfx::NullCanvas::convertCanvas(afl::base::Ref<Canvas> orig)
{
    return orig;
}
