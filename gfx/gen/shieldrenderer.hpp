/**
  *  \file gfx/gen/shieldrenderer.hpp
  */
#ifndef C2NG_GFX_GEN_SHIELDRENDERER_HPP
#define C2NG_GFX_GEN_SHIELDRENDERER_HPP

#include "gfx/gen/particlerenderer.hpp"

namespace gfx { namespace gen {

    class ShieldRenderer {
     public:
        ShieldRenderer(Point area, int angle, int size, util::RandomNumberGenerator& rng);
        ~ShieldRenderer();

        afl::base::Ref<Canvas> renderFrame();
        afl::base::Ref<Canvas> renderAll();

        bool hasMoreFrames() const;

     private:
        ParticleRenderer m_renderer;
        Point m_area;
    };

} }

#endif
