/**
  *  \file client/vcr/beamsprite.hpp
  */
#ifndef C2NG_CLIENT_VCR_BEAMSPRITE_HPP
#define C2NG_CLIENT_VCR_BEAMSPRITE_HPP

#include "ui/colorscheme.hpp"
#include "gfx/point.hpp"
#include "gfx/anim/sprite.hpp"

namespace client { namespace vcr {

    class BeamSprite : public gfx::anim::Sprite {
     public:
        static const int LIMIT = 6;

        BeamSprite(ui::ColorScheme& cs, gfx::Point a, gfx::Point b);
        ~BeamSprite();

        virtual void draw(gfx::Canvas& can);
        virtual void tick();

     private:
        int m_state;
        ui::ColorScheme& m_colors;
        gfx::Point m_a;
        gfx::Point m_b;
    };

} }

#endif
