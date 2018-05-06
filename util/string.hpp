/**
  *  \file util/string.hpp
  *  \brief String Utilities
  */
#ifndef C2NG_UTIL_STRING_HPP
#define C2NG_UTIL_STRING_HPP

#include "afl/string/string.hpp"

namespace util {

    /** String Match, PHost way.
        \param pattern  Pattern we want to match against. Consists of
        an initial sequence of capital letters, followed
        by a sequence of other letters. Lower-case letters
        are optional, the rest is mandatory.
        \param tester   The string to test against the pattern.

        Matching is case-insensitive.

        Example: stringMatch("ENglish", p) matches if p is
        "english", or any abbreviation of "english" up to "en". */
    bool stringMatch(const char* pattern, const char* tester);
    bool stringMatch(const char* pattern, const String_t& tester);

    /** Consume word from comma-separated list.
        This is intended for matching of user-given words against a fixed template list,
        thus there are no provisions for specially processing the template (e.g. no removal of whitespace).
        \param tpl [in/out] Comma-separated word list
        \param word [out] Word
        \retval true Word has been consumed from tpl and pointer advanced; \c word has been set
        \retval false No more words, nothing changed */
    bool eatWord(const char*& tpl, String_t& word);

    /** Parse a range. Syntax is one of
        - "nn", means set min=max=nn
        - "nn-", means set min=nn, do not modify max
        - "nn-mm", means set min=nn, max=mm

        Note that this function does not enforce that the result actually is a real range with min<=max,
        and does not enforce that the returned range is a subrange of [min,max].
        It just parses the numbers, and min/max are only default values.

        \param s    [in] User input
        \param min  [in/out] Default range start
        \param max  [in/out] Default range end
        \param pos  [out] Position where parsing stopped if return value is false
        \return true iff input was completely valid, false otherwise (min,max unmodified, pos set) */
    bool parseRange(const String_t& s, int& min, int& max, String_t::size_type& pos);

    /** Parse a player character.
        Characters are '0'..'9' for players 0-9, 'a'-'z' (or 'A'-'Z') for 10-35.
        Typically not all values are valid players, it is up to the caller to decide.
        \param ch [in] User input
        \param number [out] Player number
        \return true if character was parsed correctly, false on error */
    bool parsePlayerCharacter(const char ch, int& number);

    /** Parse a boolean value.
        This is intended for booleans parsed from command-line options and configuration files.
        \param s [in] String
        \param result [out] Result
        \retval true String parsed correctly, result has been updated
        \retval false String not valid, result unchanged */
    bool parseBooleanValue(const String_t& s, bool& result);

    /** Format a textual list of options.
        The string can contain
        - regular lines (subheadings, blank lines)
        - options, separated from their descriptions by a tab

        This function will format all options such that the descriptions line up nicely.
        For example,
        <pre>
        Options:
        -width[tab]Set the width
        -height[tab]Set the height
        </pre>
        will be formatted to
        <pre>
        Options:
          -width    Set the width
          -height   Set the height
        </pre>

        \param s String to format
        \return result */
    String_t formatOptions(String_t s);

    /** Beautify variable name.
        Converts UGLY.CAPS to Nicely.Formatted.Text.
        \param name Name
        \return formatted name */
    String_t formatName(String_t name);

    /** Encode MIME header.
        \param input text to encode
        \param charset character set */
    String_t encodeMimeHeader(String_t input, String_t charsetName);

    /** Encode as HTML.
        \param input text to encode
        \param rawUnicode If true, Unicode characters are reproduced as-is.
                          This saves memory but requires the output to be delivered to the client with a "UTF-8" character set declaration.
                          If false, Unicode characters are encoded as numerical escapes,
                          which requires more memory but is independant of the character set.
        \return encoded text */
    String_t encodeHtml(const String_t& input, bool rawUnicode);

}

#endif
