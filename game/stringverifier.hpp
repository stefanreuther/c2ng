/**
  *  \file game/stringverifier.hpp
  */
#ifndef C2NG_GAME_STRINGVERIFIER_HPP
#define C2NG_GAME_STRINGVERIFIER_HPP

#include "afl/base/deletable.hpp"
#include "afl/string/string.hpp"
#include "afl/charset/unicode.hpp"
#include "afl/base/clonable.hpp"

namespace game {

    /** Interface to test validity of strings.
        Different games allow different characters at different places.
        - v3 games are played with a DOS codepage
        - friendly codes do not allow "high-ASCII" characters
        - nu has trouble with things like '<' or '&'

        The scope of these tests is to test printable characters and overall string lengths.
        It is not scope of these tests to verify linefeed formats etc. */
    class StringVerifier : public afl::base::Deletable, public afl::base::Clonable<StringVerifier> {
     public:
        /** Context in which a string is used. */
        enum Context {
            /** Unknown context; make a guess.
                This should be an estimate, not a conservative assumption:
                This test is allowed to accept strings that another context rejects. */
            Unknown,

            /** Ship name. */
            ShipName,

            /** Planet name. */
            PlanetName,

            /** Long player name (see Player::LongName). */
            PlayerLongName,

            /** Short player name (see Player::ShortName). */
            PlayerShortName,

            /** Player adjective name (see Player::AdjectiveName). */
            PlayerAdjectiveName,

            /** Friendly code. */
            FriendlyCode,

            /** In-game message. */
            Message
        };

        /** Test validity of a string.
            \param ctx Context
            \param text String to test */
        virtual bool isValidString(Context ctx, const String_t& text) = 0;

        /** Test validity of a character.
            \param ctx Context
            \param ch Character to test */
        virtual bool isValidCharacter(Context ctx, afl::charset::Unichar_t ch) = 0;

        /** Get maximum possible string length.
            \param ctx Context
            \return Number of characters / UTF-8 runes (not bytes!) */
        virtual size_t getMaxStringLength(Context ctx) = 0;

     protected:
        bool defaultIsValidString(Context ctx, const String_t& text);
    };

}

#endif
