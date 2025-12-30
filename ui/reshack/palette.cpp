/**
  *  \file ui/reshack/palette.cpp
  *  \brief Class ui::reshack::Palette
  */

#include "ui/reshack/palette.hpp"

#include <algorithm>
#include "gfx/codec/custom.hpp"
#include "gfx/colorquantizer.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "ui/colorscheme.hpp"   // Color_Avail

namespace {
    bool hasStandardPalette(gfx::Canvas& in)
    {
        // ex identifyColorMode(Ptr<GfxPixmap> pix) - sort-of
        if (in.getBitsPerPixel() != 8) {
            return false;
        } else {
            /* palettized */
            bool same = true;
            afl::base::Memory<const gfx::ColorQuad_t> palette = gfx::codec::Custom::getPalette();
            gfx::Color_t i = 0;
            while (const gfx::ColorQuad_t* p = palette.eat()) {
                gfx::ColorQuad_t quad = 0;
                in.decodeColors(afl::base::Memory<const gfx::Color_t>::fromSingleObject(i), afl::base::Memory<gfx::ColorQuad_t>::fromSingleObject(quad));
                if (quad != *p) {
                    same = false;
                    break;
                }
                ++i;
            }
            return same;
        }
    }
}

afl::base::Ref<gfx::PalettizedPixmap>
ui::reshack::Palette::makeEditable(afl::base::Ref<gfx::Canvas> pix)
{
    // ex makeEditable(Ptr<GfxPixmap> pix) - totally revised
    if (hasStandardPalette(*pix)) {
        // Create output
        gfx::Point size = pix->getSize();
        afl::base::Ref<gfx::PalettizedPixmap> result = gfx::PalettizedPixmap::create(size.getX(), size.getY());
        copyPalette(*result, *pix);

        // Copy pixels
        afl::base::GrowableMemory<gfx::Color_t> pixels;
        pixels.resize(size.getX());
        for (int y = 0; y < size.getY(); ++y) {
            pix->getPixels(gfx::Point(0, y), pixels);

            afl::base::Memory<gfx::Color_t> in = pixels;
            afl::base::Memory<uint8_t> out = result->row(y);
            while (!in.empty() && !out.empty()) {
                *out.eat() = static_cast<uint8_t>(*in.eat());
            }
        }

        return result;
    } else {
        return gfx::ColorQuantizer()
            .setPalette(0, afl::base::Memory<const gfx::ColorQuad_t>(gfx::codec::Custom::getPalette()))
            .setUsablePaletteRange(0, 256)
            .setDynamicPaletteRange(192, 64)
            .quantize(*pix);
    }
}

void
ui::reshack::Palette::copyPalette(gfx::PalettizedPixmap& out, gfx::Canvas& in)
{
    for (size_t i = 0; i < 256; ++i) {
        gfx::Color_t index = static_cast<gfx::Color_t>(i);
        gfx::ColorQuad_t def = 0;
        in.decodeColors(afl::base::Memory<gfx::Color_t>::fromSingleObject(index), afl::base::Memory<gfx::ColorQuad_t>::fromSingleObject(def));
        def = COLORQUAD_FROM_RGB(RED_FROM_COLORQUAD(def), GREEN_FROM_COLORQUAD(def), BLUE_FROM_COLORQUAD(def)); // strip alpha
        out.setPalette(static_cast<uint8_t>(i), def);
    }
}

bool
ui::reshack::Palette::isEditableColor(ColorMode cm, uint8_t color)
{
    // ex isEditableColor(ColorMode cm, uint8_t color)
    switch (cm) {
     case StandardPaletteColor:
        return color >= Color_Avail;

     case GrayscaleColor:
        return false;
    }
    return false;
}
