/**
  *  \file gfx/canvas.cpp
  */

#include <algorithm>
#include "gfx/canvas.hpp"

namespace {
    gfx::ColorQuad_t stripAlpha(gfx::ColorQuad_t orig)
    {
        return COLORQUAD_FROM_RGBA(RED_FROM_COLORQUAD(orig),
                                   GREEN_FROM_COLORQUAD(orig),
                                   BLUE_FROM_COLORQUAD(orig),
                                   gfx::OPAQUE_ALPHA);
    }
}

void
gfx::Canvas::defaultBlit(Point pt, Canvas& src, Rectangle rect)
{
    const int width = rect.getWidth();
    const int height = rect.getHeight();

    pt += rect.getTopLeft();
    for (int yy = 0; yy < height; ++yy) {
        int xx = 0;
        while (xx < width) {
            // How much to do?
            const int MAX = 256;
            int now = std::min(width - xx, MAX);

            // Buffers
            Color_t colorBuf[MAX];
            ColorQuad_t quadBuf[MAX];

            // Set up descriptors
            afl::base::Memory<Color_t> colorDesc(colorBuf);
            afl::base::Memory<ColorQuad_t> quadDesc(quadBuf);
            colorDesc.trim(now);
            quadDesc.trim(now);

            // Read pixels
            src.getPixels(Point(rect.getLeftX() + xx, rect.getTopY() + yy), colorDesc);
            src.decodeColors(colorDesc, quadDesc);

            // Write pixels.
            // We're doing blit, that is, pixel alpha must be turned into operation alpha.
            int pos = 0;
            while (pos < now) {
                // First alpha of a run
                uint8_t opAlpha = ALPHA_FROM_COLORQUAD(quadBuf[pos]);
                quadBuf[pos] = stripAlpha(quadBuf[pos]);

                // Remaining run
                int pos2 = pos+1;
                while (pos2 < now && ALPHA_FROM_COLORQUAD(quadBuf[pos2]) == opAlpha) {
                    quadBuf[pos2] = stripAlpha(quadBuf[pos2]);
                    ++pos2;
                }

                // Blit
                if (opAlpha != TRANSPARENT_ALPHA) {
                    encodeColors(quadDesc.subrange(pos, pos2-pos), colorDesc.subrange(pos, pos2-pos));
                    drawPixels(Point(pt.getX() + xx + pos, pt.getY() + yy), colorDesc.subrange(pos, pos2-pos), opAlpha);
                }

                // Advance
                pos = pos2;
            }

            // Advance
            xx += now;
        }
    }
}
