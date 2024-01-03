/**
  *  \file gfx/colorquantizer.hpp
  *  \brief Class gfx::ColorQuantizer
  */
#ifndef C2NG_GFX_COLORQUANTIZER_HPP
#define C2NG_GFX_COLORQUANTIZER_HPP

#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/types.hpp"

namespace gfx {

    class Canvas;

    /** Quantize colors into palettized image.

        For now, this class is intended for one-time, offline use, on reasonably-sized images,
        namely, to create PCC resource files using our custom file formats.
        It is not intended/optimized for general use.

        PCC1 had a bunch of converters from BMP to internal palettized images.
        Most of those used manual input.
        This one is derived from "replaceb", an end-user tool.

        Usage:
        - create
        - configure as needed
        - call quantize() as often as needed */
    class ColorQuantizer {
     public:
        /** Constructor.
            Create ColorQuantizer with default settings
            (entire palette range usable and dynamic). */
        ColorQuantizer();

        /** Create palettized image from canvas.

            Assigns dynamic colors and converts the image to the determined palette using simple dithering.

            @param can Canvas
            @return Palettized image */
        afl::base::Ref<PalettizedPixmap> quantize(Canvas& can) const;

        /** Set usable palette range.
            Resulting PalettizedPixmap objects will only use color indexes within the specified range.
            The range needs to be contiguous, overflow is not supported.

            @param from First usable color index
            @param count Number of usable color indexes (max. 256-from)

            @return this */
        ColorQuantizer& setUsablePaletteRange(uint8_t from, size_t count);

        /** Set dynamic palette range.
            Colors within this range will be dynamically allocated.
            The range needs to be a subset of the usable palette range.
            The range needs to be contiguous, overflow is not supported.

            @param from First dynamic color index
            @param count Number of dynamic color indexes (max. 256-from)

            @return this */
        ColorQuantizer& setDynamicPaletteRange(uint8_t from, size_t count);

        /** Set palette.
            This sets a set of palette entries.
            The first element from \c colorDefinitions is applied to slot @c start, the next one to @c start+1, and so on.
            The increment is applied "as if" a real 8-bit counter were incremented, i.e. 255 overflows back to 0.

            This function can be used to set elements of the usable, but not dynamic, range.
            Settings outside this range will be ignored.

            @param start First palette index
            @param colorDefinitions Color definitions

            @return this */
        ColorQuantizer& setPalette(uint8_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions);

        /** Set single palette entry.

            This function can be used to set elements of the usable, but not dynamic, range.
            Settings outside this range will be ignored.

            @param slot Palette index
            @param colorDefinition Color definition

            @return this */
        ColorQuantizer& setPalette(uint8_t slot, ColorQuad_t colorDefinition);

     private:
        ColorQuad_t m_palette[256];
        uint8_t m_firstUsable;
        uint8_t m_firstDynamic;
        size_t m_numUsable;
        size_t m_numDynamic;
    };

}

#endif
