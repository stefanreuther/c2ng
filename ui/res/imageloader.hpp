/**
  *  \file ui/res/imageloader.hpp
  */
#ifndef C2NG_UI_RES_IMAGELOADER_HPP
#define C2NG_UI_RES_IMAGELOADER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/canvas.hpp"
#include "afl/io/stream.hpp"

namespace ui { namespace res {

    class ImageLoader : public afl::base::Deletable {
     public:
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in) = 0;
    };

} }

#endif
