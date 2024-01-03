/**
  *  \file gfx/codec/bmp.hpp
  *  \brief Class gfx::codec::BMP
  */
#ifndef C2NG_GFX_CODEC_BMP_HPP
#define C2NG_GFX_CODEC_BMP_HPP

#include "gfx/codec/codec.hpp"

namespace gfx { namespace codec {

    /** BMP file codec.
        For loading, supports only basic 8-bit and 24-bit formats, producing PalettizedPixmap and RGBAPixmap, respectively.
        For saving, always 24-bit format.

        Integration check as of 20240102:
        - can read all files from a standard Winplan installation except for vpl019 (compression enabled) and vpl023 (4 bpp)
        - can read files it wrote
        - can read files written by ImageMagick
        - written file correctly processed without warnings by ImageMagick */
    class BMP : public Codec {
     public:
        virtual void save(Canvas& can, afl::io::Stream& stream);
        virtual afl::base::Ref<Canvas> load(afl::io::Stream& stream);
    };

} }

#endif
