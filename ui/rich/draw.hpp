/**
  *  \file ui/rich/draw.hpp
  *  \brief Simple Rich-Text Drawing Functions
  */
#ifndef C2NG_UI_RICH_DRAW_HPP
#define C2NG_UI_RICH_DRAW_HPP

#include "util/rich/text.hpp"
#include "gfx/context.hpp"
#include "gfx/point.hpp"
#include "gfx/resourceprovider.hpp"
#include "util/skincolor.hpp"

namespace ui { namespace rich {

    /** Get width of text.
        Similar to gfx::Font::getTextWidth(), but for rich text that uses different attributes.
        \param text Text
        \param provider ResourceProvider to access fonts
        \return width in pixels
        \see ui::rich::outText() */
    int getTextWidth(const util::rich::Text& text, gfx::ResourceProvider& provider);

    /** Get height of text.
        Similar to gfx::Font::getTextHeight(), but for rich text that uses different attributes.
        \param text Text
        \param provider ResourceProvider to access fonts
        \return height in pixels
        \see ui::rich::outText() */
    int getTextHeight(const util::rich::Text& text, gfx::ResourceProvider& provider);

    /** Get size of text.
        Returns width and height in one call.
        \param text Text
        \param provider ResourceProvider to access fonts
        \return point containing X=width, Y=height
        \see ui::rich::outText() */
    gfx::Point getTextSize(const util::rich::Text& text, gfx::ResourceProvider& provider);

    /** Write line of rich text.
        This is a simpler version than ui::rich::Document to produce text-with-attributes.
        It supports
        - colors
        - fonts
        - underlining
        - text alignment according to the gfx::Context
        It does not support
        - word-wrap
        - keycaps
        - links

        Text X alignment works as expected and aligns the whole line.
        Y alignment aligns individual parts.
        This means that with Y-alignment 0, all parts are aligned at their top,
        which is usually not what you want when using different font heights.

        \param ctx Context (using a SkinColor color scheme)
        \param pt Origin
        \param text Text
        \param provider ResourceProvider to access fonts */
    void outText(gfx::Context<util::SkinColor::Color>& ctx, gfx::Point pt, const util::rich::Text& text, gfx::ResourceProvider& provider);

} }

#endif
