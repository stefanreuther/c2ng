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

    const uint32_t CURSOR_BLINK_INTERVAL = 400;

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
        HighlightedButton,      ///< Highlighted (ex bf_Highlight)
        FocusedButton,          ///< Forwarding of FocusedState
        DisabledButton          ///< Forwarding of DisabledState
    };
    typedef afl::bits::SmallSet<ButtonFlag> ButtonFlags_t;

    struct WindowStyle {
        const char* backgroundTile;
        const ColorSet& colors;
    };
    extern const WindowStyle BLUE_WINDOW, BLUE_BLACK_WINDOW, BLUE_DARK_WINDOW, RED_WINDOW, GREEN_WINDOW;

    void drawFrameUp(gfx::Context<uint8_t>& ctx, gfx::Rectangle r);
    void drawFrameDown(gfx::Context<uint8_t>& ctx, gfx::Rectangle r);

    /** Tile area with pixmap.
        \param ctx    graphics context
        \param r      area to tile with pixmap
        \param pix    pixmap to use, may be null
        \param color  when /pix/ is null, the image is filled with this color
        \param alter  X coordinate alteration. With alteration 0, the area is
                      tiled with a regular grid, like on a checkered paper.
                      With nonzero alteration, the second row is shifted
                      that many pixels to the left, the next one is shifted
                      to the right again, etc, to make the pattern look more
                      interesting. */
    void drawTiledArea(gfx::Context<uint8_t>& ctx, gfx::Rectangle r, const afl::base::Ptr<gfx::Canvas>& pix, uint8_t color, int alter);

    void drawWindow(gfx::Context<uint8_t>& ctx,
                    const gfx::Rectangle& extent,
                    gfx::ResourceProvider& provider,
                    const WindowStyle& style,
                    String_t name);

    /** Draw a button.
        Exported to be callable from non-button widgets that contain button-lookalikes, such as scrollbars.
        \param ctx     Context, with appropriate font, ui::ColorScheme, and text alignment set
        \param extent  Dimensions of the button
        \param flags   Button flags
        \param text    Label */
    void drawButton(gfx::Context<uint8_t>& ctx, const gfx::Rectangle& extent, ButtonFlags_t flags, String_t text);

    void prepareHighContrastListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state);
    void prepareColorListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state, ColorScheme& uiColors, afl::base::Deleter& h);

    enum FrameType {
        NoFrame,            ///< Don't draw a frame. ex WColorFrame cf_None, using fw=2
        RedFrame,           ///< Red frame.          ex WColorFrame cf_Red, using fw=2
        YellowFrame,        ///< Yellow frame.       ex WColorFrame cf_Yellow, using fw=2
        GreenFrame,         ///< Green frame.        ex WColorFrame cf_Green, using fw=2
        RaisedFrame,        ///< Raised 3D frame.
        LoweredFrame        ///< Lowered 3D frame.   ex UIFrameGroup, using fw=1, pad=frame-1
    };
    void drawFrame(gfx::Context<uint8_t>& ctx, gfx::Rectangle r, FrameType type, int frameWidth);
}

#endif
