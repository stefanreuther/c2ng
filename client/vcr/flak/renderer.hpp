/**
  *  \file client/vcr/flak/renderer.hpp
  */
#ifndef C2NG_CLIENT_VCR_FLAK_RENDERER_HPP
#define C2NG_CLIENT_VCR_FLAK_RENDERER_HPP

#include "afl/base/deletable.hpp"
#include "gfx/canvas.hpp"
#include "gfx/rectangle.hpp"

namespace client { namespace vcr { namespace flak {

    class Renderer : public afl::base::Deletable {
     public:
        virtual void init() = 0;
        virtual void draw(gfx::Canvas& can, const gfx::Rectangle& area, bool grid) = 0;
    };

} } }

#endif
