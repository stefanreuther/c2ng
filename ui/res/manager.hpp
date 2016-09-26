/**
  *  \file ui/res/manager.hpp
  */
#ifndef C2NG_UI_RES_MANAGER_HPP
#define C2NG_UI_RES_MANAGER_HPP

#include "afl/container/ptrvector.hpp"
#include "afl/io/stream.hpp"
#include "gfx/canvas.hpp"

namespace ui { namespace res {

    class ImageLoader;
    class Provider;

    class Manager {
     public:
        Manager();

        ~Manager();

        void addNewImageLoader(ImageLoader* p);

        void addNewProvider(Provider* p);

        afl::base::Ptr<gfx::Canvas> loadImage(String_t name);

        afl::base::Ptr<gfx::Canvas> loadImage(afl::io::Stream& s);

        gfx::Point getScreenSize() const;

        void setScreenSize(gfx::Point sz);

     private:
        afl::container::PtrVector<ImageLoader> m_imageLoaders;
        afl::container::PtrVector<Provider> m_providers;
        gfx::Point m_screenSize;
    };

} }

#endif
