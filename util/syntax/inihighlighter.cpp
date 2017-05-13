/**
  *  \file util/syntax/inihighlighter.cpp
  */

#include "util/syntax/inihighlighter.hpp"
#include "util/syntax/segment.hpp"
#include "util/syntax/keywordtable.hpp"

namespace {
    enum Class {
        cSpace     = 1,
        cCommentH  = 2,
        cCommentS  = 4,
        cLBracket  = 8,
        cRBracket  = 16,
        cPercent   = 32,
        cEqual     = 64,
        cNewline   = 128,
        cOther     = 256
    };

    Class classify(char c) {
        switch (c) {
         case ' ':
         case '\t':
         case '\v':
         case '\f':
            return cSpace;

         case '\r':
         case '\n':
            return cNewline;

         case '#':
            return cCommentH;

         case ';':
            return cCommentS;

         case '[':
            return cLBracket;

         case ']':
            return cRBracket;

         case '%':
            return cPercent;

         case '=':
            return cEqual;

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

// /** Constructor.
//     \param db Syntax database
//     \param defaultSection Start interpreting keys in this section */
util::syntax::IniHighlighter::IniHighlighter(const KeywordTable& tab, String_t defaultSection)
    : m_table(tab),
      m_section(defaultSection),
      m_text(),
      m_state(sBOL)
{
    // ex SyntaxIni::SyntaxIni
}

util::syntax::IniHighlighter::~IniHighlighter()
{ }

void
util::syntax::IniHighlighter::init(afl::string::ConstStringMemory_t text)
{
    m_text = text;
    m_state = sBOL;
}

bool
util::syntax::IniHighlighter::scan(Segment& result)
{
    // ex SyntaxIni::scan
    // End reached?
    if (m_text.empty()) {
        return false;
    }

    // Dispatch depending on state
    switch (m_state) {
     case sBOL:
        result.start(m_text);
        if (skip(m_text, cSpace | cNewline)) {
            // Whitespace
            result.finish(DefaultFormat, m_text);
        } else if (skip1(m_text, cCommentS)) {
            // Comment (;)
            Format fmt = skip1(m_text, cCommentS) ? Comment2Format : CommentFormat;
            skip(m_text, ~cNewline);
            result.finish(fmt, m_text);
        } else if (skip1(m_text, cCommentH)) {
            // Comment (#)
            Format fmt = skip1(m_text, cCommentH) ? Comment2Format : CommentFormat;
            skip(m_text, ~cNewline);
            result.finish(fmt, m_text);
        } else if (skip1(m_text, cLBracket)) {
            // "["
            Segment tmp;
            tmp.start(m_text);
            skip(m_text, ~(cNewline | cRBracket));
            tmp.finish(DefaultFormat, m_text);
            m_section = afl::string::strTrim(afl::string::fromMemory(tmp.getText()));
            if (skip1(m_text, cRBracket)) {
                m_state = sAfterSection;
            }
            result.finish(SectionFormat, m_text);
        } else if (skip1(m_text, cPercent)) {
            // "%"
            skip(m_text, cSpace);
            Segment tmp;
            tmp.start(m_text);
            skip(m_text, ~(cNewline | cSpace));
            tmp.finish(DefaultFormat, m_text);
            m_section = afl::string::strTrim(afl::string::fromMemory(tmp.getText()));
            result.finish(SectionFormat, m_text);
            m_state = sAfterSection;
        } else if (skip(m_text, cOther | cLBracket | cRBracket)) {
            // Name. Accept [] as well for things like "foo[1] = ...", maybe someone uses that.
            // FIXME? If a word stands alone on a line, we do not highlight it, but if it has trailing space, we do.
            // PlanetsCentral does this, do we want to keep it?
            if (skip1(m_text, cNewline)) {
                result.finish(DefaultFormat, m_text);
            } else {
                result.finish(NameFormat, m_text);
                m_state = sAfterName;

                // Links
                String_t key = afl::string::strLTrim(afl::string::fromMemory(result.getText()));

                String_t pfx = "ini.";
                String_t::size_type dot = key.find('.');
                if (dot != String_t::npos
                    && (m_section.empty()
                        || (dot == m_section.size() && afl::string::strCaseCompare(m_section, key.substr(0, dot)) == 0)))
                {
                    pfx += key;
                } else {
                    pfx += m_section;
                    pfx += ".";
                    pfx += key;
                }
                if (const String_t* link = m_table.get(pfx + ".link")) {
                    result.setLink(*link);
                }
                if (const String_t* info = m_table.get(pfx + ".info")) {
                    result.setInfo(*info);
                }
            }
        } else {
            // Don't know what it is. Skip the whole line.
            skip(m_text, ~cNewline);
            skip1(m_text, cNewline);
            result.finish(DefaultFormat, m_text);
        }
        break;

     case sAfterSection:
        result.start(m_text);
        if (skip(m_text, cSpace)) {
            // At least some spaces
            if (skip1(m_text, cNewline)) {
                m_state = sBOL;
            }
            result.finish(DefaultFormat, m_text);
        } else if (skip(m_text, cCommentS | cCommentH)) {
            // A comment
            skip(m_text, ~cNewline);
            if (skip1(m_text, cNewline)) {
                m_state = sBOL;
            }
            result.finish(CommentFormat, m_text);
        } else {
            // Something else
            skip(m_text, ~cNewline);
            if (skip1(m_text, cNewline)) {
                m_state = sBOL;
            }
            result.finish(DefaultFormat, m_text);
        }
        break;

     case sAfterName:
        result.start(m_text);
        skip(m_text, ~cNewline);
        if (skip1(m_text, cNewline)) {
            m_state = sBOL;
        }
        result.finish(DefaultFormat, m_text);
        break;
    }
    return true;
}
