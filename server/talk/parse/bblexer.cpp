/**
  *  \file server/talk/parse/bblexer.cpp
  *  \brief Class server::talk::parse::BBLexer
  */

#include "server/talk/parse/bblexer.hpp"
#include "afl/string/char.hpp"

namespace {
    bool isTagChar(char c)
    {
        return (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z');
    }

    bool isUserChar(char c)
    {
        return (c >= 'A' && c <= 'Z')
            || (c >= 'a' && c <= 'z')
            || (c >= '0' && c <= '9')
            || c == '_';
    }
}

server::talk::parse::BBLexer::BBLexer(const String_t& text)
    : m_text(text),
      m_cursor(0),
      m_tokenStart(0),
      m_tokenLength(0),
      m_attributeStart(0),
      m_attributeLength(0),
      m_tag(),
      m_token(Eof)
{
    // ex BBLexer::BBLexer
}

server::talk::parse::BBLexer::Token
server::talk::parse::BBLexer::read()
{
    // ex BBLexer::read
    m_tokenStart  = m_cursor;
    m_tokenLength = 0;
    m_attributeStart   = 0;
    m_attributeLength  = 0;

    // End?
    if (m_cursor >= m_text.size()) {
        m_token = Eof;
        return m_token;
    }

    // What is it?
    char ch = m_text[m_cursor++];
    Token result;
    if (ch == '\n' && findNewline()) {
        // Double newline. This is a new paragraph.
        result = Paragraph;
        skipBlanks();
    } else if (ch == '[') {
        // Could be any kind of tag
        if (m_cursor >= m_text.size()) {
            // text ends with "["
            result = Text;
        } else if (m_text[m_cursor] == '/') {
            // Possibly a closing tag
            ++m_cursor;
            m_tag.clear();
            while (m_cursor < m_text.size() && isTagChar(m_text[m_cursor])) {
                m_tag.append(1, afl::string::charToLower(m_text[m_cursor]));
                ++m_cursor;
            }
            if (m_cursor < m_text.size() && m_text[m_cursor] == ']') {
                // ok
                ++m_cursor;
                result = TagEnd;
            } else {
                // not a closing tag
                result = Text;
            }
        } else if (m_text[m_cursor] == '*') {
            // Possibly a list item
            ++m_cursor;
            if (m_cursor < m_text.size() && m_text[m_cursor] == ']') {
                // ok
                ++m_cursor;
                m_tag = "*";
                result = TagStart;
            } else {
                // not a list item
                result = Text;
            }
        } else if (m_text[m_cursor] == ':') {
            // Possibly a smiley
            m_tag.clear();
            ++m_cursor;
            while (m_cursor < m_text.size() && isTagChar(m_text[m_cursor])) {
                m_tag.append(1, afl::string::charToLower(m_text[m_cursor]));
                ++m_cursor;
            }
            if (m_text.size() - m_cursor >= 2 && m_text[m_cursor] == ':' && m_text[m_cursor+1] == ']') {
                // Smiley
                m_cursor += 2;
                result = Smiley;
                m_attributeStart  = 0;
                m_attributeLength = 0;
            } else {
                // Not a smiley
                result = Text;
            }
        } else {
            // Possibly a tag
            m_tag.clear();
            while (m_cursor < m_text.size() && isTagChar(m_text[m_cursor])) {
                m_tag.append(1, afl::string::charToLower(m_text[m_cursor]));
                ++m_cursor;
            }
            if (m_cursor >= m_text.size()) {
                // end with partial tag
                result = Text;
            } else if (m_text[m_cursor] == ']') {
                // tag without attribute
                ++m_cursor;
                m_attributeStart  = 0;
                m_attributeLength = 0;
                result = TagStart;
            } else if (m_text[m_cursor] == '=') {
                // tag with attribute
                ++m_cursor;
                if (m_text[m_cursor] == '"') {
                    ++m_cursor;
                    m_attributeStart = m_cursor;
                    while (m_cursor < m_text.size() && m_text[m_cursor] != '\n' && m_text[m_cursor] != '"') {
                        ++m_cursor;
                    }
                    if (m_cursor+1 < m_text.size() && m_text[m_cursor] == '"' && m_text[m_cursor+1] == ']') {
                        // valid tag
                        m_attributeLength = m_cursor - m_attributeStart;
                        m_cursor += 2;
                        result = TagStart;
                    } else {
                        // partial tag
                        result = Text;
                    }
                } else {
                    m_attributeStart = m_cursor;
                    while (m_cursor < m_text.size() && m_text[m_cursor] != '\n' && m_text[m_cursor] != ']') {
                        ++m_cursor;
                    }
                    if (m_cursor < m_text.size() && m_text[m_cursor] == ']') {
                        // valid tag
                        m_attributeLength = m_cursor - m_attributeStart;
                        ++m_cursor;
                        result = TagStart;
                    } else {
                        // partial tag
                        result = Text;
                    }
                }
            } else {
                // partial tag
                result = Text;
            }
        }
    } else if (ch == '@') {
        // Could be an at-link
        result = Text;
        m_attributeStart = m_cursor;
        while (m_cursor < m_text.size() && isUserChar(m_text[m_cursor])) {
            ++m_cursor;
        }
        m_attributeLength = m_cursor - m_attributeStart;
        if (m_attributeLength != 0) {
            result = AtLink;
        }
    } else {
        // Text
        result = Text;
        bool allowAt = !isUserChar(ch);
        while (m_cursor < m_text.size()) {
            char ch = m_text[m_cursor];
            if (ch == '\n' || ch == '[') {
                break;
            } else if (ch == '@' && allowAt) {
                break;
            } else {
                allowAt = !isUserChar(ch);
            }
            ++m_cursor;
        }
    }
    m_tokenLength = m_cursor - m_tokenStart;
    m_token = result;

    // If token ends in a '\r', strip that.
    // Otherwise, paragraphs might be left ending in a \r if user submitted \r\n linefeeds.
    if (m_tokenLength > 0 && m_text[m_cursor-1] == '\r') {
        --m_tokenLength;
    }
    return result;
}

server::talk::parse::BBLexer::Token
server::talk::parse::BBLexer::getTokenType() const
{
    // ex BBLexer::getTokenType
    return m_token;
}

String_t
server::talk::parse::BBLexer::getTokenString() const
{
    // ex BBLexer::getTokenString
    return String_t(m_text, m_tokenStart, m_tokenLength);
}

String_t
server::talk::parse::BBLexer::getTag() const
{
    // ex BBLexer::getTag
    return m_tag;
}

String_t
server::talk::parse::BBLexer::getAttribute() const
{
    // ex BBLexer::getAttribute
    return String_t(m_text, m_attributeStart, m_attributeLength);
}

void
server::talk::parse::BBLexer::skipBlanks()
{
    // ex BBLexer::skipBlanks
    while (m_cursor < m_text.size() && (m_text[m_cursor] == '\r' || m_text[m_cursor] == '\n' || m_text[m_cursor] == '\t' || m_text[m_cursor] == ' ')) {
        ++m_cursor;
    }
}

/** Skip blanks until a newline is found.
    \retval true Newline was found, cursor was updated
    \retval false No newline found, cursor not modified */
bool
server::talk::parse::BBLexer::findNewline()
{
    // ex BBLexer::findNewline
    String_t::size_type i = m_cursor;
    while (i < m_text.size() && (m_text[i] == '\r' || m_text[i] == '\t' || m_text[i] == ' ')) {
        ++i;
    }
    if (i < m_text.size() && m_text[i] == '\n') {
        m_cursor = i;
        return true;
    } else {
        return false;
    }
}
