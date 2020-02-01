/**
  *  \file util/keystring.hpp
  *  \brief Class util::KeyString
  */
#ifndef C2NG_UTIL_KEYSTRING_HPP
#define C2NG_UTIL_KEYSTRING_HPP

#include "util/key.hpp"

namespace util {

    /** Handle for "a key and a string".
        Intended for locale-aware passing around of those pairs, typically for labelling buttons.

        \todo Right now, parses just the first character from a string.
        A future version would parse this information in a more sophisticated way to allow true internationalisation. */
    class KeyString {
     public:
        /** Construct from string literal.
            \param s string literal */
        explicit KeyString(const char* s);

        /** Construct from string.
            \param s string */
        explicit KeyString(const String_t& s);

        /** Construct from explicit paramters.
            \param s string */
        KeyString(const String_t& s, Key_t key);

        /** Get string.
            This string is used to label the button.
            \return string; 0 if none. */
        const String_t& getString() const;

        /** Get key.
            This key is used to trigger the button.
            \return key; 0 if none. */
        Key_t getKey() const;

     private:
        Key_t m_key;
        String_t m_string;
    };

}

#endif
