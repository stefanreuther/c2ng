/**
  *  \file gfx/basecontext.hpp
  *  \brief Class gfx::BaseContext
  */
#ifndef C2NG_GFX_BASECONTEXT_HPP
#define C2NG_GFX_BASECONTEXT_HPP

#include "gfx/fillpattern.hpp"
#include "gfx/types.hpp"
#include "gfx/point.hpp"

namespace gfx {

    class Font;
    class Canvas;

    /** Graphics context (state), base version.
        A graphics context stores state for complex graphics operations to reduce the number of per-call parameters.
        BaseContext is the basic version independant of the used color scheme. */
    class BaseContext {
     public:
        /** Constructor.
            \param canvas Canvas to use */
        explicit BaseContext(Canvas& canvas);

        /** Set color, raw.
            Drawing operations will use this color.
            \param color Color
            \return *this */
        BaseContext& setRawColor(Color_t color);

        /** Make background solid.
            With solid background enabled, text drawing operations will use ColorScheme::drawBackground do fill the area covered by the text.
            \return *this */
        BaseContext& setSolidBackground();

        /** Make background transparent.
            This is the default.
            \return *this */
        BaseContext& setTransparentBackground();

        /** Set line thickness.
            Line drawing operations will make lines this many pixels wide.
            \param n thickness (default: 1)
            \return *this */
        BaseContext& setLineThickness(int n);

        /** Set line pattern.
            Line drawing operations will use this pattern.
            \param pat pattern (default: solid/0xFF)
            \return *this */
        BaseContext& setLinePattern(LinePattern_t pat);

        /** Set fill pattern.
            Solid-shape drawing operations will use this fill pattern.
            \param pat pattern (default: solid)
            \return *this */
        BaseContext& setFillPattern(const FillPattern& pat);

        /** Set alpha.
            All drawing operations will use this alpha.
            \param alpha alpha
            \return *this */
        BaseContext& setAlpha(Alpha_t alpha);

        /** Set cursor.
            Some drawing operations that don't take a position use the cursor.
            Some drawing operations return a position as cursor.
            \param pt cursor
            \return *this */
        BaseContext& setCursor(Point pt);

        /** Set text alignment.
            Text drawing operations will use this to align the given position and the text.
            \param x horizontal alignment
            \param y vertical alignment
            \return *this */
        BaseContext& setTextAlign(HorizontalAlignment x, VerticalAlignment y);

        /** Use a font.
            \param font Font
            \return *this */
        BaseContext& useFont(Font& font);

        /** Use a canvas.
            \param canvas Canvas
            \return *this */
        BaseContext& useCanvas(Canvas& canvas);

        /** Get color, raw.
            \return color */
        Color_t getRawColor() const;

        /** Check for transparent background.
            \retval true transparent background
            \retval false solid background  */
        bool isTransparentBackground() const;

        /** Get line thickness.
            \return thickness */
        int getLineThickness() const;

        /** Get line pattern.
            \return pattern */
        LinePattern_t getLinePattern() const;

        /** Access fill pattern.
            You can modify the pattern in place.
            \return pattern */
        FillPattern& fillPattern();

        /** Access fill pattern.
            \return pattern */
        const FillPattern& fillPattern() const;

        /** Get alpha.
            \return alpha */
        Alpha_t getAlpha() const;

        /** Get cursor.
            \return cursor */
        Point getCursor() const;

        /** Get text alignment.
            \return x and y alignment as a Point */
        Point getTextAlign() const;

        /** Get font.
            \return font */
        Font* getFont() const;

        /** Access canvas.
            \return canvas */
        Canvas& canvas() const;

     private:
        Color_t m_rawColor;
        int m_lineThickness;
        LinePattern_t m_linePattern;
        bool m_transparentBackground;
        FillPattern m_fillPattern;
        Alpha_t m_alpha;
        Point m_cursor;
        Point m_textAlign;
        Font* m_font;
        Canvas* m_canvas;
    };

}

// Set color, raw.
inline gfx::BaseContext&
gfx::BaseContext::setRawColor(Color_t color)
{
    // ex GfxContext::setRawColor
    m_rawColor = color;
    return *this;
}

// Make background solid.
inline gfx::BaseContext&
gfx::BaseContext::setSolidBackground()
{
    // ex GfxContext::setSolidBackground
    m_transparentBackground = false;
    return *this;
}

// Make background transparent.
inline gfx::BaseContext&
gfx::BaseContext::setTransparentBackground()
{
    // ex GfxContext::setTransparentBackground
    m_transparentBackground = true;
    return *this;
}

// Set line thickness.
inline gfx::BaseContext&
gfx::BaseContext::setLineThickness(int n)
{
    // ex GfxContext::setLineThickness
    m_lineThickness = n;
    return *this;
}

// Set line pattern.
inline gfx::BaseContext&
gfx::BaseContext::setLinePattern(LinePattern_t pat)
{
    // ex GfxContext::setLinePattern
    m_linePattern = pat;
    return *this;
}

// Set fill pattern.
inline gfx::BaseContext&
gfx::BaseContext::setFillPattern(const FillPattern& pat)
{
    // ex GfxContext::setFillPattern
    // FIXME: remove because we have fillPattern()?
    m_fillPattern = pat;
    return *this;
}

// Set alpha.
inline gfx::BaseContext&
gfx::BaseContext::setAlpha(Alpha_t alpha)
{
    // ex GfxContext::setAlpha
    m_alpha = alpha;
    return *this;
}

// Set cursor.
inline gfx::BaseContext&
gfx::BaseContext::setCursor(Point pt)
{
    // ex GfxContext::setCursor
    m_cursor = pt;
    return *this;
}

// Set text alignment.
inline gfx::BaseContext&
gfx::BaseContext::setTextAlign(HorizontalAlignment x, VerticalAlignment y)
{
    // ex GfxContext::setTextAlign
    m_textAlign = Point(x, y);
    return *this;
}

// Use a font.
inline gfx::BaseContext&
gfx::BaseContext::useFont(Font& font)
{
    // ex GfxContext::useFont
    m_font = &font;
    return *this;
}

// Use a canvas.
inline gfx::BaseContext&
gfx::BaseContext::useCanvas(Canvas& canvas)
{
    // ex GfxContext::useCanvas
    m_canvas = &canvas;
    return *this;
}

// Get color, raw.
inline gfx::Color_t
gfx::BaseContext::getRawColor() const
{
    // ex GfxContext::getRawColor
    return m_rawColor;
}

// Check for transparent background.
inline bool
gfx::BaseContext::isTransparentBackground() const
{
    // ex GfxContext::isTransparentBackground
    return m_transparentBackground;
}

// Get line thickness.
inline int
gfx::BaseContext::getLineThickness() const
{
    // ex GfxContext::getLineThickness
    return m_lineThickness;
}

// Get line pattern.
inline gfx::LinePattern_t
gfx::BaseContext::getLinePattern() const
{
    // ex GfxContext::getLinePattern
    return m_linePattern;
}

// Access fill pattern.
inline gfx::FillPattern&
gfx::BaseContext::fillPattern()
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

// Access fill pattern.
inline const gfx::FillPattern&
gfx::BaseContext::fillPattern() const
{
    // ex GfxContext::getFillPattern
    return m_fillPattern;
}

// Get alpha.
inline gfx::Alpha_t
gfx::BaseContext::getAlpha() const
{
    // ex GfxContext::getAlpha
    return m_alpha;
}

// Get cursor.
inline gfx::Point
gfx::BaseContext::getCursor() const
{
    // ex GfxContext::getCursor
    return m_cursor;
}

// Get text alignment.
inline gfx::Point
gfx::BaseContext::getTextAlign() const
{
    // ex GfxContext::getTextAlign
    return m_textAlign;
}

// Get font.
inline gfx::Font*
gfx::BaseContext::getFont() const
{
    // ex GfxContext::getFont
    return m_font;
}

// Access canvas.
inline gfx::Canvas&
gfx::BaseContext::canvas() const
{
    // ex GfxContext::getCanvas
    return *m_canvas;
}

#endif
