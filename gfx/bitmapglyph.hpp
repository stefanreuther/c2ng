/**
  *  \file gfx/bitmapglyph.hpp
  */
#ifndef C2NG_GFX_BITMAPGLYPH_HPP
#define C2NG_GFX_BITMAPGLYPH_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "gfx/point.hpp"
#include "gfx/types.hpp"

namespace gfx {

    class Context;
    class Canvas;

    /** Glyph for a bitmap font.
        Contains the bitmap of the character, plus the list of anti-aliasing "hints". */
    class BitmapGlyph {
     public:
        BitmapGlyph();
        BitmapGlyph(uint16_t width, uint16_t height);
        BitmapGlyph(uint16_t width, uint16_t height, const uint8_t* data);
        ~BitmapGlyph();

        void addAAHint(uint16_t x, uint16_t y);
        int getHeight() const;
        int getWidth() const;
        void draw(Context& ctx, Point pt) const;
        void drawColored(Canvas& can, Point pt, Color_t pixel_color, Color_t aa_color) const;

        void set(int x, int y, bool value);
        bool get(int x, int y) const;

        const std::vector<uint16_t>& getAAData() const;
        const std::vector<uint8_t>& getData() const;

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
