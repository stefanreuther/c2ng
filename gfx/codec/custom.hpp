/**
  *  \file gfx/codec/custom.hpp
  *  \brief Class gfx::codec::Custom
  */
#ifndef C2NG_GFX_CODEC_CUSTOM_HPP
#define C2NG_GFX_CODEC_CUSTOM_HPP

#include "gfx/codec/codec.hpp"

namespace gfx { namespace codec {

    /** Custom image codecs.
        Historically, PCC1 used custom image formats.
        - 4 bit per pixel, ".cc"
        - 8 bit per pixel, ".cd"
        - 8 bit per pixel with transparency, ".gfx"
        They are used in cc.res, which PCC2 supports as extra source for image files.
        They are normally used with a compression layer (run-length encoding) on top.

        This class supports loading all three file formats with optional compression,
        and automatic format detection.

        This class supports writing the first two formats with optional compression;
        actual type is configured during construction. */
    class Custom : public Codec {
     public:
        /** File format for save(). */
        enum Mode {
            FourBit,            ///< 4 bit per pixel, ".cc"
            EightBit            ///< 8 bit per pixel, ".cd"
        };

        /** Default constructor.
            Use if you only want to use load(). */
        Custom();

        /** Constructor.
            @param mode        Mode for files written by save()
            @param compressed  Enable compression */
        Custom(Mode mode, bool compressed);

        // Codec:
        virtual void save(Canvas& can, afl::io::Stream& stream);
        virtual afl::base::Ref<Canvas> load(afl::io::Stream& stream);

     private:
        Mode m_mode;
        bool m_compressed;
    };

} }

inline
gfx::codec::Custom::Custom()
    : m_mode(EightBit), m_compressed(false)
{ }

inline
gfx::codec::Custom::Custom(Mode mode, bool compressed)
    : m_mode(mode), m_compressed(compressed)
{ }

#endif
