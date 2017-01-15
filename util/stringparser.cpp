/**
  *  \file util/stringparser.cpp
  *  \brief Class util::StringParser
  */

#include <cstring>
#include "util/stringparser.hpp"
#include "afl/string/parse.hpp"

// Constructor.
util::StringParser::StringParser(const String_t& s)
    : m_string(s),
      m_pos(0)
{ }

// Check constant string segment.
bool
util::StringParser::parseString(const char* s)
{
    const String_t::size_type len = std::strlen(s);
    if (m_string.size() - m_pos >= len && m_string.compare(m_pos, len, s, len) == 0) {
        m_pos += len;
        return true;
    } else {
        return false;
    }
}

// Check character literal.
bool
util::StringParser::parseChar(char ch)
{
    if (m_pos < m_string.size() && ch == m_string[m_pos]) {
        ++m_pos;
        return true;
    } else {
        return false;
    }
}

// Check delimited variable string.
bool
util::StringParser::parseDelim(const char* delim, String_t& out)
{
    const String_t::size_type start = m_pos;
    while (m_pos < m_string.size() && std::strchr(delim, m_string[m_pos]) == 0) {
        ++m_pos;
    }
    out.assign(m_string, start, m_pos - start);
    return true;
}

// Check variable integer.
bool
util::StringParser::parseInt(int& out)
{
    String_t::size_type err, err2;
    if (afl::string::strToInteger(m_string.substr(m_pos), out, err)) {
        // completely valid
        m_pos = m_string.size();
        return true;
    } else if (err != 0 && afl::string::strToInteger(m_string.substr(m_pos, err), out, err2)) {
        // part is valid
        m_pos += err;
        return true;
    } else {
        // invalid
        return false;
    }
}

// Parse character class.
bool
util::StringParser::parseWhile(bool classify(char), String_t& out)
{
    const String_t::size_type start = m_pos;
    while (m_pos < m_string.size() && classify(m_string[m_pos])) {
        ++m_pos;
    }
    out.assign(m_string, start, m_pos - start);
    return m_pos != start;
}

// Check end of string.
bool
util::StringParser::parseEnd()
{
    return m_pos == m_string.size();
}

// Get current character.
bool
util::StringParser::getCurrentChar(char& ch) const
{
    if (m_pos < m_string.size()) {
        ch = m_string[m_pos];
        return true;
    } else {
        return false;
    }
}

// Get remaining unparsed text.
String_t
util::StringParser::getRemainder() const
{
    return m_string.substr(m_pos);
}
