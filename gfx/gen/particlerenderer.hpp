/**
  *  \file gfx/gen/particlerenderer.hpp
  */
#ifndef C2NG_GFX_GEN_PARTICLERENDERER_HPP
#define C2NG_GFX_GEN_PARTICLERENDERER_HPP

#include <vector>
#include "afl/base/types.hpp"
#include "gfx/palettizedpixmap.hpp"
#include "gfx/point.hpp"
#include "util/randomnumbergenerator.hpp"

namespace gfx { namespace gen {

    class ParticleRenderer {
     public:
        static const size_t NUM_COLORS = 64;

        ParticleRenderer();
        ~ParticleRenderer();

        void addParticles(size_t count, Point pos, Point fractionalSpeed, Point fractionalSpeedDelta, util::RandomNumberGenerator& rng);
        void render(PalettizedPixmap& pix);
        void advanceTime(int time);
        bool hasMoreFrames() const;
        int getNumRemainingFrames(int timePerFrame) const;

     private:
        struct Particle {
            int32_t x, y;
            int32_t dx, dy;
        };
        std::vector<Particle> m_particles;
        int m_time;
    };

} }

#endif
