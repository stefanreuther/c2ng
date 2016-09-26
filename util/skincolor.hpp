/**
  *  \file util/skincolor.hpp
  */
#ifndef C2NG_UTIL_SKINCOLOR_HPP
#define C2NG_UTIL_SKINCOLOR_HPP

#include "afl/base/types.hpp"

namespace util {

    /** Logical color names. */
    struct SkinColor {
        enum Color {
            Static,              ///< static text
            Green,               ///< green (variable)
            Yellow,              ///< yellow (variable)
            Red,                 ///< red (variable)
            White,               ///< white (headings)
            Contrast,            ///< some contrast against background (frame)
            Input,               ///< input lines (blue)
            Blue,                ///< blue for shaded list items
            Faded,               ///< gray for faded list items
            Heading,             ///< headings
            Selection,           ///< selection marker (yellow/green)
            InvStatic,           ///< opposite of static to build contrasts.
            Background,          ///< background color approximation
            Link,                ///< Link text color (on background).
            LinkShade,           ///< Link shade (background to tc_Link).
            LinkFocus            ///< Link focus (background to tc_Link).
        };
        static const size_t NUM_COLORS = LinkFocus+1;
    };

}

#endif
