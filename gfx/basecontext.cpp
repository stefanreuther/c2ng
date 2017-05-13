/**
  *  \file gfx/basecontext.cpp
  *  \brief Class gfx::BaseContext
  */

#include "gfx/basecontext.hpp"

// Constructor.
gfx::BaseContext::BaseContext(Canvas& canvas)
    : m_rawColor(0),
      m_lineThickness(1),
      m_linePattern(SOLID_LINE),
      m_transparentBackground(true),
      m_fillPattern(FillPattern::SOLID),
      m_alpha(OPAQUE_ALPHA),
      m_cursor(),
      m_textAlign(),
      m_font(0),
      m_canvas(&canvas)
{
    // ex GfxContext::GfxContext
}

// Set color, raw.
gfx::BaseContext&
gfx::BaseContext::setRawColor(Color_t color)
{
    // ex GfxContext::setRawColor
    m_rawColor = color;
    return *this;
}

// Make background solid.
gfx::BaseContext&
gfx::BaseContext::setSolidBackground()
{
    // ex GfxContext::setSolidBackground
    m_transparentBackground = false;
    return *this;
}

// Make background transparent.
gfx::BaseContext&
gfx::BaseContext::setTransparentBackground()
{
    // ex GfxContext::setTransparentBackground
    m_transparentBackground = true;
    return *this;
}

// Set line thickness.
gfx::BaseContext&
gfx::BaseContext::setLineThickness(int n)
{
    // ex GfxContext::setLineThickness
    m_lineThickness = n;
    return *this;
}

// Set line pattern.
gfx::BaseContext&
gfx::BaseContext::setLinePattern(LinePattern_t pat)
{
    // ex GfxContext::setLinePattern
    m_linePattern = pat;
    return *this;
}

// Set fill pattern.
gfx::BaseContext&
gfx::BaseContext::setFillPattern(const FillPattern& pat)
{
    // ex GfxContext::setFillPattern
    // FIXME: remove because we have fillPattern()?
    m_fillPattern = pat;
    return *this;
}

// Set alpha.
gfx::BaseContext&
gfx::BaseContext::setAlpha(Alpha_t alpha)
{
    // ex GfxContext::setAlpha
    m_alpha = alpha;
    return *this;
}

// Set cursor.
gfx::BaseContext&
gfx::BaseContext::setCursor(Point pt)
{
    // ex GfxContext::setCursor
    m_cursor = pt;
    return *this;
}

// Set text alignment.
gfx::BaseContext&
gfx::BaseContext::setTextAlign(int x, int y)
{
    // ex GfxContext::setTextAlign
    m_textAlign = Point(x, y);
    return *this;
}

// Use a font.
gfx::BaseContext&
gfx::BaseContext::useFont(Font& font)
{
    // ex GfxContext::useFont
    m_font = &font;
    return *this;
}

// Use a canvas.
gfx::BaseContext&
gfx::BaseContext::useCanvas(Canvas& canvas)
{
    // ex GfxContext::useCanvas
    m_canvas = &canvas;
    return *this;
}

// Get color, raw.
gfx::Color_t
gfx::BaseContext::getRawColor() const
{
    // ex GfxContext::getRawColor
    return m_rawColor;
}

// Check for transparent background.
bool
gfx::BaseContext::isTransparentBackground() const
{
    // ex GfxContext::isTransparentBackground
    return m_transparentBackground;
}

// Get line thickness.
int
gfx::BaseContext::getLineThickness() const
{
    // ex GfxContext::getLineThickness
    return m_lineThickness;
}

// Get line pattern.
gfx::LinePattern_t
gfx::BaseContext::getLinePattern() const
{
    // ex GfxContext::getLinePattern
    return m_linePattern;
}

// Access fill pattern.
gfx::FillPattern&
gfx::BaseContext::fillPattern()
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

// Access fill pattern.
const gfx::FillPattern&
gfx::BaseContext::fillPattern() const
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

// Get alpha.
gfx::Alpha_t
gfx::BaseContext::getAlpha() const
{
    // ex GfxContext::getAlpha
    return m_alpha;
}

// Get cursor.
gfx::Point
gfx::BaseContext::getCursor() const
{
    // ex GfxContext::getCursor
    return m_cursor;
}

// Get text alignment.
gfx::Point
gfx::BaseContext::getTextAlign() const
{
    // ex GfxContext::getTextAlign
    return m_textAlign;
}

// Get font.
gfx::Font*
gfx::BaseContext::getFont() const
{
    // ex GfxContext::getFont
    return m_font;
}

// Access canvas.
gfx::Canvas&
gfx::BaseContext::canvas() const
{
    // ex GfxContext::getCanvas
    return *m_canvas;
}
