/**
  *  \file ui/reshack/palette.hpp
  *  \brief Class ui::reshack::Palette
  */
#ifndef C2NG_UI_RESHACK_PALETTE_HPP
#define C2NG_UI_RESHACK_PALETTE_HPP

#include "afl/base/ref.hpp"
#include "gfx/canvas.hpp"
#include "gfx/palettizedpixmap.hpp"

namespace ui { namespace reshack {

    /** Palette handling for c2reshack.

        @change This has been radically simplified compared to PCC2.
        In c2ng, we only support palettized images using our standard palette,
        because our file formats support only those anyway.
        For editing no-palettized, regular editing tools can be used.

        In c2ng, Painter always works on PalettizedPixmap.
        The palette in PalettizedPixmap is always up-to-date.
        For StandardPaletteColor, the variable slots of the palette are active on the screen palette;
        a change to the palette updates the screen.
        For GrayscaleColor, we also use the standard palette, but screen is not editable. */
    class Palette {
     public:
        /** Color mode. */
        enum ColorMode {
            /** Image uses standard palette colors.
                That is, the first Color_Avail colors equal our palette and are not modifyable,
                the following colors are variable. */
            StandardPaletteColor,

            /** Image uses grayscales (for font editing).
                This is a standard-palette image, but palette is not editable. */
            GrayscaleColor
        };

        /** Font Colors (for GrayscaleColor). */
        enum {
            FC_Black = 0,
            FC_Half  = 23,
            FC_White = 15
        };

        /** Make arbitrary canvas editable.
            This always copies the pixel data.
            If the canvas is palettized and matches our palette, the pixel data is taken over verbatim.
            Otherwise, the image is quantized before being converted.
            @param pix Input pixmap
            @return Editable PalettizedPixmap */
        static afl::base::Ref<gfx::PalettizedPixmap> makeEditable(afl::base::Ref<gfx::Canvas> pix);

        /** Copy palette from Canvas to PalettizedPixmap.
            @param out Target pixmap
            @param in  Source canvas */
        static void copyPalette(gfx::PalettizedPixmap& out, gfx::Canvas& in);

        /** Check whether user is permitted to edit the specified color in a color mode.
            @param cm color mode
            @param color color number */
        static bool isEditableColor(ColorMode cm, uint8_t color);
    };

} }

#endif
