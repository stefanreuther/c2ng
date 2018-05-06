/**
  *  \file gfx/gen/explosionrenderer.hpp
  */
#ifndef C2NG_GFX_GEN_EXPLOSIONRENDERER_HPP
#define C2NG_GFX_GEN_EXPLOSIONRENDERER_HPP

#include "gfx/gen/particlerenderer.hpp"

namespace gfx { namespace gen {

    class ExplosionRenderer {
     public:
        ExplosionRenderer(Point area, int size, int speed, util::RandomNumberGenerator& rng);
        ~ExplosionRenderer();

        afl::base::Ref<Canvas> renderFrame();
        afl::base::Ref<Canvas> renderAll();

        bool hasMoreFrames() const;

     private:
        ParticleRenderer m_renderer;
        Point m_area;
        int m_speed;
    };

} }

#endif
