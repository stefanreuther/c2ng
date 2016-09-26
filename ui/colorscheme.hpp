/**
  *  \file ui/colorscheme.hpp
  */
#ifndef C2NG_UI_COLORSCHEME_HPP
#define C2NG_UI_COLORSCHEME_HPP

#include "gfx/types.hpp"
#include "afl/base/types.hpp"
#include "gfx/colorscheme.hpp"
#include "gfx/canvas.hpp"

namespace ui {

    /** Symbolic Names for Standard Colors. The palette is "historically grown"
        from PCC v1. PCC v1 replicated slots 0 to 15 as 16..31 and 32..47,
        to simplify display of multiple 16-color pictures. We do no longer do
        that, so we have 96 free color slots available for picture mixing, not
        just 64.

        Note that the image loading code in pixmap.cc depends on the palette
        contents (and the images, too, of course). */
    enum {
        /* PCC v1 16-color palette */
        Color_Black      = 0,
        Color_Dark       = 1,
        Color_Gray       = 2,
        Color_Green      = 3,
        Color_Red        = 4,
        Color_GreenBlack = 5,
        Color_Blue       = 6,
        Color_BlueGray   = 7,
        Color_BlueBlack  = 8,
        Color_Yellow     = 9,
        Color_Vcr3       = 10,       ///< 3 colors for VCR. Also used by 16-color images in 16-color mode in PCC v1
        Color_Unused2    = 13,       ///< 2 slots unused here. Used by 16-color images in 16-color mode in PCC v1
        Color_White      = 15,

        /* PCC v1 256-color palette, moved by -32 slots */
        Color_Grayscale     = 16,    ///< Grayscales. (COLOR_BLACK, COLOR_GRAYSCALE, ... COLOR_GRAYSCALE + 15)
        Color_Fire          = 32,    ///< Fire colors. black/red/yellow/white gradient (COLOR_FIRE, ..., COLOR_FIRE + 31, COLOR_WHITE)
        Color_Shield        = 64,    ///< Shield colors. black/blue/white gradient (COLOR_SHIELD, ..., COLOR_SHIELD + 15)
        Color_Status        = 80,    ///< Status bar colors. red/green gradient (COLOR_STATUS, ..., COLOR_STATUS + 15)

        Color_BlueberryIce  = 96,    ///< Blueberry ice/yoghurt. How'd you call that?

        /* the following colors are available as 11 to 30 in CCScript,
           and they are used to map UFO colors. */
        Color_DarkBlue         = 97,
        Color_BrightBlue       = 98,
        Color_DarkGreen        = 99,
        Color_BrightGreen      = 100,
        Color_DarkCyan         = 101,
        Color_BrightCyan       = 102,
        Color_DarkRed          = 103,
        Color_BrightRed        = 104,
        Color_DarkMagenta      = 105,
        Color_BrightMagenta    = 106,
        Color_DarkYellow       = 107, ///< pretty dirty-looking yellow
        Color_BrightYellow     = 108,
        Color_Brown            = 109, ///< greenish brown
        Color_BrightBrown      = 110, ///< yellowish brown
        Color_Pink             = 111,
        Color_BrightPink       = 112, ///< body-color pink
        Color_Orange           = 113,
        Color_BrightOrange     = 114, ///< yellowish orange
        Color_Blueberry        = 115,
        Color_BluePink         = 116, ///< blue'ish pink

        /* Miscellaneous */
        Color_GreenScale6      = 117, ///< 6 scales of green (COLOR_GREENBLACK, COLOR_GREENSCALE, ... COLOR_GREENSCALE + 5, COLOR_GREEN)
        Color_DarkPink         = 123, ///< pretty dark pink
        Color_DarkYellowScale3 = 124, ///< 3 very dark yellow scales; used in the title backdrop (COLOR_DARKYELLOWSCALE3, ... COLOR_DARKYELLOWSCALE3 + 2)
        Color_DarkPink2        = 127, ///< not so dark pink
        Color_GreenScale       = 128, ///< 16 scales of green (COLOR_BLACK, COLOR_GREENSCALE, ... COLOR_GREENSCALE + 15)
        Color_DarkYellowScale  = 144, ///< 8 scales of yellow (COLOR_BLACK, COLOR_DARKYELLOWSCALE, ... COLOR_DARKYELLOWSCALE + 7)
    
        Color_Unused8          = 152, ///< 8 slots unused

        /* colors available for image mixing */
        Color_Avail            = 160
    };

    extern const gfx::ColorQuad_t STANDARD_COLORS[Color_Avail];

    class ColorScheme : public gfx::ColorScheme {
     public:
        ColorScheme();
        virtual ~ColorScheme();
        virtual gfx::Color_t getColor(uint32_t index);
        virtual void drawBackground(gfx::Context& ctx, const gfx::Rectangle& area);

        void init(gfx::Canvas& can);

     private:
        gfx::Color_t m_colors[Color_Avail];
    };
}

#endif
