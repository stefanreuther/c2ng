/**
  *  \file util/syntax/chighlighter.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_CHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_CHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    /** Syntax highlighter for C/C++/Java/JavaScript files.
        These highlight C99, C++11, Java5, JavaScript5, with the following exceptions:
        - no trigraphs in C/C++ (e.g. "??/" would be a real backslash that could quote the quote ending a string)
        - no Unicode escapes in Java (e.g. "\u0022" is a real quote that terminates a string)
        - no raw string literals in C++
        - comments within preprocessor statements reset highlighting
        - backslash continuation is applied to all languages */
    class CHighlighter : public Highlighter {
     public:
        enum {
            LangC          = 1,
            LangCXX        = 2,
            LangJava       = 4,
            LangJavaScript = 8
        };

        CHighlighter(int language);
        ~CHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        int language;

        afl::string::ConstStringMemory_t m_text;

        /* States */
        enum State {
            sDefault,               /* Default state */
            sIncludeFileName,       /* After #include */
            sMacroName              /* After #define */
        };
        State state;

        /* Modifiers. These are always set according to the current position,
           but only evaluated when applicable to the current language. */
        bool accept_regexp;
        bool accept_preproc;
    };

} }

#endif
