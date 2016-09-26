/**
  *  \file gfx/palettizedpixmap.hpp
  */
#ifndef C2NG_GFX_PALETTIZEDPIXMAP_HPP
#define C2NG_GFX_PALETTIZEDPIXMAP_HPP

#include "gfx/pixmap.hpp"
#include "afl/base/types.hpp"
#include "gfx/types.hpp"
#include "afl/base/ptr.hpp"
#include "canvas.hpp"
#include "afl/base/refcounted.hpp"
#include "afl/base/uncopyable.hpp"

namespace gfx {

    class PalettizedPixmap : public afl::base::RefCounted,
                             public Pixmap<uint8_t>,
                             private afl::base::Uncopyable
    {
     public:
        static afl::base::Ptr<PalettizedPixmap> create(int w, int h);

        void setPalette(uint8_t start, afl::base::Memory<const ColorQuad_t> colorDefinitions);

        void setPalette(uint8_t slot, ColorQuad_t colorDefinition);

        void getPalette(uint8_t start, afl::base::Memory<ColorQuad_t> colorDefinitions) const;

        uint8_t findNearestColor(ColorQuad_t def) const;

        afl::base::Ptr<Canvas> makeCanvas();

     private:
        inline PalettizedPixmap(int w, int h);

        class CanvasImpl;
        friend class CanvasImpl;
        class TraitsImpl;
        friend class TraitsImpl;

        ColorQuad_t m_palette[256];
    };

}

#endif
