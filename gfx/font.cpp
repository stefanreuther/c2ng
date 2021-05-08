/**
  *  \file gfx/font.cpp
  */

#include <algorithm>
#include "gfx/font.hpp"
#include "gfx/context.hpp"
#include "afl/charset/utf8.hpp"
#include "gfx/colorscheme.hpp"
#include "gfx/rectangle.hpp"
#include "gfx/basecolorscheme.hpp"

int
gfx::Font::getEmWidth()
{
    return getTextWidth("m");
}

int
gfx::Font::getLineHeight()
{
    return getTextHeight("Tp");
}

gfx::Point
gfx::Font::getCellSize()
{
    return Point(getEmWidth(), getLineHeight());
}

int
gfx::Font::getMaxTextWidth(const afl::functional::Mapping<int,String_t>& tab)
{
    // ex getTextWidth(GfxFont& font, const StringTable& table)
    int result = 0;
    int i;
    for (bool ok = tab.getFirstKey(i); ok; ok = tab.getNextKey(i)) {
        result = std::max(result, getTextWidth(tab(i)));
    }
    return result;
}


// /** Output Text, using Alignment Parameters.
//     \param ctx    Output
//     \param x,y    Anchor point
//     \param text   Text to output */
void
gfx::outText(BaseContext& ctx, Point pt, String_t text)
{
    if (Font* fnt = ctx.getFont()) {
        Point align = ctx.getTextAlign();
        if (int x = align.getX()) {
            pt.addX(-(fnt->getTextWidth(text) * x / 2));
        }
        if (int y = align.getY()) {
            pt.addY(-(fnt->getTextHeight(text) * y / 2));
        }
        fnt->outText(ctx, pt, text);
    }
}

// /** Output Text, using Alignment Parameters.
//     \overload */
void
gfx::outText(BaseContext& ctx, Point pt, const char* text)
{
    outText(ctx, pt, String_t(text));
}

// /** Output Text with fixed maximum width. Outputs the text so that it
//     occupies at most maxWidth pixels on the screen. When non-transparent output
//     is used, draws appropriate boxes so that exactly the specified area is
//     covered. Otherwise like outText().

//     This function moves the graphics cursor, depending on the text
//     justification:
//     - when using left-justified text, the cursor is set to just after
//       the output text;
//     - when using right-justified text, the cursor is set to just before
//       the output text;
//     - when using centered text, the cursor is set to (x,y)

//     The rationale is that (when not using centered output) one can do
//     several outText()s in a row and always know the correct place without
//     explicitly having to compute the width. */
void
gfx::outTextF(BaseContext& ctx, BaseColorScheme& cs, Point pt, int maxWidth, String_t text)
{
    if (Font* fnt = ctx.getFont()) {
        // Limit text width
        int width;
        while (1) {
            width = fnt->getTextWidth(text);
            if (width <= maxWidth || text.empty()) {
                break;
            }

            // We must remove some characters.
            // Make sure to not hit a UTF-8 rune in the middle.
            size_t n = text.length() - 1;
            while (n > 0 && afl::charset::Utf8::isContinuationByte(text[n])) {
                --n;
            }
            text.erase(n);
        }

        // Adjust height for alignment
        Point align = ctx.getTextAlign();
        int height = fnt->getTextHeight(text);
        int newCursorY = pt.getY();
        int x = pt.getX();
        int y = pt.getY() - height * align.getY() / 2;

        switch (align.getX()) {
         case 0:
            /* Left */
            if (!ctx.isTransparentBackground()) {
                cs.drawBackground(ctx.canvas(), Rectangle(x, y, maxWidth, height));
            }
            fnt->outText(ctx, Point(x, y), text);
            ctx.setCursor(Point(x + width, newCursorY));
            break;
         case 1: {
            /* Centered */
            int xl = x - maxWidth/2;
            int xtl = x - width/2;
            if (!ctx.isTransparentBackground()) {
                cs.drawBackground(ctx.canvas(), Rectangle(xl, y, maxWidth, height));
            }
            fnt->outText(ctx, Point(xtl, y), text);
            ctx.setCursor(Point(x, newCursorY));
            break;
         }
         case 2:
            /* Right */
            if (!ctx.isTransparentBackground()) {
                cs.drawBackground(ctx.canvas(), Rectangle(x - maxWidth, y, maxWidth, height));
            }
            fnt->outText(ctx, Point(x - width, y), text);
            ctx.setCursor(Point(x - width, newCursorY));
            break;
         default:
            break;
        }
    }
}

// // /** Output Text with fixed maximum width. \overload */
// void
// gfx::outTextF(BaseContext& ctx, Point pt, int maxWidth, const char* text)
// {
//     outTextF(ctx, pt, maxWidth, String_t(text));
// }

// /** Output Text with fixed area.
//     See outTextF(GfxContext&,int,int,int,string_t) for more information.
//     This fills the area above and below the text as well.

//     Note: this does not enforce a maximum height; therefore, the area
//     should be at least as tall as the text.

//     \param ctx  Context
//     \param area Area to fill
//     \param text Text to write */
void
gfx::outTextF(BaseContext& ctx, BaseColorScheme& cs, const Rectangle& area, String_t text)
{
    Font* fnt = ctx.getFont();
    if (fnt != 0 && area.getWidth() != 0) {
        const int height  = fnt->getTextHeight(text);
        const int align   = ctx.getTextAlign().getY();
        const int originY = area.getTopY() + area.getHeight() * align/2;

        // If filled background is requested, fill.
        if (height < area.getHeight() && !ctx.isTransparentBackground()) {
            const int topY = originY - height * align/2;
            if (topY > area.getTopY()) {
                cs.drawBackground(ctx.canvas(), Rectangle(area.getLeftX(), area.getTopY(), area.getWidth(), topY - area.getTopY()));
            }

            const int botY = topY + height;
            if (botY < area.getBottomY()) {
                cs.drawBackground(ctx.canvas(), Rectangle(area.getLeftX(), botY, area.getWidth(), area.getBottomY() - botY));
            }
        }

        // Draw the text
        outTextF(ctx, cs, Point(area.getLeftX() + ctx.getTextAlign().getX() * area.getWidth() / 2, originY), area.getWidth(), text);
    }
}

// // /** Output Text with fixed area. \overload */
// void
// gfx::outTextF(BaseContext& ctx, const Rectangle& area, const char* text)
// {
//     outTextF(ctx, area, String_t(text));
// }
