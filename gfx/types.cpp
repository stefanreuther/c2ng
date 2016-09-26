/**
  *  \file gfx/types.cpp
  */

#include "gfx/types.hpp"

int32_t
gfx::getColorDistance(gfx::ColorQuad_t x, gfx::ColorQuad_t y)
{
    int32_t dr = RED_FROM_COLORQUAD(x)   - RED_FROM_COLORQUAD(y);
    int32_t dg = GREEN_FROM_COLORQUAD(x) - GREEN_FROM_COLORQUAD(y);
    int32_t db = BLUE_FROM_COLORQUAD(x)  - BLUE_FROM_COLORQUAD(y);
    int32_t da = ALPHA_FROM_COLORQUAD(x) - ALPHA_FROM_COLORQUAD(y);

    // This distance metric is a little adventurous....
    if (da != 0) {
        return 0x40000;
    } else {
        return dr*dr + dg*dg + db*db;
    }
}
