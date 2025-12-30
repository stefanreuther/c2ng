/**
  *  \file ui/reshack/win32import.hpp
  *  \brief Class ui::reshack::Win32Import
  */
#ifndef C2NG_UI_RESHACK_WIN32IMPORT_HPP
#define C2NG_UI_RESHACK_WIN32IMPORT_HPP

#include "gfx/bitmapfont.hpp"
#include "ui/reshack/session.hpp"

namespace ui { namespace reshack {

    /** Win32 Font Import.
        When Win32 API is available, we can use its font renderer to create a font. */
    class Win32Import {
     public:
        /** Check whether feature is available.
            @return true if Win32 font import is available */
        static bool isSupported();

        /** Import a font.
            Asks the user for a font using the system dialog, and creates a BitmapFont instance matching it.
            @param session Session (for Root, Translator)
            @return font on success, null on error or if function not available */
        static afl::base::Ptr<gfx::BitmapFont> importFont(Session& session);
    };

} }

#endif
