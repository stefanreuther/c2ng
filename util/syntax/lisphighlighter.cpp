/**
  *  \file util/syntax/lisphighlighter.cpp
  *  \brief Class util::syntax::LispHighlighter
  */

#include "util/syntax/lisphighlighter.hpp"
#include "util/syntax/segment.hpp"

namespace {
    enum Class {
        cSemicolon  = 1,        // ";"
        cBackslash  = 2,        // "\"
        cDQ         = 4,        // """
        cNewline    = 8,
        cOther      = 16
    };

    Class classify(char c) {
        switch (c) {
         case ';':
            return cSemicolon;
         case '\\':
            return cBackslash;
         case '\r':
         case '\n':
            return cNewline;
         case '"':
            return cDQ;
         default:
            return cOther;
        }
    }

    bool skip(afl::string::ConstStringMemory_t& text, int c)
    {
        bool result = false;
        const char* p;
        while ((p = text.at(0)) != 0 && (classify(*p) & c) != 0) {
            text.eat();
            result = true;
        }
        return result;
    }

    bool skip1(afl::string::ConstStringMemory_t& text, int c)
    {
        bool result = false;
        const char* p;
        if ((p = text.at(0)) != 0 && (classify(*p) & c) != 0) {
            text.eat();
            result = true;
        }
        return result;
    }
}

util::syntax::LispHighlighter::LispHighlighter()
    : m_text()
{ }

util::syntax::LispHighlighter::~LispHighlighter()
{ }

void
util::syntax::LispHighlighter::init(afl::string::ConstStringMemory_t text)
{
    m_text = text;
}

bool
util::syntax::LispHighlighter::scan(Segment& result)
{
    // Standard stuff
    result.start(m_text);
    if (skip(m_text, cOther | cNewline)) {
        result.finish(DefaultFormat, m_text);
        return true;
    }

    // String
    if (skip1(m_text, cDQ)) {
        while (1) {
            skip(m_text, ~(cDQ | cBackslash));
            if (skip1(m_text, cBackslash)) {
                // Quote within string
                m_text.eat();
            } else {
                // skip ended either with a double-quote, or with end of string. Anyway, stop.
                skip1(m_text, cDQ);
                break;
            }
        }
        result.finish(StringFormat, m_text);
        return true;
    }

    // Comment
    if (skip1(m_text, cSemicolon)) {
        skip(m_text, ~cNewline);
        result.finish(CommentFormat, m_text);
        return true;
    }

    // Backslash
    if (skip1(m_text, cBackslash)) {
        m_text.eat();
        result.finish(DefaultFormat, m_text);
        return true;
    }

    return false;
}
