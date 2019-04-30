/**
  *  \file gfx/font.hpp
  */
#ifndef C2NG_GFX_FONT_HPP
#define C2NG_GFX_FONT_HPP

#include "afl/base/refcounted.hpp"
#include "gfx/point.hpp"
#include "afl/string/string.hpp"
#include "afl/base/deletable.hpp"
#include "afl/functional/mapping.hpp"

namespace gfx {

    template<typename Index>
    class Context;

    class BaseContext;
    class BaseColorScheme;
    class Rectangle;

    class Font : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Output text using the specified parameters.
            \param ctx    graphics context. outText() should use canvas, color, and alpha;
            and it should \em not use just_x/just_y.
            \param x,y    coordinates (top-left pixel)
            \param text   text */
        virtual void outText(BaseContext& ctx, Point pt, String_t text) = 0;
        virtual int getTextWidth(String_t text) = 0;
        virtual int getTextHeight(String_t text) = 0;

        // Derived methods
        int getEmWidth();
        int getLineHeight();
        Point getCellSize();

        int getMaxTextWidth(const afl::functional::Mapping<int,String_t>& tab);
    };

    void outText(BaseContext& ctx, Point pt, String_t text);
    void outText(BaseContext& ctx, Point pt, const char* text);

    template<typename Index>
    void outTextF(Context<Index>& ctx, Point pt, int maxWidth, String_t text);
    template<typename Index>
    void outTextF(Context<Index>& ctx, Point pt, int maxWidth, const char* text);
    template<typename Index>
    void outTextF(Context<Index>& ctx, const Rectangle& area, String_t text);
    template<typename Index>
    void outTextF(Context<Index>& ctx, const Rectangle& area, const char* text);

    void outTextF(BaseContext& ctx, BaseColorScheme& cs, Point pt, int maxWidth, String_t text);
    void outTextF(BaseContext& ctx, BaseColorScheme& cs, const Rectangle& r, String_t text);

}


template<typename Index>
void
gfx::outTextF(Context<Index>& ctx, Point pt, int maxWidth, String_t text)
{
    outTextF(ctx, ctx.colorScheme(), pt, maxWidth, text);
}

template<typename Index>
void
gfx::outTextF(Context<Index>& ctx, Point pt, int maxWidth, const char* text)
{
    outTextF(ctx, ctx.colorScheme(), pt, maxWidth, text);
}

template<typename Index>
void
gfx::outTextF(Context<Index>& ctx, const Rectangle& area, String_t text)
{
    outTextF(ctx, ctx.colorScheme(), area, text);
}

template<typename Index>
void
gfx::outTextF(Context<Index>& ctx, const Rectangle& area, const char* text)
{
    outTextF(ctx, ctx.colorScheme(), area, text);
}

#endif
