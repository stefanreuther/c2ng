/**
  *  \file ui/reshack/info.hpp
  *  \brief Class ui::reshack::Info
  */
#ifndef C2NG_UI_RESHACK_INFO_HPP
#define C2NG_UI_RESHACK_INFO_HPP

#include <vector>
#include "afl/charset/unicode.hpp"
#include "afl/string/string.hpp"
#include "afl/string/translator.hpp"
#include "gfx/bitmapfont.hpp"

namespace ui { namespace reshack {

    /** Functions to retrieve information about fonts/encoding. */
    class Info {
     public:
        typedef afl::charset::Unichar_t Unichar_t;

        /** Font coverage information for a character set. */
        struct Coverage {
            /** Name of character set. */
            String_t charsetName;

            /** Number of missing characters. */
            size_t numMissingCharacters;

            /** First missing character.
                Set if numMissingCharacters is not 0. */
            Unichar_t firstMissingCharacter;

            /** Constructor.
                @param charsetName Name
                @param numMissingCharacters Number of missing character
                @param firstMissingCharacter First missing character if numMissingCharacters is not 0 */
            Coverage(const String_t& charsetName, size_t numMissingCharacters, Unichar_t firstMissingCharacter)
                : charsetName(charsetName), numMissingCharacters(numMissingCharacters), firstMissingCharacter(firstMissingCharacter)
                { }
        };

        /** Get information about character set coverage.
            @param font Font to analyze
            @param tx   Translator (for util::CharsetFactory::getName())
            @return coverage information */
        static std::vector<Coverage> getFontCoverage(const gfx::BitmapFont& font, afl::string::Translator& tx);

        /** Get encoding information for a character.
            @param ch Character
            @param tx Translator
            @return Human-readable multi-line string */
        static String_t getEncodingInfo(Unichar_t ch, afl::string::Translator& tx);
    };

} }

#endif
