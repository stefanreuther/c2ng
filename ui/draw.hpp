/**
  *  \file ui/draw.hpp
  */
#ifndef C2NG_UI_DRAW_HPP
#define C2NG_UI_DRAW_HPP

#include "afl/base/types.hpp"
#include "gfx/context.hpp"
#include "gfx/rectangle.hpp"
#include "afl/string/string.hpp"
#include "gfx/resourceprovider.hpp"
#include "afl/bits/smallset.hpp"
#include "ui/widget.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "util/skincolor.hpp"

namespace ui {

    const int DefaultFont = 0;
    const int FixedFont = 1;

    class ColorScheme;

    using util::SkinColor;

    struct ColorSet {
        uint8_t m_colors[SkinColor::NUM_COLORS];

        // This dummy element is to allow recognition of initialisation errors.
        // You can write 'UIColorSet x = { a,b,c, {} }' and the compiler will tell you when you have too few colors.
        struct dummy { } m_dummy;

        uint8_t operator[](SkinColor::Color i) const
            { return m_colors[i]; }
    };
    extern const ColorSet GRAY_COLOR_SET, DARK_COLOR_SET, BLACK_COLOR_SET, BRIGHT_GRAY_COLOR_SET;

    enum ButtonFlag {
        ActiveButton,           ///< Active (ex st_Selected)
        PressedButton,          ///< Pressed (ex bf_Pressed)
        HighlightedButton       ///< Highlighted (ex bf_Highlight)
    };
    typedef afl::bits::SmallSet<ButtonFlag> ButtonFlags_t;

    struct WindowStyle {
        const char* backgroundTile;
        const ColorSet& colors;
    };
    extern const WindowStyle BLUE_WINDOW, BLUE_BLACK_WINDOW, BLUE_DARK_WINDOW, RED_WINDOW, GREEN_WINDOW;

    void drawFrameUp(gfx::Context<uint8_t>& ctx, gfx::Rectangle r);
    void drawFrameDown(gfx::Context<uint8_t>& ctx, gfx::Rectangle r);

    void drawWindow(gfx::Context<uint8_t>& ctx,
                    const gfx::Rectangle& extent,
                    gfx::ResourceProvider& provider,
                    const WindowStyle& style,
                    String_t name);

    void drawButton(gfx::Context<uint8_t>& ctx,
                    const gfx::Rectangle& extent,
                    ButtonFlags_t flags,
                    Widget::States_t state,
                    String_t text);

    void prepareHighContrastListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state);
    void prepareColorListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state, ColorScheme& uiColors, afl::base::Deleter& h);
}

#endif
