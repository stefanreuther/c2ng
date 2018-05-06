/**
  *  \file client/vcr/torpedosprite.hpp
  */
#ifndef C2NG_CLIENT_VCR_TORPEDOSPRITE_HPP
#define C2NG_CLIENT_VCR_TORPEDOSPRITE_HPP

#include "ui/colorscheme.hpp"
#include "gfx/anim/sprite.hpp"
#include "gfx/point.hpp"

namespace client { namespace vcr {

    class TorpedoSprite : public gfx::anim::Sprite {
     public:
        TorpedoSprite(ui::ColorScheme& cs, gfx::Point a, gfx::Point b, int time);
        ~TorpedoSprite();

        virtual void draw(gfx::Canvas& can);
        virtual void tick();

     private:
        ui::ColorScheme& m_colors;
        gfx::Point m_a;
        gfx::Point m_b;
        int m_time;
        int m_state;
    };

} }

#endif
