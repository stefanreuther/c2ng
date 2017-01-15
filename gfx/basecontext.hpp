/**
  *  \file gfx/basecontext.hpp
  */
#ifndef C2NG_GFX_BASECONTEXT_HPP
#define C2NG_GFX_BASECONTEXT_HPP

#include "gfx/fillpattern.hpp"
#include "gfx/types.hpp"
#include "gfx/point.hpp"

namespace gfx {

    class Font;
    class Canvas;

    class BaseContext {
     public:
        explicit BaseContext(Canvas& canvas);

        BaseContext& setRawColor(Color_t color);
        BaseContext& setSolidBackground();
        BaseContext& setTransparentBackground();
        BaseContext& setLineThickness(int n);
        BaseContext& setLinePattern(LinePattern_t pat);
        BaseContext& setFillPattern(const FillPattern& pat);
        BaseContext& setAlpha(Alpha_t alpha);
        BaseContext& setCursor(Point pt);
        BaseContext& setTextAlign(int x, int y);

        BaseContext& useFont(Font& font);
        BaseContext& useCanvas(Canvas& canvas);

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

#endif
