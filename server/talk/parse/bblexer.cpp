/**
  *  \file server/talk/parse/bblexer.cpp
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

// /** Constructor.
//     \param text Text */
server::talk::parse::BBLexer::BBLexer(const String_t& text)
    : text(text),
      cursor(0),
      token_start(0),
      token_length(0),
      attr_start(0),
      attr_length(0),
      tag()
{
    // ex BBLexer::BBLexer
}

// /** Read a token. Returns the token's type. */
server::talk::parse::BBLexer::Token
server::talk::parse::BBLexer::read()
{
    // ex BBLexer::read
    token_start  = cursor;
    token_length = 0;
    attr_start   = 0;
    attr_length  = 0;

    // End?
    if (cursor >= text.size()) {
        token = Eof;
        return token;
    }

    // What is it?
    char ch = text[cursor++];
    Token result;
    if (ch == '\n' && findNewline()) {
        // Double newline. This is a new paragraph.
        result = Paragraph;
        skipBlanks();
    } else if (ch == '[') {
        // Could be any kind of tag
        if (cursor >= text.size()) {
            // text ends with "["
            result = Text;
        } else if (text[cursor] == '/') {
            // Possibly a closing tag
            ++cursor;
            tag.clear();
            while (cursor < text.size() && isTagChar(text[cursor])) {
                tag.append(1, afl::string::charToLower(text[cursor]));
                ++cursor;
            }
            if (cursor < text.size() && text[cursor] == ']') {
                // ok
                ++cursor;
                result = TagEnd;
            } else {
                // not a closing tag
                result = Text;
            }
        } else if (text[cursor] == '*') {
            // Possibly a list item
            ++cursor;
            if (cursor < text.size() && text[cursor] == ']') {
                // ok
                ++cursor;
                tag = "*";
                result = TagStart;
            } else {
                // not a list item
                result = Text;
            }
        } else if (text[cursor] == ':') {
            // Possibly a smiley
            tag.clear();
            ++cursor;
            while (cursor < text.size() && isTagChar(text[cursor])) {
                tag.append(1, afl::string::charToLower(text[cursor]));
                ++cursor;
            }
            if (text.size() - cursor >= 2 && text[cursor] == ':' && text[cursor+1] == ']') {
                // Smiley
                cursor += 2;
                result = Smiley;
                attr_start  = 0;
                attr_length = 0;
            } else {
                // Not a smiley
                result = Text;
            }
        } else {
            // Possibly a tag
            tag.clear();
            while (cursor < text.size() && isTagChar(text[cursor])) {
                tag.append(1, afl::string::charToLower(text[cursor]));
                ++cursor;
            }
            if (cursor >= text.size()) {
                // end with partial tag
                result = Text;
            } else if (text[cursor] == ']') {
                // tag without attribute
                ++cursor;
                attr_start  = 0;
                attr_length = 0;
                result = TagStart;
            } else if (text[cursor] == '=') {
                // tag with attribute
                ++cursor;
                if (text[cursor] == '"') {
                    ++cursor;
                    attr_start = cursor;
                    while (cursor < text.size() && text[cursor] != '\n' && text[cursor] != '"')
                        ++cursor;
                    if (cursor+1 < text.size() && text[cursor] == '"' && text[cursor+1] == ']') {
                        // valid tag
                        attr_length = cursor - attr_start;
                        cursor += 2;
                        result = TagStart;
                    } else {
                        // partial tag
                        result = Text;
                    }
                } else {
                    attr_start = cursor;
                    while (cursor < text.size() && text[cursor] != '\n' && text[cursor] != ']')
                        ++cursor;
                    if (cursor < text.size() && text[cursor] == ']') {
                        // valid tag
                        attr_length = cursor - attr_start;
                        ++cursor;
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
        attr_start = cursor;
        while (cursor < text.size() && isUserChar(text[cursor]))
            ++cursor;
        attr_length = cursor - attr_start;
        if (attr_length != 0) {
            result = AtLink;
        }
    } else {
        // Text
        result = Text;
        bool allowAt = !isUserChar(ch);
        while (cursor < text.size()) {
            char ch = text[cursor];
            if (ch == '\n' || ch == '[') {
                break;
            } else if (ch == '@' && allowAt) {
                break;
            } else {
                allowAt = !isUserChar(ch);
            }
            ++cursor;
        }
    }
    token_length = cursor - token_start;
    token = result;
    return result;
}

// /** Get current token.
//     \return current token type, i.e. last value returned by read() */
server::talk::parse::BBLexer::Token
server::talk::parse::BBLexer::getTokenType() const
{
    // ex BBLexer::getTokenType
    return token;
}

// /** Get current token string.
//     \return complete token text */
String_t
server::talk::parse::BBLexer::getTokenString() const
{
    // ex BBLexer::getTokenString
    return String_t(text, token_start, token_length);
}

// /** Get current tag. For TagStart and TagEnd only.
//     \return tag, in lower-case */
String_t
server::talk::parse::BBLexer::getTag() const
{
    // ex BBLexer::getTag
    return tag;
}

// /** Get current attribute. For TagStart only.
//     \return attribute */
String_t
server::talk::parse::BBLexer::getAttribute() const
{
    // ex BBLexer::getAttribute
    return String_t(text, attr_start, attr_length);
}

// /** Skip blanks. Advances cursor until it sits at the end or a non-blank. */
void
server::talk::parse::BBLexer::skipBlanks()
{
    // ex BBLexer::skipBlanks
    while (cursor < text.size() && (text[cursor] == '\r' || text[cursor] == '\n' || text[cursor] == '\t' || text[cursor] == ' ')) {
        ++cursor;
    }
}

// /** Skip blanks until a newline is found.
//     \retval true Newline was found, cursor was updated
//     \retval false No newline found, cursor not modified */
bool
server::talk::parse::BBLexer::findNewline()
{
    // ex BBLexer::findNewline
    String_t::size_type i = cursor;
    while (i < text.size() && (text[i] == '\r' || text[i] == '\t' || text[i] == ' ')) {
        ++i;
    }
    if (i < text.size() && text[i] == '\n') {
        cursor = i;
        return true;
    } else {
        return false;
    }
}
