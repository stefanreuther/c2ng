/**
  *  \file gfx/scan.cpp
  */

#include "gfx/scan.hpp"
#include "gfx/canvas.hpp"
#include "gfx/types.hpp"
#include "afl/base/inlinememory.hpp"

namespace {
    using gfx::ColorQuad_t;
    using gfx::Point;

    bool scanLine(gfx::Canvas& canvas, const int y, const int width, int& minX, int& maxX)
    {

        // Read from the left
        const int CHUNK = 128;
        int x = 0;
        while (x < width) {
            // Read block of pixels
            afl::base::InlineMemory<ColorQuad_t,CHUNK> pixels;
            pixels.trim(std::min(width - x, CHUNK));
            canvas.getPixels(Point(x, y), pixels);

            // Locate first non-transparent pixel
            while (const ColorQuad_t* p = pixels.eat()) {
                if (ALPHA_FROM_COLORQUAD(*p) != gfx::TRANSPARENT_ALPHA) {
                    goto out;
                }
                ++x;
            }
        }
     out:

        // Found anything at all?
        if (x >= width) {
            return false;
        }

        // Read from the right
        int endX = width;
        while (endX > x) {
            // Read block of pixels
            afl::base::InlineMemory<ColorQuad_t,CHUNK> pixels;
            int amount = std::min(endX - x, CHUNK);
            pixels.trim(amount);
            canvas.getPixels(Point(endX - amount, y), pixels);

            // Locate last non-transparent pixel by reading backwards
            size_t i = 0;
            while (const ColorQuad_t* p = pixels.atEnd(i)) {
                if (ALPHA_FROM_COLORQUAD(*p) != gfx::TRANSPARENT_ALPHA) {
                    goto out2;
                }
                ++i; --endX;
            }
        }
     out2:

        // Build result
        minX = x;
        maxX = endX;
        return true;
    }
}


bool
gfx::scanCanvas(Canvas& canvas, int& y, int& minX, int& maxX)
{
    Point size = canvas.getSize();
    while (y < size.getY()) {
        if (scanLine(canvas, y, size.getX(), minX, maxX)) {
            return true;
        }
        ++y;
    }
    return false;
}
