/**
  *  \file u/t_gfx_threed_context.cpp
  *  \brief Test for gfx::threed::Context
  */

#include "gfx/threed/context.hpp"

#include "t_gfx_threed.hpp"

/** Interface test. */
void
TestGfxThreedContext::testInterface()
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

