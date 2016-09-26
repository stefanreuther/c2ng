/**
  *  \file util/keystring.cpp
  *  \brief Class util::KeyString
  */

#include "util/keystring.hpp"
#include "afl/string/char.hpp"

namespace {
    util::Key_t getKeyFromChar(char c)
    {
        // ex ui/keystring.cc:keyFromChar
        // Only convert sensible ASCII characters
        char k = afl::string::charToLower(c);
        if (k > ' ' && k < 127) {
            return k;
        } else {
            return 0;
        }
    }
}


// Construct from string literal.
util::KeyString::KeyString(const char* s)
    : m_key(getKeyFromChar(s[0])),
      m_string(s)
{
    // ex UIKeyString::UIKeyString
}

// Construct from string.
util::KeyString::KeyString(const String_t& s)
    : m_key(getKeyFromChar(s[0])),
      m_string(s)
{ }

// Get string.
const String_t&
util::KeyString::getString() const
{
    return m_string;
}

// Get key.
util::Key_t
util::KeyString::getKey() const
{
    return m_key;
}
