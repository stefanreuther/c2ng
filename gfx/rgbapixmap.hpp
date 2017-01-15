/**
  *  \file gfx/rgbapixmap.hpp
  */
#ifndef C2NG_GFX_RGBAPIXMAP_HPP
#define C2NG_GFX_RGBAPIXMAP_HPP

#include "afl/base/refcounted.hpp"
#include "afl/base/uncopyable.hpp"
#include "gfx/types.hpp"
#include "gfx/pixmap.hpp"
#include "afl/base/ref.hpp"
#include "gfx/canvas.hpp"

namespace gfx {

    class RGBAPixmap : public afl::base::RefCounted,
                       public Pixmap<ColorQuad_t>,
                       private afl::base::Uncopyable
    {
     public:
        static afl::base::Ref<RGBAPixmap> create(int w, int h);

        afl::base::Ref<Canvas> makeCanvas();

        void setAlpha(uint8_t alpha);

     private:
        inline RGBAPixmap(int w, int h);

        class CanvasImpl;
        friend class CanvasImpl;
        class TraitsImpl;
        friend class TraitsImpl;
    };

}

#endif
