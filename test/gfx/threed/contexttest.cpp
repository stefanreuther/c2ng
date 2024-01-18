/**
  *  \file test/gfx/threed/contexttest.cpp
  *  \brief Test for gfx::threed::Context
  */

#include "gfx/threed/context.hpp"

#include <stdexcept>
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("gfx.threed.Context")
{
    class Tester : public gfx::threed::Context {
     public:
        virtual void start(const gfx::Rectangle& /*r*/, gfx::Canvas& /*can*/)
            { }
        virtual void finish()
            { }
        virtual afl::base::Ref<gfx::threed::LineRenderer> createLineRenderer()
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<gfx::threed::TriangleRenderer> createTriangleRenderer()
            { throw std::runtime_error("no ref"); }
        virtual afl::base::Ref<gfx::threed::ParticleRenderer> createParticleRenderer()
            { throw std::runtime_error("no ref"); }
    };
    Tester t;
}
