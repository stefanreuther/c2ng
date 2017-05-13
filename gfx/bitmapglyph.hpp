/**
  *  \file gfx/bitmapglyph.hpp
  *  \brief Class gfx::BitmapGlyph
  */
#ifndef C2NG_GFX_BITMAPGLYPH_HPP
#define C2NG_GFX_BITMAPGLYPH_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "gfx/point.hpp"
#include "gfx/types.hpp"

namespace gfx {

    class BaseContext;
    class Canvas;

    /** Glyph for a bitmap font.
        A glyph contains a bitmap that is drawn normally, plus an optional list of half-intensity pixels for pseudo-anti-aliasing.

        The bitmap data is stored as an array of bytes, with an integer number of bytes per line.
        The leftmost column is bit 0x80 in the first byte; the topmost line is the first bytes in the array.
        If the width is not divisible by 8, the lower-valued bits of the final byte are unused. */
    class BitmapGlyph {
     public:
        /** Construct an empty glyph of zero size. */
        BitmapGlyph();

        /** Construct a blank glyph of a given size.
            \param width  [in] Width in pixels
            \param height [in] Height in pixels */
        BitmapGlyph(uint16_t width, uint16_t height);

        /** Construct glyph from bitmap data.
            \param width  [in] Width in pixels
            \param height [in] Height in pixels
            \param data   [in] Refers to getBytesForSize(width,height) bytes containing bitmap data. See class description. */
        BitmapGlyph(uint16_t width, uint16_t height, const uint8_t* data);

        /** Destructor. */
        ~BitmapGlyph();

        /** Add anti-aliasing hint.
            Specifies that the pixel at (x,y) should be drawn in half intensity.
            \param x X-coordinate [0,width)
            \param y Y-coordinate [0,height) */
        void addAAHint(uint16_t x, uint16_t y);

        /** Get height of this glyph in pixels.
            \return height */
        int getHeight() const;

        /** Get width of this glyph in pixels.
            \return width */
        int getWidth() const;

        /** Draw this glyph.
            \param ctx Context. Used attributes are canvas, color, and alpha.
            \param pt Position */
        void draw(BaseContext& ctx, Point pt) const;

        /** Draw this glyph with defined colors.
            This always uses OPAQUE_ALPHA.
            \param can Canvas
            \param pt Position
            \param pixel_color Color value for regular pixels
            \param aa_color Color value for half-intensity pixels */
        void drawColored(Canvas& can, Point pt, Color_t pixel_color, Color_t aa_color) const;

        /** Set pixel value.
            \param x,y Position. If out of range, the call is ignored.
            \param value Value */
        void set(int x, int y, bool value);

        /** Get pixel value.
            \param x,y Position
            \return pixel value; fals if position is out of range */
        bool get(int x, int y) const;

        /** Access anti-aliasing data.
            \return vector of X,Y pairs of anti-aliasing hints */
        const std::vector<uint16_t>& getAAData() const;

        /** Access pixel data.
            \return bitmap */
        const std::vector<uint8_t>& getData() const;

        /** Compute number of bytes required for a glyph of the specified size.
            \param width Width in pixels
            \param height Height in pixels
            \return required size in bytes */
        static size_t getBytesForSize(uint16_t width, uint16_t height);

     private:
        uint16_t m_width;                      /**< Width of character, in pixels. */
        uint16_t m_height;                     /**< Height of character, in pixels. */
        std::vector<uint8_t> m_data;           /**< Character data, in format for blitPattern(). */
        std::vector<uint16_t> m_aaData;        /**< Anti-aliasing "hints". Two elements per item, an X and Y element. */

        int getBytesPerLine() const;
    };

}

#endif
