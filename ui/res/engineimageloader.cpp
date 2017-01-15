/**
  *  \file ui/res/engineimageloader.cpp
  */

#include "ui/res/engineimageloader.hpp"

ui::res::EngineImageLoader::EngineImageLoader(gfx::Engine& engine)
    : m_engine(engine)
{ }

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
