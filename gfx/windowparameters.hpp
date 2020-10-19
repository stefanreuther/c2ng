/**
  *  \file gfx/windowparameters.hpp
  *  \brief Structure gfx::WindowParameters
  */
#ifndef C2NG_GFX_WINDOWPARAMETERS_HPP
#define C2NG_GFX_WINDOWPARAMETERS_HPP

#include "afl/base/ptr.hpp"
#include "afl/string/string.hpp"
#include "gfx/canvas.hpp"
#include "gfx/point.hpp"

namespace gfx {

    /** Parameters for a graphics window.
        This is a structure and you can manipulate it as you need. */
    struct WindowParameters {
        /** Window size in pixels. */
        Point size;

        /** Color depth (bits per pixel). */
        int bitsPerPixel;

        /** true to make a fullscreen window. */
        bool fullScreen;

        /** true to disable mouse-grab. */
        bool disableGrab;

        /** Window title (application name). */
        String_t title;

        /** Window icon. */
        afl::base::Ptr<Canvas> icon;

        WindowParameters()
            : size(640, 480),
              bitsPerPixel(32),
              fullScreen(false),
              disableGrab(false),
              title(),
              icon()
            { }
    };

}

#endif
