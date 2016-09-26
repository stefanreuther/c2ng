/**
  *  \file ui/res/ccimageloader.hpp
  */
#ifndef C2NG_UI_RES_CCIMAGELOADER_HPP
#define C2NG_UI_RES_CCIMAGELOADER_HPP

#include "ui/res/imageloader.hpp"

namespace ui { namespace res {

    class CCImageLoader : public ImageLoader {
     public:
        CCImageLoader();

        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in);
    };

} }

#endif
