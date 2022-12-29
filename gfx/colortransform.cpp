/**
  *  \file gfx/colortransform.cpp
  *  \brief Color Transformation
  */

#include "gfx/colortransform.hpp"
#include "gfx/rgbapixmap.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "afl/base/staticassert.hpp"

namespace {
    void convertColorsToMonochrome(afl::base::Memory<gfx::ColorQuad_t> colors, gfx::ColorQuad_t color)
    {
        while (gfx::ColorQuad_t* p = colors.eat()) {
            uint8_t r = RED_FROM_COLORQUAD(*p);
            uint8_t g = GREEN_FROM_COLORQUAD(*p);
            uint8_t b = BLUE_FROM_COLORQUAD(*p);
            uint8_t a = ALPHA_FROM_COLORQUAD(*p);

            int intensity = (r + g + b);

            uint8_t nr = static_cast<uint8_t>(intensity * RED_FROM_COLORQUAD  (color) / 765);
            uint8_t ng = static_cast<uint8_t>(intensity * GREEN_FROM_COLORQUAD(color) / 765);
            uint8_t nb = static_cast<uint8_t>(intensity * BLUE_FROM_COLORQUAD (color) / 765);

            *p = COLORQUAD_FROM_RGBA(nr, ng, nb, a);
        }
    }
}


afl::base::Ref<gfx::Canvas>
gfx::convertToMonochrome(Canvas& in, ColorQuad_t color)
{
    const Point size = in.getSize();
    if (in.getBitsPerPixel() <= 8) {
        // Palettized
        afl::base::Ref<Canvas> result = PalettizedPixmap::create(size.getX(), size.getY())->makeCanvas();

        // Copy pixels as-is
        const int N = 1024;
        Color_t buffer[N];
        for (int y = 0; y < size.getY(); ++y) {
            for (int x = 0; x < size.getX(); x += N) {
                // Read
                afl::base::Memory<Color_t> mem(buffer);
                mem.trim(size.getX() - x);
                in.getPixels(Point(x, y), buffer);

                // Write
                result->drawPixels(Point(x, y), buffer, OPAQUE_ALPHA);
            }
        }

        // Copy transformed palette
        const int NPAL = 256;
        static_assert(NPAL <= N, "NPAL");

        // - prepare buffer of palette indexes
        for (int i = 0; i < NPAL; ++i) {
            buffer[i] = i;
        }
        afl::base::Memory<Color_t> indexMem(buffer);
        indexMem.trim(NPAL);

        // - read palette
        ColorQuad_t palette[NPAL];
        in.decodeColors(indexMem, palette);

        // - transform and write back
        convertColorsToMonochrome(palette, color);
        result->setPalette(0, palette, indexMem);

        return result;
    } else {
        // Truecolor
        afl::base::Ref<Canvas> result = RGBAPixmap::create(size.getX(), size.getY())->makeCanvas();

        // Copy and transform pixels
        const int N = 1024;
        Color_t pixels[N];
        ColorQuad_t quads[N];
        for (int y = 0; y < size.getY(); ++y) {
            for (int x = 0; x < size.getX(); x += N) {
                // Read
                afl::base::Memory<Color_t> pixelMem(pixels);
                afl::base::Memory<ColorQuad_t> quadMem(quads);
                pixelMem.trim(size.getX() - x);
                quadMem.trim(size.getX() - x);

                in.getPixels(Point(x, y), pixelMem);
                in.decodeColors(pixelMem, quadMem);

                // Transform and write back
                convertColorsToMonochrome(quadMem, color);

                result->encodeColors(quadMem, pixelMem);
                result->drawPixels(Point(x, y), pixelMem, OPAQUE_ALPHA);
            }
        }

        return result;
    }
}
