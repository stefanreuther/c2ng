/**
  *  \file ui/reshack/charactergrid.hpp
  *  \brief Character Grid Dialog
  */
#ifndef C2NG_UI_RESHACK_CHARACTERGRID_HPP
#define C2NG_UI_RESHACK_CHARACTERGRID_HPP

#include "afl/base/optional.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "gfx/bitmapfont.hpp"
#include "ui/reshack/session.hpp"
#include "ui/root.hpp"

namespace ui { namespace reshack {

    /** Character grid dialog.
        Displays characters from a font and allows selecting one.

        @param session Session (for Root, Translator, CharacterNameList)
        @param title   Dialog title
        @param font    Font to display
        @param current Current character (initial selection)
        @return selected character; Nothing if dialog was canceled */
    afl::base::Optional<afl::charset::Unichar_t> pickCharacterFromGrid(Session& session, String_t title, const gfx::BitmapFont& font, afl::charset::Unichar_t current);

} }

#endif
