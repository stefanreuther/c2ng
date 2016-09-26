/**
  *  \file gfx/font.hpp
  */
#ifndef C2NG_GFX_FONT_HPP
#define C2NG_GFX_FONT_HPP

#include "afl/base/refcounted.hpp"
#include "gfx/point.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"

namespace gfx {

    class Context;
    class Rectangle;

    class Font : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Output text using the specified parameters.
            \param ctx    graphics context. outText() should use canvas, color, and alpha;
            and it should \em not use just_x/just_y.
            \param x,y    coordinates (top-left pixel)
            \param text   text */
        virtual void outText(Context& ctx, Point pt, String_t text) = 0;
        virtual int getTextWidth(String_t text) = 0;
        virtual int getTextHeight(String_t text) = 0;

        // Derived methods
        int getEmWidth();
        int getLineHeight();
        Point getCellSize();
    };

    void outText(Context& ctx, Point pt, String_t text);
    void outText(Context& ctx, Point pt, const char* text);
    void outTextF(Context& ctx, Point pt, int maxWidth, String_t text);
    void outTextF(Context& ctx, Point pt, int maxWidth, const char* text);
    void outTextF(Context& ctx, const Rectangle& area, String_t text);
    void outTextF(Context& ctx, const Rectangle& area, const char* text);

}

#endif
