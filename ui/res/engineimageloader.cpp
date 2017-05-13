/**
  *  \file ui/res/engineimageloader.cpp
  *  \brief Class ui::res::EngineImageLoader
  */

#include "ui/res/engineimageloader.hpp"

// Constructor.
ui::res::EngineImageLoader::EngineImageLoader(gfx::Engine& engine)
    : m_engine(engine)
{ }

// Load image.
afl::base::Ptr<gfx::Canvas>
ui::res::EngineImageLoader::loadImage(afl::io::Stream& in)
{
    try {
        in.setPos(0);
        return m_engine.loadImage(in).asPtr();
    }
    catch (...) {
        return 0;
    }
}
