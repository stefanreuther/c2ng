/**
  *  \file util/stringparser.hpp
  *  \brief Class util::StringParser
  */
#ifndef C2NG_UTIL_STRINGPARSER_HPP
#define C2NG_UTIL_STRINGPARSER_HPP

#include "afl/string/string.hpp"

namespace util {

    /** Simple string parser.
        Allows dissecting a string containing fixed and variable string segments.
        Call its "parse" functions in sequence to consume the input string.
        Each function returns true on match, false on mismatch; on match, the pointer is advanced. */
    class StringParser {
     public:
        /** Constructor. */
        StringParser(const String_t& s);

        /** Check constant string segment.
            \param s String literal
            \return true on match, false on mismatch */
        bool parseString(const char* s);

        /** Check character literal.
            \param ch Character
            \return true on match, false on mismatch */
        bool parseCharacter(char ch);

        /** Check delimited variable string.
            Accumulates all characters in \c out, until a delimiter or the end of the string is found.
            \param delim List of delimiters
            \param out [out] Result
            \return true (always succeeds) */
        bool parseDelim(const char* delim, String_t& out);

        /** Check delimited variable string (greedy).
            Accumulates all characters in \c out, until a delimiter or the end of the string is found.
            If multiple delimiters exist in the string, picks the last one.
            \param delim List of delimiters
            \param out [out] Result
            \return true (always succeeds) */
        bool parseDelimGreedy(const char* delim, String_t& out);

        /** Check variable integer.
            \param out [out] Integer
            \return true on match, false on mismatch */
        bool parseInt(int& out);

        /** Check variable 64-bit integer.
            \param out [out] Integer
            \return true on match, false on mismatch */
        bool parseInt64(int64_t& out);

        /** Parse character class.
            Accumulates all characters in \c out as long as the classification function returns true.
            \param classify Function that classifies characters
            \param out [out] Result
            \return true if a nonzero string results */
        bool parseWhile(bool classify(char), String_t& out);

        /** Check end of string.
            \return true if end was reached, false if not */
        bool parseEnd();

        /** Get current character.
            Does not modify the state.
            \param ch [out] Character
            \return true if character could be obtained, false if end has been reached */
        bool getCurrentCharacter(char& ch) const;

        /** Get remaining unparsed text.
            \return text */
        String_t getRemainder() const;

        /** Get current parser position.
            \return position */
        size_t getPosition() const;

     private:
        String_t m_string;
        String_t::size_type m_pos;
    };

}

#endif
