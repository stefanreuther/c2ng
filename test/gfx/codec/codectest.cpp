/**
  *  \file test/gfx/codec/codectest.cpp
  *  \brief Test for gfx::codec::Codec
  */

#include "gfx/codec/codec.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.codec.Codec")
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
