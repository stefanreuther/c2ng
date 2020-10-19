/**
  *  \file util/syntax/chighlighter.cpp
  *  \brief Class util::syntax::CHighlighter
  */

#include "util/syntax/chighlighter.hpp"
#include "util/syntax/segment.hpp"
#include "afl/base/countof.hpp"

namespace {
    enum Class {
        cWhitespace = 1,        // horizontal whitespace
        cNewline    = 2,        // newline
        cSlash      = 4,        // slash
        cStar       = 8,        // *
        cHash       = 16,       // #
        cSQuote     = 32,       // '
        cLetter     = 64,       // alpha or _
        cDigit      = 128,      // 0-9
        cPunct      = 256,      // punctuation that can appear before a regexp: binary operators, "("
        cOther      = 512,      // punctuation
        cDQuote     = 1024      // "
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

         case '/':
            return cSlash;

         case '*':
            return cStar;

         case '#':
            return cHash;

         case '"':
            return cDQuote;

         case '\'':
            return cSQuote;

         case '[':
         case '(':
         case ',':
         case '=':
         case ':':
         case '!':
         case '&':
         case '|':
         case '?':
         case '{':
         case '}':
         case ';':
         case '-':
         case '+':
         case '<':
         case '>':
         case '~':
            return cPunct;

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

    void skipContinuation(afl::string::ConstStringMemory_t& text)
    {
        while (text.size() >= 2
               && *text.at(0) == '\\'
               && (*text.at(1) == '\r' || *text.at(1) == '\n'))
        {
            if (*text.at(1) == '\r' && text.size() >= 3 && *text.at(2) == '\n') {
                text.split(3);
            } else {
                text.split(2);
            }
        }
    }

    bool skip(afl::string::ConstStringMemory_t& text, int c)
    {
        bool result = false;
        const char* p;
        while ((p = text.at(0)) != 0 && (classify(*p) & c) != 0) {
            text.eat();
            skipContinuation(text);
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
        skipContinuation(text);
        while (const char* p = text.eat()) {
            result += *p;
            skipContinuation(text);
        }
        return result;
    }

    void skipRegexp(afl::string::ConstStringMemory_t& text)
    {
        bool quote = false;
        bool bracket = false;
        while (const char* p = text.eat()) {
            char ch = *p;
            if (quote) {
                quote = false;
            } else if (ch == '\\') {
                quote = true;
            } else if (ch == '[') {
                bracket = true;
            } else if (ch == ']') {
                bracket = false;
            } else if (bracket || ch != '/') {
                // part of regexp
            } else {
                break;
            }
        }
    }

    void skipString(afl::string::ConstStringMemory_t& text, char delim)
    {
        bool quoted = false;
        skipContinuation(text);
        while (const char* p = text.eat()) {
            char ch = *p;
            skipContinuation(text);
            if (quoted) {
                quoted = false;
            } else if (ch == '\\') {
                quoted = true;
            } else if (ch != delim) {
                quoted = false;
            } else {
                break;
            }
        }
    }


    /*
     *  Keyword list
     *
     *  Keywords have been taken from
     *    C99 (iso-9899-1999.pdf)
     *    C++11 (n3242.pdf)
     *    Java5 (langspec-3.0.pdf)
     *    ES5 (ECMA-262 (ECMAScript 5th edition).pdf)
     */
    using util::syntax::CHighlighter;
    struct Keyword {
        const char* word;
        int where;
    };
    static const Keyword keywords[] = {
        { "_Bool",            CHighlighter::LangC },                                                                // C99
        { "_Complex",         CHighlighter::LangC  },                                                               // C99
        { "_Imaginary",       CHighlighter::LangC   },                                                              // C99
        { "abstract",         CHighlighter::LangJava  },                                                            // Java5
        { "alignas",          CHighlighter::LangCXX },                                                              // C++11
        { "alignof",          CHighlighter::LangCXX },                                                              // C++11
        { "and",              CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "and_eq",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "asm",              CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (but common on C)
        { "assert",           CHighlighter::LangC | CHighlighter::LangCXX | CHighlighter::LangJava },               // Java5
        { "auto",             CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11, C99
        { "bitand",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "bitor",            CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "bool",             CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "boolean",          CHighlighter::LangJava },                                                             // Java5
        { "break",            -1 },                                                                                 // common
        { "byte",             CHighlighter::LangJava },                                                             // Java5
        { "case",             -1 },                                                                                 // common
        { "catch",            CHighlighter::LangCXX | CHighlighter::LangJava | CHighlighter::LangJavaScript },      // C++11, ES5, Java5
        { "char",             CHighlighter::LangC | CHighlighter::LangCXX | CHighlighter::LangJava },               // C++11, C99, Java5
        { "char16_t",         CHighlighter::LangCXX  },                                                             // C++11
        { "char32_t",         CHighlighter::LangCXX  },                                                             // C++11
        { "class",            CHighlighter::LangCXX | CHighlighter::LangJava | CHighlighter::LangJavaScript },      // C++11, ES5 (future), Java5
        { "compl",            CHighlighter::LangCXX },                                                              // C++11
        { "complex",          CHighlighter::LangC },                                                                // C99 (pseudo)
        { "const",            -1 },                                                                                 // common (future reserved word in ES5)
        { "const_cast",       CHighlighter::LangCXX },                                                              // C++11
        { "constexpr",        CHighlighter::LangCXX  },                                                             // C++11
        { "continue",         -1 },                                                                                 // common
        { "debugger",         CHighlighter::LangJavaScript },                                                       // ES5
        { "decltype",         CHighlighter::LangCXX },                                                              // C++11
        { "default",          -1 },                                                                                 // common
        { "delete",           CHighlighter::LangCXX | CHighlighter::LangJavaScript },                               // C++11
        { "do",               -1 },                                                                                 // common
        { "double",           CHighlighter::LangCXX | CHighlighter::LangC | CHighlighter::LangJava },               // C++11, C99, Java5
        { "dynamic_cast",     CHighlighter::LangCXX },                                                              // C++11
        { "else",             -1 },                                                                                 // common
        { "enum",             -1 },                                                                                 // common (future reserved word in ES5)
        { "explicit",         CHighlighter::LangCXX },                                                              // C++11
        { "export",           CHighlighter::LangCXX | CHighlighter::LangJavaScript },                               // C++11, ES5 (future)
        { "extends",          CHighlighter::LangJavaScript | CHighlighter::LangJava },                              // ES5 (future), Java5
        { "extern",           CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11, C99
        { "false",            -1 },                                                                                 // common (macro in C)
        { "final",            CHighlighter::LangJava },                                                             // Java5
        { "finally",          CHighlighter::LangJava | CHighlighter::LangJavaScript },                              // ES5, Java5
        { "float",            CHighlighter::LangCXX | CHighlighter::LangC | CHighlighter::LangJava },               // C++11, C99, Java5
        { "for",              -1 },                                                                                 // common
        { "friend",           CHighlighter::LangCXX },                                                              // C++11
        { "function",         CHighlighter::LangJavaScript },                                                       // ES5
        { "goto",             CHighlighter::LangC | CHighlighter::LangCXX | CHighlighter::LangJava },               // C++11, C99, Java5
        { "if",               -1 },                                                                                 // common
        { "implements",       CHighlighter::LangJava | CHighlighter::LangJavaScript  },                             // ES5 (future strict), Java5
        { "import",           CHighlighter::LangJava | CHighlighter::LangJavaScript },                              // ES5 (future), Java5
        { "in",               CHighlighter::LangJavaScript },                                                       // ES5
        { "inline",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11, C99
        { "instanceof",       CHighlighter::LangJavaScript | CHighlighter::LangJava },                              // ES5, Java5
        { "int",              CHighlighter::LangC | CHighlighter::LangCXX | CHighlighter::LangJava },               // C++11, C99, Java5
        { "interface",        CHighlighter::LangJavaScript | CHighlighter::LangJava },                              // ES5 (future strict), Java5
        { "let",              CHighlighter::LangJavaScript },                                                       // ES5 (future strict)
        { "long",             CHighlighter::LangCXX | CHighlighter::LangC | CHighlighter::LangJava },               // C++11, C99, Java5
        { "mutable",          CHighlighter::LangCXX },                                                              // C++11
        { "namespace",        CHighlighter::LangCXX },                                                              // C++11
        { "native",           CHighlighter::LangJava },                                                             // Java5
        { "new",              CHighlighter::LangCXX | CHighlighter::LangJavaScript | CHighlighter::LangJava },      // C++11, ES5, Java5
        { "noexcept",         CHighlighter::LangCXX },                                                              // C++11
        { "not",              CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "not_eq",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "null",             CHighlighter::LangJava | CHighlighter::LangJavaScript },                              // ES5
        { "nullptr",          CHighlighter::LangCXX },                                                              // C++11
        { "operator",         CHighlighter::LangCXX },                                                              // C++11
        { "or",               CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "or_eq",            CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "package",          CHighlighter::LangJava | CHighlighter::LangJavaScript },                              // ES5 (future strict), Java5
        { "private",          CHighlighter::LangCXX | CHighlighter::LangJavaScript | CHighlighter::LangJava },      // C++11, ES5 (future strict), Java5
        { "protected",        CHighlighter::LangCXX | CHighlighter::LangJavaScript | CHighlighter::LangJava },      // C++11, ES5 (future strict), Java5
        { "public",           CHighlighter::LangCXX | CHighlighter::LangJavaScript | CHighlighter::LangJava },      // C++11, ES5 (future strict), Java5
        { "register",         CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11,C99
        { "reinterpret_cast", CHighlighter::LangCXX },                                                              // C++11
        { "restrict",         CHighlighter::LangC },                                                                // C99
        { "return",           -1 },                                                                                 // common
        { "short",            CHighlighter::LangCXX | CHighlighter::LangC | CHighlighter::LangJava },               // C++11, C99, Java5
        { "signed",           CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11, C99
        { "sizeof",           CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11, C99
        { "static",           -1 },                                                                                 // common (future strict reserved word in ES5)
        { "static_assert",    CHighlighter::LangCXX },                                                              // C++11
        { "static_cast",      CHighlighter::LangCXX },                                                              // C++11
        { "strictfp",         CHighlighter::LangJava },                                                             // Java5
        { "struct",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11, C99
        { "super",            CHighlighter::LangJava | CHighlighter::LangJavaScript },                              // ES5 (future), Java5
        { "switch",           -1 },                                                                                 // common
        { "synchronized",     CHighlighter::LangJava },                                                             // Java5
        { "template",         CHighlighter::LangCXX },                                                              // C++11
        { "this",             CHighlighter::LangCXX | CHighlighter::LangJava | CHighlighter::LangJavaScript },      // C++11, ES5, Java5
        { "thread_local",     CHighlighter::LangCXX },                                                              // C++11
        { "throw",            CHighlighter::LangCXX | CHighlighter::LangJava | CHighlighter::LangJavaScript },      // C++11, ES5, Java5
        { "throws",           CHighlighter::LangJava },                                                             // Java5
        { "transient",        CHighlighter::LangJava },                                                             // Java5
        { "true",             -1 },                                                                                 // common (macro in C)
        { "try",              CHighlighter::LangCXX | CHighlighter::LangJavaScript | CHighlighter::LangJava },      // C++11, ES5, Java5
        { "typedef",          CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11, C99
        { "typeid",           CHighlighter::LangCXX },                                                              // C++11
        { "typename",         CHighlighter::LangCXX },                                                              // C++11
        { "typeof",           CHighlighter::LangJavaScript },                                                       // ES5
        { "union",            CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11, C99
        { "unsigned",         CHighlighter::LangCXX | CHighlighter::LangC },                                        // C++11
        { "using",            CHighlighter::LangCXX },                                                              // C++11
        { "var",              CHighlighter::LangJavaScript },                                                       // ES5
        { "virtual",          CHighlighter::LangCXX },                                                              // C++11
        { "void",             -1 },                                                                                 // common
        { "volatile",         CHighlighter::LangCXX | CHighlighter::LangC | CHighlighter::LangJava },               // C++11, C99, Java5
        { "wchar_t",          CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (typedef in C)
        { "while",            -1 },                                                                                 // common
        { "with",             CHighlighter::LangJavaScript },                                                       // ES5
        { "xor",              CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "xor_eq",           CHighlighter::LangC | CHighlighter::LangCXX },                                        // C++11 (macro in C)
        { "yield",            CHighlighter::LangJavaScript },                                                       // ES5 (future strict)
    };

    int findKeyword(const String_t str)
    {
        size_t idx = 0;
        for (size_t i = 0x80; i > 0; i >>= 1) {
            size_t nidx = idx | i;
            if (nidx < countof(keywords) && str >= keywords[nidx].word) {
                idx = nidx;
            }
        }
        if (keywords[idx].word == str) {
            return keywords[idx].where;
        } else {
            return 0;
        }
    }
}

util::syntax::CHighlighter::CHighlighter(int language)
    : m_language(language),
      m_text(),
      m_state(sDefault),
      m_acceptRegexp(false),
      m_acceptPreprocessor(true)
{
    // ex SyntaxC::SyntaxC
}

util::syntax::CHighlighter::~CHighlighter()
{ }

void
util::syntax::CHighlighter::init(afl::string::ConstStringMemory_t text)
{
    // ex SyntaxC::init
    m_text = text;
    m_state = sDefault;
    m_acceptRegexp = false;
    m_acceptPreprocessor = true;
}

bool
util::syntax::CHighlighter::scan(Segment& result)
{
    // ex SyntaxC::scan

    // Whitespace always accepted
    result.start(m_text);
    if (skip(m_text, cWhitespace)) {
        result.finish(DefaultFormat, m_text);
        return true;
    }

    // Special states
    if (m_state == sMacroName) {
        if (skip(m_text, cLetter)) {
            skip(m_text, cLetter | cDigit);
            result.finish(NameFormat, m_text);
            m_state              = sDefault;
            m_acceptRegexp       = false;
            m_acceptPreprocessor = false;
            return true;
        } else {
            m_state              = sDefault;
            m_acceptRegexp       = false;
            m_acceptPreprocessor = false;
        }
    }

    if (m_state == sIncludeFileName) {
        const char* p = m_text.at(0);
        if (p != 0 && *p == '<') {
            while ((p = m_text.at(0)) != 0 && classify(*p) != cNewline && *p != '>') {
                m_text.eat();
                skipContinuation(m_text);
            }
            if ((p = m_text.at(0)) != 0 && *p == '>') {
                m_text.eat();
                skipContinuation(m_text);
            }
            result.finish(StringFormat, m_text);
            m_state              = sDefault;
            m_acceptRegexp       = false;
            m_acceptPreprocessor = false;
            return true;
        } else {
            m_state              = sDefault;
            m_acceptRegexp       = false;
            m_acceptPreprocessor = false;
        }
    }

    // Default state
    if (skip(m_text, cNewline)) {
        // Newline
        result.finish(DefaultFormat, m_text);
        m_acceptPreprocessor = true;
        return true;
    } else if (skip1(m_text, cSlash)) {
        // Slash. Could be comment or regexp.
        skipContinuation(m_text);
        if (skip1(m_text, cSlash)) {
            // "//" comment
            skipContinuation(m_text);
            skip(m_text, ~cNewline);
            result.finish(CommentFormat, m_text);
            return true;
        } else if (skip1(m_text, cStar)) {
            // "/*" comment
            skipContinuation(m_text);
            while (1) {
                skip(m_text, ~cStar);
                if (!skip(m_text, cStar)) {
                    // End of file reached
                    break;
                }
                if (skip1(m_text, cSlash)) {
                    // End of comment
                    skipContinuation(m_text);
                    break;
                }
            }
            result.finish(CommentFormat, m_text);
            return true;
        } else if (m_acceptRegexp && (m_language & LangJavaScript) != 0) {
            // Regexp
            skipRegexp(m_text);
            result.finish(StringFormat, m_text);
            m_acceptRegexp       = false;
            m_acceptPreprocessor = false;
            return true;
        } else {
            // Just a slash
            result.finish(DefaultFormat, m_text);
            m_acceptRegexp       = true;
            m_acceptPreprocessor = false;
            return true;
        }
    } else if (m_acceptPreprocessor && (m_language & (LangC | LangCXX)) != 0 && skip1(m_text, cHash)) {
        // Preprocessor
        skipContinuation(m_text);
        skip(m_text, cWhitespace);
        Segment mark;
        mark.start(m_text);
        skip(m_text, cLetter | cDigit);
        mark.finish(DefaultFormat, m_text);
        String_t ident = getIdentifier(mark.getText());
        if (ident == "ifdef" || ident == "undef" || ident == "define" || ident == "ifndef") {
            m_state = sMacroName;
        } else if (ident == "include" || ident == "import") {
            m_state = sIncludeFileName;
        } else {
            // keep state
        }
        result.finish(SectionFormat, m_text);
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip1(m_text, cSQuote)) {
        skipString(m_text, '\'');
        result.finish(StringFormat, m_text);
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip1(m_text, cDQuote)) {
        skipString(m_text, '"');
        result.finish(StringFormat, m_text);
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip(m_text, cDigit)) {
        // Number
        result.finish(DefaultFormat, m_text);
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip(m_text, cLetter)) {
        // Identifier
        skip(m_text, cLetter | cDigit);
        result.finish(DefaultFormat, m_text);
        if ((findKeyword(getIdentifier(result.getText())) & m_language) != 0) {
            result.setFormat(KeywordFormat);
        }
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip(m_text, cPunct | cStar)) {
        // Punctuation
        result.finish(DefaultFormat, m_text);
        m_acceptRegexp       = true;
        m_acceptPreprocessor = false;
        return true;
    } else if (skip(m_text, cOther | cHash)) {
        // More punctuation
        result.finish(DefaultFormat, m_text);
        m_acceptRegexp       = false;
        m_acceptPreprocessor = false;
        return true;
    } else {
        return false;
    }
}
