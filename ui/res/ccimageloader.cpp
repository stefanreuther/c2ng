/**
  *  \file ui/res/ccimageloader.cpp
  *  \brief Class ui::res::EngineImageLoader
  */

#include "ui/res/ccimageloader.hpp"
#include "gfx/codec/custom.hpp"
#include "afl/except/fileformatexception.hpp"

// Constructor
ui::res::CCImageLoader::CCImageLoader()
{ }

// Load image.
afl::base::Ptr<gfx::Canvas>
ui::res::CCImageLoader::loadImage(afl::io::Stream& in)
{
    try {
        return gfx::codec::Custom().load(in).asPtr();
    }
    catch (afl::except::FileProblemException&) {
        return 0;
    }
}
