/**
  *  \file gfx/palettizedpixmap.hpp
  *  \brief Class gfx::PalettizedPixmap
  */
#ifndef C2NG_GFX_PALETTIZEDPIXMAP_HPP
#define C2NG_GFX_PALETTIZEDPIXMAP_HPP

#include "gfx/pixmap.hpp"
#include "afl/base/types.hpp"
#include "gfx/types.hpp"
#include "afl/base/ref.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/uncopyable.hpp"
#include "gfx/canvas.hpp"

namespace gfx {

    /** Palettized pixmap.
        Implements a one-byte-per-pixel, 256 color, palettized pixmap.

        PalettizedPixmap implements the pixmap interface that allows you to access the pixels and the palette directly.
        Use makeCanvas() to obtain a Canvas you can use with normal drawing functions.

        PalettizedPixmap is always allocated on the heap. */
    class PalettizedPixmap : public afl::base::RefCounted,
                             public Pixmap<uint8_t>,
                             private afl::base::Uncopyable
    {
     public:
        /** Create a PalettizedPixmap.
            \param w Width in pixels
            \param h Height in pixels
            \return newly-allocated PalettizedPixmap */
        static afl::base::Ref<PalettizedPixmap> create(int w, int h);

        /** Set palette.
            This sets a set of palette entries.
            The first element from \c colorDefinitions is applied to slot \c start, the next one to \c start+1, and so on.
            The increment is applied "as if" a real 8-bit counter were incremented, i.e. 255 overflows back to 0.
            \param start First palette index
            \param colorDefinitions Color definitions */
        void setPalette(uint8_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions);

        /** Set single palette entry.
            \param slot Palette index
            \param colorDefinition Color definition */
        void setPalette(uint8_t slot, ColorQuad_t colorDefinition);

        /** Get palette.
            This retrieves a set of palette entries.
            The first element from \c colorDefinitions receives the value of slot \c start, the next one is \c start+1, and so on.
            The increment is applied "as if" a real 8-bit counter were incremented, i.e. 255 overflows back to 0.
            \param [in] start First palette index
            \param [out] colorDefinitions Color definitions */
        void getPalette(uint8_t start, afl::base::Memory<ColorQuad_t> colorDefinitions) const;

        /** Find nearest color, given a ColorQuad_t.
            \param def Color
            \return nearest palette entry. If multiple entries are equally close, returns the numerically lowest one. */
        uint8_t findNearestColor(ColorQuad_t def) const;

        /** Create canvas to draw on.
            \return newly-allocated canvas */
        afl::base::Ref<Canvas> makeCanvas();

     private:
        /** Constructor.
            \param w Width in pixels
            \param h Height in pixels */
        inline PalettizedPixmap(int w, int h);

        class CanvasImpl;
        friend class CanvasImpl;
        class TraitsImpl;
        friend class TraitsImpl;

        ColorQuad_t m_palette[256];
    };

}

#endif
