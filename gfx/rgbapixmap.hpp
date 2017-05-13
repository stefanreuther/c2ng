/**
  *  \file gfx/rgbapixmap.hpp
  *  \brief Class gfx::RGBAPixmap
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

    /** RGBA pixmap.
        Implements a 32-bit-per-pixel truecolor pixmap.
        Pixels are represented as ColorQuad_t in native endianness.

        RGBAPixmap implements the pixmap interface that allows you to access the pixels and the palette directly.
        Use makeCanvas() to obtain a Canvas you can use with normal drawing functions.

        RGBAPixmap is always allocated on the heap. */
    class RGBAPixmap : public afl::base::RefCounted,
                       public Pixmap<ColorQuad_t>,
                       private afl::base::Uncopyable
    {
     public:
        /** Create a RGBAPixmap.
            \param w Width in pixels
            \param h Height in pixels
            \return newly-allocated RGBAPixmap */
        static afl::base::Ref<RGBAPixmap> create(int w, int h);

        /** Create canvas to draw on.
            \return newly-allocated canvas */
        afl::base::Ref<Canvas> makeCanvas();

        /** Set pixmap alpha.
            Pixels have individual alpha channels that are used when this pixmap is blitted anywhere.
            This function sets the alpha channel for the whole pixmap (by modifying all pixels) to a single value for the whole pixmap.
            \param alpha New alpha */
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
