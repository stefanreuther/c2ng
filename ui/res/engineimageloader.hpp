/**
  *  \file ui/res/engineimageloader.hpp
  */
#ifndef C2NG_UI_RES_ENGINEIMAGELOADER_HPP
#define C2NG_UI_RES_ENGINEIMAGELOADER_HPP

#include "ui/res/imageloader.hpp"
#include "gfx/engine.hpp"

namespace ui { namespace res {

    class EngineImageLoader : public ImageLoader {
     public:
        EngineImageLoader(gfx::Engine& engine);

        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in);

     private:
        gfx::Engine& m_engine;
    };

} }

#endif
