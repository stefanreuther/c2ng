/**
  *  \file gfx/font.hpp
  *  \brief Class gfx::Font
  */
#ifndef C2NG_GFX_FONT_HPP
#define C2NG_GFX_FONT_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/functional/mapping.hpp"
#include "afl/string/string.hpp"
#include "gfx/point.hpp"

namespace gfx {

    template<typename Index>
    class Context;

    class BaseContext;
    class BaseColorScheme;
    class Rectangle;

    /** Font.
        Implements simple text rendering.
        Each piece of text is fit into a rectangle.
        Details like baseline, descenders etc. are currently not handled.
        An implementation can (but doesn't have to) handle kerning/ligatures. */
    class Font : public afl::base::Deletable, public afl::base::RefCounted {
     public:
        /** Output text using the specified parameters.
            \param ctx  Graphics context. outText() should use canvas, color, and alpha; it must not use the text alignment parameters.
            \param pt   Position (top-left pixel)
            \param text Text in UTF-8 encoding */
        virtual void outText(BaseContext& ctx, Point pt, String_t text) = 0;

        /** Get text width.
            \param text Text in UTF-8 encoding
            \return Number of pixels the text will occupy in X direction */
        virtual int getTextWidth(String_t text) = 0;

        /** Get text height.
            \param text Text in UTF-8 encoding
            \return Number of pixels the text will occupy in Y direction */
        virtual int getTextHeight(String_t text) = 0;

        // Derived methods

        /** Get "em" width.
            This provides a coarse estimate of this fonts size and can be used for widget size estimations.
            \return width of "m". */
        int getEmWidth();

        /** Get line height.
            This provides an estimate of this fonts height.
            \return height */
        int getLineHeight();

        /** Get cell size.
            This provides a coarse estimate of this fonts size and can be used for widget size estimations.
            \return Point(getEmWidth(), getLineHeight()) */
        Point getCellSize();

        /** Get maximum text width.
            \param tab Mapping producing a set of strings
            \return maximum width */
        int getMaxTextWidth(const afl::functional::Mapping<int,String_t>& tab);
    };

    /** Output Text, using Alignment Parameters.
        \param ctx    Output
        \param pt     Anchor point
        \param text   Text to output */
    void outText(BaseContext& ctx, Point pt, String_t text);

    /** Output Text, using Alignment Parameters.
        \param ctx    Output
        \param pt     Anchor point
        \param text   Text to output (C string) */
    void outText(BaseContext& ctx, Point pt, const char* text);

    /** Output Text with fixed maximum width.
        \see outTextF(BaseContext&, BaseColorScheme&, Point, int, String_t)
        \param ctx      Context (foreground and background colors)
        \param pt       Anchor position
        \param maxWidth Maximum width
        \param text     Text in UTF-8 encoding */
    template<typename Index>
    void outTextF(Context<Index>& ctx, Point pt, int maxWidth, String_t text);

    /** Output Text with fixed maximum width.
        \see outTextF(BaseContext&, BaseColorScheme&, Point, int, String_t)
        \param ctx      Context (foreground and background colors)
        \param pt       Anchor position
        \param maxWidth Maximum width
        \param text     Text in UTF-8 encoding */
    template<typename Index>
    void outTextF(Context<Index>& ctx, Point pt, int maxWidth, const char* text);

    /** Output Text with fixed area.
        \see outTextF(BaseContext&, BaseColorScheme&, const Rectangle&, String_t)
        \param ctx      Context (foreground and background colors)
        \param area     Area
        \param text     Text in UTF-8 encoding */
    template<typename Index>
    void outTextF(Context<Index>& ctx, const Rectangle& area, String_t text);

    /** Output Text with fixed area.
        \see outTextF(BaseContext&, BaseColorScheme&, const Rectangle&, String_t)
        \param ctx      Context (foreground and background colors)
        \param area     Area
        \param text     Text in UTF-8 encoding */
    template<typename Index>
    void outTextF(Context<Index>& ctx, const Rectangle& area, const char* text);

    /** Output Text with fixed maximum width.
        Outputs the text so that it occupies at most \c maxWidth pixels on the screen.
        When non-transparent output is used, draws appropriate boxes so that exactly the specified area is covered.
        Otherwise like outText().

        This function moves the graphics cursor, depending on the text justification:
        - when using left-justified text, the cursor is set to just after the output text;
        - when using right-justified text, the cursor is set to just before the output text;
        - when using centered text, the cursor is set to (x,y)

        The rationale is that (when not using centered output) one can do
        several outText()s in a row and always know the correct place without
        explicitly having to compute the width.

        \param ctx      Context (foreground and background colors)
        \param cs       Color scheme (for background)
        \param pt       Anchor point
        \param maxWidth Maximum width
        \param text     Text in UTF-8 encoding */
    void outTextF(BaseContext& ctx, BaseColorScheme& cs, Point pt, int maxWidth, String_t text);

    /** Output Text with fixed area.
        This fills the area above and below the text as well.

        Note: this does not enforce a maximum height; therefore, the area
        should be at least as tall as the text.

        \param ctx  Context
        \param cs   Color scheme (for background)
        \param r    Area to fill
        \param text Text to write */
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
