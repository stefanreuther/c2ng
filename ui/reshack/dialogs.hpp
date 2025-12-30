/**
  *  \file ui/reshack/dialogs.hpp
  *  \brief Class ui::reshack::Dialogs
  */
#ifndef C2NG_UI_RESHACK_DIALOGS_HPP
#define C2NG_UI_RESHACK_DIALOGS_HPP

#include "afl/base/optional.hpp"
#include "gfx/font.hpp"
#include "ui/reshack/info.hpp"
#include "ui/reshack/session.hpp"

namespace ui { namespace reshack {

    /** Resource editor dialogs. */
    class Dialogs {
     public:
        typedef afl::charset::Unichar_t Unichar_t;

        /** Show character coverage information.
            Shows all information as a list; if user chooses an entry refering to missing data,
            returns that entry's first missing character.
            @param session Session (for Root, Translator)
            @param data    Coverage data
            @return chosen character; Nothing if none chosen or dialog canceled */
        static afl::base::Optional<Unichar_t> showCoverage(Session& session, std::vector<Info::Coverage> data);

        /** Show preview of a font.
            Allows user to enter a sentence and show that, or pick from predefined sequences of characters.
            @param session Session (for Root, Translator, preview text)
            @param font    Font to preview */
        static void showPreview(Session& session, gfx::BitmapFont& font);

        /** Show character in system fonts.
            Allows user to choose a font.
            @param session Session (for Root, Translator)
            @param ch      Character to show
            @return chosen font; null if user canceled */
        static gfx::Font* showCharacterInSystemFonts(Session& session, Unichar_t ch);

        /** Change font alignment.
            Asks user for parameters.
            Creates a new font with these parameters applied.
            @param session Session (for Root, Translator)
            @param font    Font */
        static void changeFontAlignment(Session& session, gfx::BitmapFont& font);

        /** Synthesize a character.
            Tries to synthesize the character in all known ways; informs the user if that's not possible.
            @param session Session (for Root, Translator, CharacterNameList)
            @param font    Font
            @param ch      Character to synthesize
            @return synthesized character; null if not possible */
        static afl::base::Ptr<gfx::PalettizedPixmap> synthesizeCharacter(Session& session, const gfx::BitmapFont& font, Unichar_t ch);

        /** Change font encoding.
            Asks user for parameters.
            If user confirms, updates the font in-place.
            @param session Session (for Root, Translator)
            @param font    Font */
        static void changeFontEncoding(Session& session, gfx::BitmapFont& font);
    };

} }

#endif
