/**
  *  \file gfx/windowparameters.cpp
  *  \brief Structure gfx::WindowParameters
  */

#include "gfx/windowparameters.hpp"
#include "afl/except/commandlineexception.hpp"

bool
gfx::handleWindowParameterOption(WindowParameters& param, const String_t& option, afl::sys::CommandLineParser& parser, afl::string::Translator& tx)
{
    // ex gfx/init.cc:options
    if (option == "fullscreen") {
        param.fullScreen = true;
        return true;
    } else if (option == "windowed") {
        param.fullScreen = false;
        return true;
    } else if (option == "nomousegrab") {
        param.disableGrab = true;
        return true;
    } else if (option == "bpp") {
        // ex gfx/init.cc:optSetBpp
        util::StringParser sp(parser.getRequiredParameter(option));
        int bpp = 0;
        if (!sp.parseInt(bpp) || !sp.parseEnd()) {
            throw afl::except::CommandLineException(tx("Invalid parameter to \"-bpp\""));
        }
        if (bpp != 8 && bpp != 16 && bpp != 32) {
            throw afl::except::CommandLineException(tx("Parameter to \"-bpp\" must be 8, 16 or 32"));
        }
        param.bitsPerPixel = bpp;
        return true;
    } else if (option == "size") {
        // ex gfx/init.cc:optSetSize
        util::StringParser sp(parser.getRequiredParameter(option));
        int w = 0, h = 0;
        if (!sp.parseInt(w)) {
            throw afl::except::CommandLineException(tx("Invalid parameter to \"-size\""));
        }
        if (sp.parseCharacter('X') || sp.parseCharacter('x') || sp.parseCharacter('*')) {
            if (!sp.parseInt(h)) {
                throw afl::except::CommandLineException(tx("Invalid parameter to \"-size\""));
            }
        } else {
            h = 3*w/4;
        }
        if (!sp.parseEnd()) {
            throw afl::except::CommandLineException(tx("Invalid parameter to \"-size\""));
        }
        if (w < MIN_WIDTH || h < MIN_HEIGHT || w > MAX_DIM || h > MAX_DIM) {
            throw afl::except::CommandLineException(tx("Parameter to \"-size\" is out of range"));
        }
        param.size = gfx::Point(w, h);
        return true;
    } else {
        return false;
    }
}

String_t
gfx::getWindowParameterHelp(afl::string::Translator& tx)
{
    return tx("-fullscreen"     "\tRun fullscreen\n"
              "-windowed"       "\tRun in a window\n"
              "-bpp=N"          "\tUse color depth of N bits per pixel\n"
              "-size=W[xH]"     "\tUse resolution of WxH pixels\n"
              "-nomousegrab"    "\tDon't grab (lock into window) mouse pointer\n");
}
