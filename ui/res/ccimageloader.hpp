/**
  *  \file ui/res/ccimageloader.hpp
  */
#ifndef C2NG_UI_RES_CCIMAGELOADER_HPP
#define C2NG_UI_RES_CCIMAGELOADER_HPP

#include "ui/res/imageloader.hpp"

namespace ui { namespace res {

    /** ImageLoader implementation for PCC's custom image formats.
        Historical reasons. */
    class CCImageLoader : public ImageLoader {
     public:
        /** Constructor. */
        CCImageLoader();

        // ImageLoader
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in);
    };

} }

#endif
