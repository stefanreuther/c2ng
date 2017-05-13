/**
  *  \file ui/res/engineimageloader.hpp
  *  \brief Class ui::res::EngineImageLoader
  */
#ifndef C2NG_UI_RES_ENGINEIMAGELOADER_HPP
#define C2NG_UI_RES_ENGINEIMAGELOADER_HPP

#include "ui/res/imageloader.hpp"
#include "gfx/engine.hpp"

namespace ui { namespace res {

    /** ImageLoader implementation using an Engine's native image loader.
        This ImageLoader calls gfx::Engine::loadImage. */
    class EngineImageLoader : public ImageLoader {
     public:
        /** Constructor.
            \param engine Engine. Must live longer than the EngineImageLoader. */
        explicit EngineImageLoader(gfx::Engine& engine);

        // ImageLoader:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in);

     private:
        gfx::Engine& m_engine;
    };

} }

#endif
