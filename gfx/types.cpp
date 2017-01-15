/**
  *  \file gfx/types.cpp
  */

#include <algorithm>
#include "gfx/types.hpp"

namespace {
    uint32_t mixMask(uint32_t a, uint32_t b, uint32_t mask, gfx::Alpha_t alpha)
    {
        a &= mask;
        b &= mask;
        return (a + ((b - a) * alpha / 255)) & mask;
    }
}


gfx::ColorQuad_t
gfx::mixColor(ColorQuad_t a, ColorQuad_t b, Alpha_t alpha)
{
#if 0
    uint8_t rresult = mixColorComponent(RED_FROM_COLORQUAD(a),   RED_FROM_COLORQUAD(b),   alpha);
    uint8_t gresult = mixColorComponent(GREEN_FROM_COLORQUAD(a), GREEN_FROM_COLORQUAD(b), alpha);
    uint8_t bresult = mixColorComponent(BLUE_FROM_COLORQUAD(a),  BLUE_FROM_COLORQUAD(b),  alpha);
    uint8_t aresult = mixColorComponent(ALPHA_FROM_COLORQUAD(a), ALPHA_FROM_COLORQUAD(b), alpha);
    return COLORQUAD_FROM_RGBA(rresult, gresult, bresult, aresult);
#else
    // Special case to avoid overflow
    uint32_t f1 = mixColorComponent(a >> 24, b >> 24, alpha) << 24;

    // Regular formula for others
    uint32_t f2 = mixMask(a, b, 0x00FF0000U, alpha);
    uint32_t f3 = mixMask(a, b, 0x0000FF00U, alpha);
    uint32_t f4 = mixMask(a, b, 0x000000FFU, alpha);
    return f1 + f2 + f3 + f4;
#endif
}

gfx::ColorQuad_t
gfx::addColor(ColorQuad_t a, ColorQuad_t b)
{
#if 0
    uint8_t rresult = uint8_t(std::min(255U, RED_FROM_COLORQUAD  (a) + RED_FROM_COLORQUAD  (b)));
    uint8_t gresult = uint8_t(std::min(255U, GREEN_FROM_COLORQUAD(a) + GREEN_FROM_COLORQUAD(b)));
    uint8_t bresult = uint8_t(std::min(255U, BLUE_FROM_COLORQUAD (a) + BLUE_FROM_COLORQUAD (b)));
    uint8_t aresult = uint8_t(std::min(255U, ALPHA_FROM_COLORQUAD(a) + ALPHA_FROM_COLORQUAD(b)));
    return COLORQUAD_FROM_RGBA(rresult, gresult, bresult, aresult);
#else
    // Overflow detection for MSB component!
    uint32_t f1 = (a & 0xFF000000U) + (b & 0xFF000000U);
    if (f1 < a) {
        f1 = 0xFF000000U;
    }

    // Others are simpler
    uint32_t f2 = std::min(0x00FF0000U, (a & 0x00FF0000U) + (b & 0x00FF0000U));
    uint32_t f3 = std::min(0x0000FF00U, (a & 0x0000FF00U) + (b & 0x0000FF00U));
    uint32_t f4 = std::min(0x000000FFU, (a & 0x000000FFU) + (b & 0x000000FFU));
    return f1 + f2 + f3 + f4;
#endif
}

int32_t
gfx::getColorDistance(gfx::ColorQuad_t x, gfx::ColorQuad_t y)
{
    int32_t dr = RED_FROM_COLORQUAD(x)   - RED_FROM_COLORQUAD(y);
    int32_t dg = GREEN_FROM_COLORQUAD(x) - GREEN_FROM_COLORQUAD(y);
    int32_t db = BLUE_FROM_COLORQUAD(x)  - BLUE_FROM_COLORQUAD(y);
    int32_t da = ALPHA_FROM_COLORQUAD(x) - ALPHA_FROM_COLORQUAD(y);

    // This distance metric is a little adventurous....
    // The idea is to require an exact match on alpha, so we do not choose a matching transparent color when looking for an opaque one.
    if (da != 0) {
        return 0x40000;
    } else {
        return dr*dr + dg*dg + db*db;
    }
}
