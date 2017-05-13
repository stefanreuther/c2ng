/**
  *  \file ui/res/imageloader.hpp
  *  \brief Interface ui::res::ImageLoader
  */
#ifndef C2NG_UI_RES_IMAGELOADER_HPP
#define C2NG_UI_RES_IMAGELOADER_HPP

#include "afl/base/deletable.hpp"
#include "afl/base/ptr.hpp"
#include "gfx/canvas.hpp"
#include "afl/io/stream.hpp"

namespace ui { namespace res {

    /** Image Loader base class.
        An ImageLoader provides support for a particular image format or decoder library.
        It transforms a recognized stream into a gfx::Canvas. */
    class ImageLoader : public afl::base::Deletable {
     public:
        /** Load image.
            This function must load the image from the stream if it has a recognized format.
            If the function does not recognize the file type, it must return null.
            On other errors (out of memory, type recognized but content invalid, I/O error),
            it can throw an exception or return 0.

            The image must be newly allocated because the caller may want to modify it.

            \param in Stream
            \return Newly-allocated image; 0 if file type not recognized */            
        virtual afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& in) = 0;
    };

} }

#endif
