/**
  *  \file gfx/codec/codec.hpp
  *  \brief Interface gfx::codec::Codec
  */
#ifndef C2NG_GFX_CODEC_CODEC_HPP
#define C2NG_GFX_CODEC_CODEC_HPP

#include "afl/io/stream.hpp"
#include "gfx/canvas.hpp"

namespace gfx { namespace codec {

    /** Image file codec.
        Allows loading and saving images. */
    class Codec {
     public:
        /** Virtual destructor. */
        virtual ~Codec()
            { }

        /** Save image.
            @param can     Canvas to save
            @param stream  Stream to save to */
        virtual void save(Canvas& can, afl::io::Stream& stream) = 0;

        /** Load image.
            @param stream  Stream to load from
            @return newly-allocated canvas containing image
            @throw afl::except::FileFormatException on error */
        virtual afl::base::Ref<Canvas> load(afl::io::Stream& stream) = 0;
    };

} }

#endif
