/**
  *  \file ui/res/winplanbitmapprovider.hpp
  */
#ifndef C2NG_UI_RES_WINPLANBITMAPPROVIDER_HPP
#define C2NG_UI_RES_WINPLANBITMAPPROVIDER_HPP

#include "ui/res/provider.hpp"
#include "afl/base/ref.hpp"
#include "afl/io/directory.hpp"

namespace ui { namespace res {

    // /* Winplan Resource Provider

    //   This resource provider provides access to a Winplan "bmp" directory.
    //   It reads the ship pictures.

    //   Provides res::ship(n). */
    class WinplanBitmapProvider : public Provider {
     public:
        WinplanBitmapProvider(afl::base::Ref<afl::io::Directory> dir);

        virtual afl::base::Ptr<gfx::Canvas> loadImage(String_t name, Manager& mgr);

     private:
        const afl::base::Ref<afl::io::Directory> m_directory;
    };

} }

#endif
