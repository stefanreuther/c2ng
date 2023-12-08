/**
  *  \file gfx/windowparameters.hpp
  *  \brief Structure gfx::WindowParameters
  */
#ifndef C2NG_GFX_WINDOWPARAMETERS_HPP
#define C2NG_GFX_WINDOWPARAMETERS_HPP

#include "afl/base/ptr.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "afl/sys/commandlineparser.hpp"
#include "gfx/canvas.hpp"
#include "gfx/point.hpp"

namespace gfx {

    static const int MIN_WIDTH = 640;   ///< Minimum window width. ex GFX_MIN_WIDTH
    static const int MIN_HEIGHT = 480;  ///< Minimum window height. ex GFX_MIN_HEIGHT
    static const int MAX_DIM = 10000;   ///< Maximum accepted window width/height.

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

    /** Handle a window parameter option.
        @param param  [in,out] WindowParameters structure
        @param option [in]     Option as returned by CommandLineParser
        @param parser [in]     CommandLineParser instance, to retrieve parameters
        @param tx     [in]     Translator (for error messages)
        @return true if option was accepted
        @throw afl::except::CommandLineException on invalid parameters */
    bool handleWindowParameterOption(WindowParameters& param, const String_t& option, afl::sys::CommandLineParser& parser, afl::string::Translator& tx);

    /** Get help text for handleWindowParameterOption().
        @param tx Translator
        @return Help text, suitable as input to util::formatOptions(). */
    String_t getWindowParameterHelp(afl::string::Translator& tx);

}

#endif
