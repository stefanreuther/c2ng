/**
  *  \file util/syntax/chighlighter.hpp
  *  \brief Class util::syntax::CHighlighter
  */
#ifndef C2NG_UTIL_SYNTAX_CHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_CHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    /** Syntax highlighter for C/C++/Java/JavaScript files.
        These highlight C99, C++11, Java5, JavaScript5, with the following exceptions:
        - no trigraphs in C/C++ (e.g. "??/" would be a real backslash that could quote the quote ending a string)
        - no Unicode escapes in Java (e.g. "\u0022" would be a real quote that terminates a string)
        - no raw string literals in C++
        - comments within preprocessor statements reset highlighting
        - backslash continuation is applied to all languages */
    class CHighlighter : public Highlighter {
     public:
        /** Languages to recognize. */
        enum {
            /** Recognize C.
                Enables C keywords, preprocessor. */
            LangC = 1,

            /** Recognize C++.
                Enables C++ keywords, preprocessor. */
            LangCXX = 2,

            /** Recognize Java.
                Enables Java keywords. */
            LangJava = 4,

            /** Recognize JavaScript.
                Enables JavaScript keywords, regexps. */
            LangJavaScript = 8
        };

        /** Constructor.
            \param language Languages to recognize */
        explicit CHighlighter(int language);

        /** Destructor. */
        ~CHighlighter();

        // Highlighter:
        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        int m_language;

        afl::string::ConstStringMemory_t m_text;

        /* States */
        enum State {
            sDefault,               /* Default state */
            sIncludeFileName,       /* After #include */
            sMacroName              /* After #define */
        };
        State m_state;

        /* Modifiers. These are always set according to the current position,
           but only evaluated when applicable to the current language. */
        bool m_acceptRegexp : 1;
        bool m_acceptPreprocessor : 1;
    };

} }

#endif
