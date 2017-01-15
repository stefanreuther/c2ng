/**
  *  \file gfx/defaultfont.hpp
  *  \brief Default Font
  */
#ifndef C2NG_GFX_DEFAULTFONT_HPP
#define C2NG_GFX_DEFAULTFONT_HPP

#include "afl/base/ref.hpp"

namespace gfx {

    class Font;

    /** Create a default font.
        The idea of this default font is to support display of a last-resort error message
        or a "please wait while I load the fonts" message, it does not fulfil any higher aesthetic demands.
        The default font supports the ASCII repertoire.

        Note that this creates a new instance on every call.

        \return newly-allocated font */
    afl::base::Ref<Font> createDefaultFont();

}

#endif
