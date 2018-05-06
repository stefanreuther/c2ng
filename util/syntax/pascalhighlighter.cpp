/**
  *  \file util/syntax/pascalhighlighter.cpp
  *  \brief Class util::syntax::PascalHighlighter
  */

#include "util/syntax/pascalhighlighter.hpp"
#include "afl/base/countof.hpp"
#include "afl/string/char.hpp"
#include "util/syntax/segment.hpp"

namespace {
    enum Class {
        cWhitespace = 1,        // horizontal whitespace
        cNewline    = 2,        // newline
        cLParen     = 4,        // '('
        cRParen     = 8,        // ')'
        cLBrace     = 16,       // '{'
        cRBrace     = 32,       // '}'
        cStar       = 64,       // '*'
        cQuote      = 128,      // '''
        cSlash      = 256,      // '/'
        cLetter     = 512,      // alpha or _
        cDigit      = 1024,     // 0-9
        cOther      = 2048
    };

    Class classify(char c) {
        switch (c) {
         case ' ':
         case '\t':
         case '\v':
         case '\f':
            return cWhitespace;

         case '\r':
         case '\n':
            return cNewline;

         case '(':
            return cLParen;

         case ')':
            return cRParen;

         case '{':
            return cLBrace;

         case '}':
            return cRBrace;

         case '*':
            return cStar;

         case '\'':
            return cQuote;

         case '/':
            return cSlash;

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

    bool isDollar(afl::string::ConstStringMemory_t& text)
    {
        const char* p = text.at(0);
        return p != 0 && *p == '$';
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

    String_t getIdentifier(afl::string::ConstStringMemory_t text)
    {
        String_t result;
        while (const char* p = text.eat()) {
            result += afl::string::charToLower(*p);
        }
        return result;
    }

    /*
     *  Keywords
     *
     *  TP6: from hardcopy manual
     *  Delphi: from https://en.wikibooks.org/w/index.php?title=Delphi_Programming/Reserved_keywords&oldid=2753942
     *  FreePascal: from http://wiki.freepascal.org/index.php?title=Reserved_words&oldid=114947
     */

    static const char*const KEYWORDS[] = {
        "absolute",               // TP6 directive
        "and",                    // TP6 keyword, Delphi, FreePascal
        "array",                  // TP6 keyword, Delphi, FreePascal
        "as",                     // Delphi, FreePascal
        "asm",                    // TP6 keyword, Delphi, FreePascal
        "assembler",              // TP6 directive
        "begin",                  // TP6 keyword, Delphi, FreePascal
        "break",                  // FreePascal
        "case",                   // TP6 keyword, Delphi, FreePascal
        "class",                  // Delphi, FreePascal
        "const",                  // TP6 keyword, Delphi, FreePascal
        "constructor",            // TP6 keyword, Delphi, FreePascal
        "continue",               // FreePascal
        "destructor",             // TP6 keyword, Delphi, FreePascal
        "dispinterface",          // Delphi
        "dispose",                // FreePascal
        "div",                    // TP6 keyword, Delphi, FreePascal
        "do",                     // TP6 keyword, Delphi, FreePascal
        "downto",                 // TP6 keyword, Delphi, FreePascal
        "else",                   // TP6 keyword, Delphi, FreePascal
        "end",                    // TP6 keyword, Delphi, FreePascal
        "except",                 // Delphi, FreePascal
        "exit",                   // FreePascal
        "exports",                // Delphi, FreePascal
        "external",               // TP6 directive
        "false",                  // FreePascal
        "far",                    // TP6 directive
        "file",                   // TP6 keyword, Delphi, FreePascal
        "finalization",           // Delphi, FreePascal
        "finally",                // Delphi, FreePascal
        "for",                    // TP6 keyword, Delphi, FreePascal
        "forward",                // TP6 directive
        "function",               // TP6 keyword, Delphi, FreePascal
        "goto",                   // TP6 keyword, Delphi, FreePascal
        "if",                     // TP6 keyword, Delphi, FreePascal
        "implementation",         // TP6 keyword, Delphi, FreePascal
        "in",                     // TP6 keyword, Delphi, FreePascal
        "inherited",              // Delphi, FreePascal
        "initialization",         // Delphi, FreePascal
        "inline",                 // TP6 keyword, Delphi, FreePascal
        "interface",              // TP6 keyword, Delphi, FreePascal
        "interrupt",              // TP6 directive
        "is",                     // Delphi, FreePascal
        "label",                  // TP6 keyword, Delphi, FreePascal
        "library",                // Delphi, FreePascal
        "mod",                    // TP6 keyword, Delphi, FreePascal
        "near",                   // TP6 directive
        "new",                    // FreePascal
        "nil",                    // TP6 keyword, Delphi, FreePascal
        "not",                    // TP6 keyword, Delphi, FreePascal
        "object",                 // TP6 keyword, Delphi, FreePascal
        "of",                     // TP6 keyword, Delphi, FreePascal
        "on",                     // FreePascal
        "operator",               // FreePascal
        "or",                     // TP6 keyword, Delphi, FreePascal
        "out",                    // Delphi, FreePascal
        "packed",                 // TP6 keyword, Delphi, FreePascal
        "private",                // TP6 directive
        "procedure",              // TP6 keyword, Delphi, FreePascal
        "program",                // TP6 keyword, Delphi, FreePascal
        "property",               // Delphi, FreePascal
        "raise",                  // Delphi, FreePascal
        "record",                 // TP6 keyword, Delphi, FreePascal
        "repeat",                 // TP6 keyword, Delphi, FreePascal
        "resourcestring",         // Delphi
        "self",                   // FreePascal
        "set",                    // TP6 keyword, Delphi, FreePascal
        "shl",                    // TP6 keyword, Delphi, FreePascal
        "shr",                    // TP6 keyword, Delphi, FreePascal
        "string",                 // TP6 keyword, Delphi, FreePascal
        "then",                   // TP6 keyword, Delphi, FreePascal
        "threadvar",              // Delphi, FreePascal
        "to",                     // TP6 keyword, Delphi, FreePascal
        "true",                   // FreePascal
        "try",                    // Delphi, FreePascal
        "type",                   // TP6 keyword, Delphi, FreePascal
        "unit",                   // TP6 keyword, Delphi, FreePascal
        "until",                  // TP6 keyword, Delphi, FreePascal
        "uses",                   // TP6 keyword, Delphi, FreePascal
        "var",                    // TP6 keyword, Delphi, FreePascal
        "virtual",                // TP6 directive
        "while",                  // TP6 keyword, Delphi, FreePascal
        "with",                   // TP6 keyword, Delphi, FreePascal
        "xor",                    // TP6 keyword, Delphi, FreePascal
    };

    bool findKeyword(const String_t& str)
    {
        size_t idx = 0;
        for (size_t i = 0x80; i > 0; i >>= 1) {
            size_t nidx = idx | i;
            if (nidx < countof(KEYWORDS) && str >= KEYWORDS[nidx]) {
                idx = nidx;
            }
        }
        return (KEYWORDS[idx] == str);
    }
}

util::syntax::PascalHighlighter::PascalHighlighter()
    : m_text()
{ }

util::syntax::PascalHighlighter::~PascalHighlighter()
{ }

void
util::syntax::PascalHighlighter::init(afl::string::ConstStringMemory_t text)
{
    m_text = text;
}

bool
util::syntax::PascalHighlighter::scan(Segment& result)
{
    // Whitespace
    result.start(m_text);
    if (skip(m_text, cWhitespace | cNewline)) {
        result.finish(DefaultFormat, m_text);
        return true;
    }

    // String '...'
    if (skip1(m_text, cQuote)) {
        skip(m_text, ~(cQuote | cNewline));
        skip1(m_text, cQuote);
        result.finish(StringFormat, m_text);
        return true;
    }

    // "//" comment?
    if (skip1(m_text, cSlash)) {
        if (skip1(m_text, cSlash)) {
            skip(m_text, ~cNewline);
            result.finish(CommentFormat, m_text);
        } else {
            result.finish(DefaultFormat, m_text);
        }
        return true;
    }

    // '{' comment?
    if (skip1(m_text, cLBrace)) {
        bool directive = isDollar(m_text);
        skip(m_text, ~cRBrace);
        skip1(m_text, cRBrace);
        result.finish(directive ? Comment2Format : CommentFormat, m_text);
        return true;
    }

    // '(*' comment?
    if (skip1(m_text, cLParen)) {
        if (skip1(m_text, cStar)) {
            bool directive = isDollar(m_text);
            while (1) {
                skip(m_text, ~cStar);
                if (!skip(m_text, cStar)) {
                    // End of file
                    break;
                }
                if (skip1(m_text, cRParen)) {
                    // End of comment
                    break;
                }
            }
            result.finish(directive ? Comment2Format : CommentFormat, m_text);
        } else {
            result.finish(DefaultFormat, m_text);
        }
        return true;
    }

    // Identifier?
    if (skip1(m_text, cLetter)) {
        skip(m_text, cLetter | cDigit);
        result.finish(DefaultFormat, m_text);
        if (findKeyword(getIdentifier(result.getText()))) {
            result.setFormat(KeywordFormat);
        }
        return true;
    }

    // Other
    if (skip(m_text, cRParen | cRBrace | cStar | cDigit | cOther)) {
        result.finish(DefaultFormat, m_text);
        return true;
    } else {
        return false;
    }
}
