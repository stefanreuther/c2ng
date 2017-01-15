/**
  *  \file ui/draw.cpp
  */

#include "ui/draw.hpp"
#include "gfx/complex.hpp"
#include "ui/colorscheme.hpp"
#include "ui/skincolorscheme.hpp"

using gfx::Rectangle;
using gfx::Point;

// FIXME: these probably belong elsewhere
/** Colors used to display something on a light-gray background.
    Examples include a light-gray window, or a standard list box
    entry. */
const ui::ColorSet ui::GRAY_COLOR_SET = {
    Color_Black,                // static
    Color_GreenScale+6,         // green
    Color_DarkYellowScale+6,    // yellow
    Color_Red,                  // red
    Color_White,                // white
    Color_Black,                // contrast
    Color_BlueGray,             // input
    Color_BlueBlack,            // blue
    Color_Dark,                 // faded
    Color_Black,                // heading
    Color_BlueBlack,            // selection
    Color_White,                // invstatic
    Color_Gray,                 // background
    Color_Shield+4,             // link
    Color_Shield+11,            // link shade
    Color_Shield+8,             // link focus
    { }
};

/** Colors used to display something on a dark-gray background.
    Examples include a dark-gray window, or a control screen. */
const ui::ColorSet ui::DARK_COLOR_SET = {
    Color_Gray,                 // static
    Color_Green,                // green
    Color_Yellow,               // yellow
    Color_Red,                  // red
    Color_White,                // white
    Color_White,                // contrast
    Color_Yellow,               // input
    Color_BlueGray,             // blue      // ???
    Color_Grayscale+4,          // faded     // ???
    Color_White,                // heading
    Color_Yellow,               // selection
    Color_Black,                // invstatic
    Color_Grayscale+4,          // background
    Color_Shield+12,            // link
    Color_Shield+8,             // link shade
    Color_Shield+6,             // link focus
    { }
};

/** Colors used to display something on a black background. Examples
    include the selection bar on a list box. */
const ui::ColorSet ui::BLACK_COLOR_SET = {
    Color_White,                // static
    Color_Green,                // green
    Color_Yellow,               // yellow
    Color_Red,                  // red       // FIXME: darker?
    Color_White,                // white
    Color_White,                // contrast
    Color_Gray,                 // input
    Color_BlueGray,             // blue
    Color_Dark,                 // faded
    Color_White,                // heading
    Color_Yellow,               // selection
    Color_Black,                // invstatic
    Color_Black,                // background
    Color_Shield+12,            // link
    Color_Shield+8,             // link shade
    Color_Shield+6,             // link focus
    { }
};

/** Colors used to display something bright on a light-gray
    background. This is used for the selection of a listbox when the
    listbox doesn't have focus. */
const ui::ColorSet ui::BRIGHT_GRAY_COLOR_SET = {
    Color_White,                // static
    Color_Green,                // green
    Color_Yellow,               // yellow
    Color_Red,                  // red
    Color_White,                // white
    Color_White,                // contrast
    Color_Gray,                 // input
    Color_BlueGray,             // blue
    Color_Dark,                 // faded
    Color_White,                // heading
    Color_BlueBlack,            // selection
    Color_Black,                // invstatic
    Color_Gray,                 // background
    Color_Shield+4,             // link
    Color_Shield+11,            // link shade
    Color_Shield+8,             // link focus
    { }
};


const ui::WindowStyle ui::BLUE_WINDOW       = { "winblue",  GRAY_COLOR_SET };
const ui::WindowStyle ui::BLUE_BLACK_WINDOW = { "winblue",  BLACK_COLOR_SET };
const ui::WindowStyle ui::BLUE_DARK_WINDOW  = { "winblue",  DARK_COLOR_SET };
const ui::WindowStyle ui::RED_WINDOW        = { "winred",   GRAY_COLOR_SET };
const ui::WindowStyle ui::GREEN_WINDOW      = { "wingreen", GRAY_COLOR_SET };


// /** Draw "up" frame. Like drawRectangle(), but the rectangle is colored
//     to give the effect of a 3D raised panel. */
// Precondition: ctx prepared with a ui::ColorScheme
void
ui::drawFrameUp(gfx::Context<uint8_t>& ctx, gfx::Rectangle r)
{
    // ex ui/util.h:drawFrameUp
    int x2 = r.getRightX() - 1;
    int y2 = r.getBottomY() - 1;

    ctx.setColor(Color_White);
    ctx.canvas().drawHLine(Point(r.getLeftX(), r.getTopY()),   x2-r.getLeftX(),  ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.canvas().drawVLine(Point(r.getLeftX(), r.getTopY()+1), y2-r.getTopY(),   ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.setColor(Color_Black);
    ctx.canvas().drawHLine(Point(r.getLeftX(), y2),            x2-r.getLeftX(),  ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.canvas().drawVLine(Point(x2, r.getTopY()),             y2+1-r.getTopY(), ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
}

// /** Draw "down" frame. Like drawRectangle(), but the rectangle is colored
//     to give the effect of a 3D lowered panel. */
// Precondition: ctx prepared with a ui::ColorScheme
void
ui::drawFrameDown(gfx::Context<uint8_t>& ctx, gfx::Rectangle r)
{
    // ex ui/util.h:drawFrameDown
    int x2 = r.getRightX() - 1;
    int y2 = r.getBottomY() - 1;

    ctx.setColor(Color_Black);
    ctx.canvas().drawHLine(Point(r.getLeftX(), r.getTopY()),   x2-r.getLeftX(),  ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.canvas().drawVLine(Point(r.getLeftX(), r.getTopY()+1), y2-r.getTopY(),   ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.setColor(Color_White);
    ctx.canvas().drawHLine(Point(r.getLeftX(), y2),            x2-r.getLeftX(),  ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
    ctx.canvas().drawVLine(Point(x2, r.getTopY()),             y2+1-r.getTopY(), ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
}


// /** Draw a window. Available as static function to be callable from
//     outside (widgets that look like windows but aren't, like the VCR
//     screen) */
// Precondition: ctx prepared with a ui::ColorScheme.
void
ui::drawWindow(gfx::Context<uint8_t>& ctx,
               const gfx::Rectangle& extent,
               gfx::ResourceProvider& provider,
               const WindowStyle& style,
               String_t name)
{
    // ex UIWindow::drawWindow
    afl::base::Ptr<gfx::Canvas> pix(provider.getImage(style.backgroundTile));

    drawFrameUp(ctx, extent);
    if (pix.get() != 0) {
        gfx::blitTiled(ctx, Rectangle(extent.getLeftX() + 1,  extent.getTopY() + 1,    extent.getWidth() - 2, 22),                      *pix, 16);
        gfx::blitTiled(ctx, Rectangle(extent.getLeftX() + 1,  extent.getTopY() + 23,   2,                     extent.getHeight() - 26), *pix, 16);
        gfx::blitTiled(ctx, Rectangle(extent.getRightX() - 3, extent.getTopY() + 23,   2,                     extent.getHeight() - 26), *pix, 16);
        gfx::blitTiled(ctx, Rectangle(extent.getLeftX() + 1,  extent.getBottomY() - 3, extent.getWidth() - 2, 2),                       *pix, 16);
    } else {
        gfx::drawSolidBar(ctx, Rectangle(extent.getLeftX() + 1,  extent.getTopY() + 1,    extent.getWidth() - 2, 22),                      Color_BlueBlack);
        gfx::drawSolidBar(ctx, Rectangle(extent.getLeftX() + 1,  extent.getTopY() + 23,   2,                     extent.getHeight() - 26), Color_BlueBlack);
        gfx::drawSolidBar(ctx, Rectangle(extent.getRightX() - 3, extent.getTopY() + 23,   2,                     extent.getHeight() - 26), Color_BlueBlack);
        gfx::drawSolidBar(ctx, Rectangle(extent.getLeftX() + 1,  extent.getBottomY() - 3, extent.getWidth() - 2, 2),                       Color_BlueBlack);
    }

    drawFrameDown(ctx, Rectangle(extent.getLeftX() + 3, extent.getTopY() + 23, extent.getWidth() - 6, extent.getHeight() - 26));

    // synchronize this with UIWindowSkin::drawBackground
    drawSolidBar(ctx, Rectangle(extent.getLeftX() + 4, extent.getTopY() + 24,
                                extent.getWidth() - 8, extent.getHeight() - 28),
                 style.colors[SkinColor::Background]);

    afl::base::Ref<gfx::Font> font(provider.getFont(gfx::FontRequest().addSize(1)));
    ctx.setColor(Color_White);
    ctx.setTextAlign(1, 0);
    ctx.useFont(*font);
    gfx::outTextF(ctx, Point(extent.getLeftX() + extent.getWidth()/2, extent.getTopY() + 2), extent.getWidth(), name);
}

namespace {
    /* Definition of a button shape. A number consists of a number of components
       that have different count and color depending on the button's state.

                 aaaaaaaaaaaaaaaaaaaaaaaaaaaa    a,b  left/top lines
                 abbbbbbbbbbbbbbbbbbbbbbbbbbc    c    bottom/right line
                 ab.........................c    ...  body (with text)
                 ab.........................c
                 ab.........................c
                 ab.........................c
                 accccccccccccccccccccccccccc

       The shapes defined here are a 99% approximation of the "standard" shape
       used in PCC 1.x. 1.x had the fun config setting of a "button thickness",
       but I omit that here. The setting was a remainder of the original PCC 1.x
       button style which had an even thicker frame. */

    struct ButtonShape {
        uint8_t num_left;         // Number of left/top lines
        uint8_t left[4];          // Colors of left/top lines
        uint8_t num_right;        // Number of right/bottom lines
        uint8_t right[4];         // Colors of right/bottom lines
        uint8_t body_color;       // Color of button body
    };

    const ButtonShape button_shapes[] = {
        // --- Normal Size ---
        // normal
        { 2, { ui::Color_White, ui::Color_Grayscale+10 }, 2, { ui::Color_Black, ui::Color_Grayscale+8 }, ui::Color_Grayscale+9 },
        { 2, { ui::Color_White, ui::Color_Grayscale+11 }, 2, { ui::Color_Black, ui::Color_Grayscale+9 }, ui::Color_Grayscale+10 },
        // normal highlight
        { 3, { ui::Color_Black, ui::Color_Grayscale+6, ui::Color_Grayscale+9 },  2, { ui::Color_White, ui::Color_Grayscale+13 }, ui::Color_Grayscale+12 },
        { 3, { ui::Color_Black, ui::Color_Grayscale+7, ui::Color_Grayscale+10 }, 2, { ui::Color_White, ui::Color_Grayscale+14 }, ui::Color_Grayscale+13 },
        // pressed
        { 3, { ui::Color_Black, ui::Color_Grayscale+13, ui::Color_Grayscale+11 }, 2, { ui::Color_Grayscale+3, ui::Color_Grayscale+9 },  ui::Color_Grayscale+9 },
        { 3, { ui::Color_Black, ui::Color_Grayscale+14, ui::Color_Grayscale+12 }, 2, { ui::Color_Grayscale+4, ui::Color_Grayscale+10 }, ui::Color_Grayscale+10 },
        // pressed highlight
        { 3, { ui::Color_Grayscale+0, ui::Color_Grayscale+5, ui::Color_Grayscale+7 }, 1, { ui::Color_Grayscale+15 }, ui::Color_Grayscale+12 },
        { 3, { ui::Color_Grayscale+1, ui::Color_Grayscale+6, ui::Color_Grayscale+8 }, 1, { ui::Color_Grayscale+15 }, ui::Color_Grayscale+13 },

        // --- Small Size ---
        // normal
        { 1, { ui::Color_White }, 1, { ui::Color_Black }, ui::Color_Grayscale+9 },
        { 1, { ui::Color_White }, 1, { ui::Color_Black }, ui::Color_Grayscale+10 },
        // normal highlight
        { 1, { ui::Color_White }, 1, { ui::Color_Black }, ui::Color_Grayscale+12 },
        { 1, { ui::Color_White }, 1, { ui::Color_Black }, ui::Color_Grayscale+13 },

        // pressed
        { 2, { ui::Color_Black, ui::Color_Grayscale+9 },  1, { ui::Color_White }, ui::Color_Grayscale+9 },
        { 2, { ui::Color_Black, ui::Color_Grayscale+9 },  1, { ui::Color_White }, ui::Color_Grayscale+10 },
        // pressed highlight
        { 2, { ui::Color_Black, ui::Color_Grayscale+12 }, 1, { ui::Color_White }, ui::Color_Grayscale+12 },
        { 2, { ui::Color_Black, ui::Color_Grayscale+12 }, 1, { ui::Color_White }, ui::Color_Grayscale+13 },
    };
}


// /** Draw a button. Exported to be callable from non-button widgets that contain
//     button-lookalikes, such as scrollbars.
//     \param can        Canvas to draw on
//     \param extent     Dimensions of the button
//     \param name       Text on button
//     \param flags      Style flags (bf_XXX, st_XXX) */
// Precondition: ctx has appropriate font, and a ui::ColorScheme, and text alignment
void
ui::drawButton(gfx::Context<uint8_t>& ctx,
               const gfx::Rectangle& extent,
               ButtonFlags_t flags,
               Widget::States_t state,
               String_t text)
{
    // Figure out font
    gfx::Font* font = ctx.getFont();
    if (font != 0) {
        // Convert flags into button_shapes index
        int slot = 0;
        if (font->getTextHeight("Tp") >= 16) {
            slot += 8;
        }
        if (flags.contains(PressedButton)) {
            slot += 4;
        }
        if (flags.contains(HighlightedButton)) {
            slot += 2;
        }
        if (flags.contains(ActiveButton) && !state.contains(Widget::DisabledState)) {
            slot += 1;
        }

        const ButtonShape& sh = button_shapes[slot];

        // draw button
        int ex = extent.getLeftX();
        int ey = extent.getTopY();
        int eh = extent.getHeight();
        int ew = extent.getWidth();
        int delta = 0;
        for (int i = 0; i < 4; ++i) {
            if (i < sh.num_left) {
                // upper-left corner
                ctx.setColor(sh.left[i]);
                ctx.canvas().drawHLine(gfx::Point(ex, ey),   ew,   ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
                ctx.canvas().drawVLine(gfx::Point(ex, ey+1), eh-1, ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
                ++ex;
                ++ey;
                --ew;
                --eh;
                ++delta;
            }
            if (i < sh.num_right) {
                // lower-right corner
                ctx.setColor(sh.right[i]);
                ctx.canvas().drawHLine(gfx::Point(ex,      ey+eh-1), ew,   ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
                ctx.canvas().drawVLine(gfx::Point(ex+ew-1, ey),      eh-1, ctx.getRawColor(), gfx::SOLID_LINE, gfx::OPAQUE_ALPHA);
                --ew;
                --eh;
                --delta;
            }
        }
        ctx.setColor(sh.body_color);
        ctx.canvas().drawBar(gfx::Rectangle(ex, ey, ew, eh), ctx.getRawColor(), gfx::TRANSPARENT_COLOR, gfx::FillPattern::SOLID, gfx::OPAQUE_ALPHA);

        // draw text
        int x = ex + ctx.getTextAlign().getY()*(ew-16)/2 + delta + 8;
        int y = ey + ctx.getTextAlign().getY()*eh/2 + delta;
        if (state.contains(ui::Widget::DisabledState)) {
            ctx.setColor(ui::Color_Black);
        } else {
            ctx.setColor(ui::Color_Shield + 5);
        }
        outText(ctx, gfx::Point(x, y), text);
    }
}

void
ui::prepareHighContrastListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state)
{
    // ex UIListbox::prepareEntry (part 1)
    switch (state) {
     case ui::widgets::AbstractListbox::DisabledItem:
        drawBackground(ctx, area);
        ctx.setColor(SkinColor::Faded);
        break;

     case ui::widgets::AbstractListbox::FocusedItem:
        drawSolidBar(ctx, area, SkinColor::Static);
        ctx.setColor(SkinColor::InvStatic);
        break;

     case ui::widgets::AbstractListbox::ActiveItem:
        drawBackground(ctx, area);
        ctx.setColor(SkinColor::Static);
        drawRectangle(ctx, area);
        break;

     case ui::widgets::AbstractListbox::PassiveItem:
        drawBackground(ctx, area);
        ctx.setColor(SkinColor::Static);
        break;
    }
}

void
ui::prepareColorListItem(gfx::Context<util::SkinColor::Color>& ctx, gfx::Rectangle area, ui::widgets::AbstractListbox::ItemState state, ColorScheme& uiColors, afl::base::Deleter& h)
{
    // ex UIListbox::prepareEntry (part 2)
    // CursorlessColorStyle means: use PassiveItem instead of ActiveItem/FocusedItem
    switch (state) {
     case ui::widgets::AbstractListbox::DisabledItem:
        ctx.useColorScheme(h.addNew(new SkinColorScheme(GRAY_COLOR_SET, uiColors)));
        ctx.setColor(SkinColor::Faded);
        break;

     case ui::widgets::AbstractListbox::FocusedItem:
        ctx.useColorScheme(h.addNew(new SkinColorScheme(BLACK_COLOR_SET, uiColors)));
        ctx.setColor(SkinColor::Static);
        break;

     case ui::widgets::AbstractListbox::ActiveItem:
        ctx.useColorScheme(h.addNew(new SkinColorScheme(BRIGHT_GRAY_COLOR_SET, uiColors)));
        ctx.setColor(SkinColor::Static);
        break;

     case ui::widgets::AbstractListbox::PassiveItem:
        ctx.useColorScheme(h.addNew(new SkinColorScheme(GRAY_COLOR_SET, uiColors)));
        ctx.setColor(SkinColor::Static);
        break;
    }
    drawBackground(ctx, area);
}
