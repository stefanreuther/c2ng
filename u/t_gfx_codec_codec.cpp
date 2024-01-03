/**
  *  \file u/t_gfx_codec_codec.cpp
  *  \brief Test for gfx::codec::Codec
  */

#include "gfx/codec/codec.hpp"

#include "t_gfx_codec.hpp"

/** Interface test. */
void
TestGfxCodecCodec::testInterface()
{
    class Tester : public gfx::codec::Codec {
     public:
        virtual void save(gfx::Canvas& /*can*/, afl::io::Stream& /*stream*/)
            { }
        virtual afl::base::Ref<gfx::Canvas> load(afl::io::Stream& /*stream*/)
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}

