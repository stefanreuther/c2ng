/**
  *  \file util/syntax/scripthighlighter.cpp
  */

#include "util/syntax/scripthighlighter.hpp"
#include "util/syntax/segment.hpp"
#include "interpreter/keywords.hpp"

namespace {
    enum Class {
        cSpace    = 1,
        cNewline  = 2,
        cComment  = 4,
        cLParen   = 8,
        cRParen   = 16,
        cDot      = 32,
        cQuote    = 64,
        cComma    = 128,
        cDigit    = 256,
        cLetter   = 512,
        cOther    = 1024,
        cSQuote   = 2048
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

         case '%':
            return cComment;

         case '(':
            return cLParen;

         case ')':
            return cRParen;

         case '.':
            return cDot;

         case '\'':
            return cSQuote;

         case '"':
            return cQuote;

         case ',':
            return cComma;

         default:
            if (c >= '0' && c <= '9') {
                return cDigit;
            } else if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
                return cLetter;
            } else {
                return cOther;
            }
        }
    }

    bool skip(afl::string::ConstStringMemory_t& str, int c)
    {
        bool result = false;
        const char* p;
        while ((p = str.at(0)) != 0 && (classify(*p) & c) != 0) {
            str.eat();
            result = true;
        }
        return result;
    }

    // size_t skip(afl::string::ConstStringMemory_t& str, size_t pos, int c)
    // {
    //     const char* p;
    //     while ((p = str.at(start)) != 0 && (classify(*p) & c) != 0) {
    //         ++pos;
    //     }
    //     return pos;
    // }

    bool skip1(afl::string::ConstStringMemory_t& str, int c)
    {
        bool result = false;
        const char* p;
        if ((p = str.at(0)) != 0 && (classify(*p) & c) != 0) {
            str.eat();
            result = true;
        }
        return result;
    }

    void skipString(afl::string::ConstStringMemory_t& str)
    {
        bool quote = false;
        while (const char* p = str.eat()) {
            if (quote) {
                quote = false;
            } else if (*p == '\\') {
                quote = true;
            } else if (*p == '\"') {
                break;
            } else {
                // normal character
            }
        }
    }
}


util::syntax::ScriptHighlighter::ScriptHighlighter(const KeywordTable& table)
    : Highlighter(),
      m_table(table),
      m_text(),
      m_state(sDefaultBOL),
      m_parenLevel(0)
{
    // ex SyntaxScript::SyntaxScript
}

util::syntax::ScriptHighlighter::~ScriptHighlighter()
{ }

void
util::syntax::ScriptHighlighter::init(afl::string::ConstStringMemory_t text)
{
    // ex SyntaxScript::init
    m_text = text;
    m_state = sDefaultBOL;
    m_parenLevel = 0;
}

bool
util::syntax::ScriptHighlighter::scan(Segment& result)
{
    // ex SyntaxScript::scan
    result.start(m_text);
    if (skip(m_text, cSpace)) {
        // Whitespace
        result.finish(DefaultFormat, m_text);
        return true;
    } else if (skip(m_text, cNewline)) {
        // Newline
        m_state = sDefaultBOL;
        m_parenLevel = 0;
        result.finish(DefaultFormat, m_text);
        return true;
    } else if (skip(m_text, cComment)) {
        // Comment
        skip(m_text, ~cNewline);
        result.finish(CommentFormat, m_text);
        return true;
    } else if (skip1(m_text, cLParen)) {
        // Left paren
        ++m_parenLevel;
        result.finish(DefaultFormat, m_text);
        leaveDefault();
        return true;
    } else if (skip1(m_text, cRParen)) {
        // Right paren
        if (m_parenLevel > 0) {
            --m_parenLevel;
        }
        result.finish(DefaultFormat, m_text);
        leaveDefault();
        return true;
    } else if (skip1(m_text, cSQuote)) {
        // Single-quoted string
        skip(m_text, ~cSQuote);
        skip1(m_text, cSQuote);
        result.finish(StringFormat, m_text);
        return true;
    } else if (skip1(m_text, cQuote)) {
        // String
        skipString(m_text);
        result.finish(StringFormat, m_text);
        leaveDefault();
        return true;
    } else if (skip(m_text, cComma)) {
        // Comma
        leaveDefault();
        if (m_state == sAfterSubDef && m_parenLevel == 1) {
            m_state = sAfterSub;
        } else if (m_state == sAfterDimDef && m_parenLevel == 0) {
            m_state = sAfterDim;
        } else {
            // nothing particular
        }
        result.finish(DefaultFormat, m_text);
        return true;
    } else if (skip(m_text, cDigit)) {
        // Number
        skip(m_text, cDigit | cDot);
        result.finish(DefaultFormat, m_text);
        leaveDefault();
        return true;
    } else if (skip(m_text, cLetter)) {
        // Identifier
        skip(m_text, cLetter | cDot | cDigit);

        // Finish the result so we can obtain the text easily
        result.finish(DefaultFormat, m_text);
        String_t id(afl::string::strUCase(afl::string::fromMemory(result.getText())));

        // Look up keyowrd
        interpreter::Keyword kw = interpreter::lookupKeyword(id);

        // The following keywords are not reported by lookupKeyword.
        // Translate them into a harmless keyword that doesn't cause unnecessary state transitions.
        if (kw == interpreter::kwNone && (id == "TRUE" || id == "FALSE" || id == "PI" || id == "AND" || id == "OR"
                                          || id == "XOR" || id == "NOT" || id == "MOD"))
        {
            kw = interpreter::kwPrint;
        }

        // Perform state transitions
        switch (m_state) {
         case sDefaultBOL:
            if (kw == interpreter::kwNone) {
                // Not a keyword
                m_state = sDefault;
                result.setFormat(DefaultFormat);
            } else if (kw == interpreter::kwSub || kw == interpreter::kwFunction) {
                // Sub/Function: handle that it is followed by a name, and a parameter list
                m_state = sAfterSub;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwDim || kw == interpreter::kwLocal || kw == interpreter::kwStatic || kw == interpreter::kwShared || kw == interpreter::kwCreatePlanetProperty || kw == interpreter::kwCreateShipProperty) {
                // Dim/Local/Static/Shared: handle that it is followed by an initializer list
                m_state = sAfterDim;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwFor) {
                // For: "To" is a keyword, "Do" ends the statement
                m_state = sAfterFor;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwDo || kw == interpreter::kwLoop) {
                // Do/Loop: "Until" is a keyword
                m_state = sAfterLoop;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwIf) {
                // If: "Then" is a keyword and ends the statement
                m_state = sAfterIf;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwCase) {
                // Case: "Is" is a keyword
                m_state = sAfterCase;
                result.setFormat(KeywordFormat);
            } else if (kw == interpreter::kwOn || kw == interpreter::kwWith) {
                // On/With: "Do" ends the statement
                m_state = sAfterWith;
                result.setFormat(KeywordFormat);
            } else {
                // Just a keyword
                m_state = sDefault;
                result.setFormat(KeywordFormat);
            }
            break;

         case sDefault:
         case sAfterSubDef:
            result.setFormat(kw == interpreter::kwNone ? DefaultFormat : KeywordFormat);
            break;

         case sAfterDimDef:
            result.setFormat((kw == interpreter::kwNone && id != "AS") ? DefaultFormat : KeywordFormat);
            break;

         case sAfterSub:
            if (kw == interpreter::kwNone && id != "OPTIONAL") {
                if (m_parenLevel == 0) {
                    result.setFormat(NameFormat);
                } else if (m_parenLevel == 1) {
                    result.setFormat(NameFormat);
                    m_state = sAfterSubDef;
                } else {
                    result.setFormat(DefaultFormat);
                }
            } else {
                result.setFormat(KeywordFormat);
            }
            break;
            
         case sAfterDim:
            if (kw == interpreter::kwNone) {
                if (m_parenLevel == 0) {
                    result.setFormat(NameFormat);
                    m_state = sAfterDimDef;
                } else {
                    result.setFormat(DefaultFormat);
                }
            } else if (kw == interpreter::kwFunction || kw == interpreter::kwSub) {
                m_state = sAfterSub;
                result.setFormat(KeywordFormat);
            } else {
                result.setFormat(KeywordFormat);
            }
            break;

         case sAfterFor:
            result.setFormat(kw == interpreter::kwNone && id != "TO" ? DefaultFormat : KeywordFormat);
            if (kw == interpreter::kwDo) {
                m_state = sDefaultBOL;
            }
            break;

         case sAfterLoop:
            result.setFormat(kw == interpreter::kwNone && id != "UNTIL" ? DefaultFormat : KeywordFormat);
            break;

         case sAfterIf:
            result.setFormat(kw == interpreter::kwNone && id != "THEN" ? DefaultFormat : KeywordFormat);
            if (id == "THEN") {
                m_state = sDefaultBOL;
            }
            break;

         case sAfterCase:
            result.setFormat(kw == interpreter::kwNone && id != "IS" ? DefaultFormat : KeywordFormat);
            break;

         case sAfterWith:
            result.setFormat(kw == interpreter::kwNone ? DefaultFormat : KeywordFormat);
            if (kw == interpreter::kwDo) {
                m_state = sDefaultBOL;
            }
            break;
        }
        return true;
    } else if (skip(m_text, cDot | cOther)) {
        // Anything else
        result.finish(DefaultFormat, m_text);
        leaveDefault();
        return true;
    } else {
        return false;
    }
}

// /** Leave default state. Call whenever a non-whitespace token is consumed.
//     This will reset the state from sDefaultBOL to sDefault, to turn off statement recognition. */
void
util::syntax::ScriptHighlighter::leaveDefault()
{
    // ex SyntaxScript::leaveDefault
    if (m_state == sDefaultBOL) {
        m_state = sDefault;
    }
}
