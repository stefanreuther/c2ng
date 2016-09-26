/**
  *  \file gfx/context.hpp
  */
#ifndef C2NG_GFX_CONTEXT_HPP
#define C2NG_GFX_CONTEXT_HPP

#include "gfx/types.hpp"
#include "gfx/fillpattern.hpp"
#include "gfx/point.hpp"

namespace gfx {

    class FillPattern;
    class Font;
    class Canvas;
    class ColorScheme;

    class Context {
     public:
        explicit Context(Canvas& canvas);

        Context& setColor(uint32_t color);
        Context& setRawColor(Color_t color);
        Context& setSolidBackground();
        Context& setTransparentBackground();
        Context& setLineThickness(int n);
        Context& setLinePattern(LinePattern_t pat);
        Context& setFillPattern(const FillPattern& pat);
        Context& setAlpha(Alpha_t alpha);
        Context& setCursor(Point pt);
        Context& setTextAlign(int x, int y);

        Context& useFont(Font& font);
        Context& useCanvas(Canvas& canvas);
        Context& useColorScheme(ColorScheme& colorScheme);

        Color_t getRawColor() const;
        bool isTransparentBackground() const;
        int getLineThickness() const;
        LinePattern_t getLinePattern() const;
        FillPattern& fillPattern();
        const FillPattern& fillPattern() const;
        Alpha_t getAlpha() const;
        Point getCursor() const;
        Point getTextAlign() const;
        Font* getFont() const;
        Canvas& canvas() const;
        ColorScheme& colorScheme() const;

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
        ColorScheme* m_colorScheme;
    };

}

#endif
