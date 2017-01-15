/**
  *  \file gfx/basecontext.cpp
  */

#include "gfx/basecontext.hpp"

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

// gfx::Context&
// gfx::Context::setColor(uint32_t color)
// {
//     // ex GfxContext::setColor
//     m_rawColor = colorScheme().getColor(color);
//     return *this;
// }

gfx::BaseContext&
gfx::BaseContext::setRawColor(Color_t color)
{
    // ex GfxContext::setRawColor
    m_rawColor = color;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setSolidBackground()
{
    // ex GfxContext::setSolidBackground
    m_transparentBackground = false;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setTransparentBackground()
{
    // ex GfxContext::setTransparentBackground
    m_transparentBackground = true;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setLineThickness(int n)
{
    // ex GfxContext::setLineThickness
    m_lineThickness = n;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setLinePattern(LinePattern_t pat)
{
    // ex GfxContext::setLinePattern
    m_linePattern = pat;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setFillPattern(const FillPattern& pat)
{
    // ex GfxContext::setFillPattern
    // FIXME: remove because we have fillPattern()?
    m_fillPattern = pat;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setAlpha(Alpha_t alpha)
{
    // ex GfxContext::setAlpha
    m_alpha = alpha;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setCursor(Point pt)
{
    // ex GfxContext::setCursor
    m_cursor = pt;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::setTextAlign(int x, int y)
{
    // ex GfxContext::setTextAlign
    m_textAlign = Point(x, y);
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::useFont(Font& font)
{
    // ex GfxContext::useFont
    m_font = &font;
    return *this;
}

gfx::BaseContext&
gfx::BaseContext::useCanvas(Canvas& canvas)
{
    // ex GfxContext::useCanvas
    m_canvas = &canvas;
    return *this;
}

// gfx::Context&
// gfx::Context::useColorScheme(ColorScheme<uint32_t>& colorScheme)
// {
//     // ex GfxContext::useColorScheme
//     m_colorScheme = &colorScheme;
//     return *this;
// }

gfx::Color_t
gfx::BaseContext::getRawColor() const
{
    // ex GfxContext::getRawColor
    return m_rawColor;
}

bool
gfx::BaseContext::isTransparentBackground() const
{
    // ex GfxContext::isTransparentBackground
    return m_transparentBackground;
}

int
gfx::BaseContext::getLineThickness() const
{
    // ex GfxContext::getLineThickness
    return m_lineThickness;
}

gfx::LinePattern_t
gfx::BaseContext::getLinePattern() const
{
    // ex GfxContext::getLinePattern
    return m_linePattern;
}

gfx::FillPattern&
gfx::BaseContext::fillPattern()
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

const gfx::FillPattern&
gfx::BaseContext::fillPattern() const
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

gfx::Alpha_t
gfx::BaseContext::getAlpha() const
{
    // ex GfxContext::getAlpha
    return m_alpha;
}

gfx::Point
gfx::BaseContext::getCursor() const
{
    // ex GfxContext::getCursor
    return m_cursor;
}

gfx::Point
gfx::BaseContext::getTextAlign() const
{
    // ex GfxContext::getTextAlign
    return m_textAlign;
}

gfx::Font*
gfx::BaseContext::getFont() const
{
    // ex GfxContext::getFont
    return m_font;
}

gfx::Canvas&
gfx::BaseContext::canvas() const
{
    // ex GfxContext::getCanvas
    return *m_canvas;
}

// gfx::ColorScheme<uint32_t>&
// gfx::Context::colorScheme() const
// {
//     // ex GfxContext::getColorScheme
//     return *m_colorScheme;
// }
