/**
  *  \file util/syntax/pascalhighlighter.hpp
  *  \brief Class util::syntax::PascalHighlighter
  */
#ifndef C2NG_UTIL_SYNTAX_PASCALHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_PASCALHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    /** Pascal highlighter.
        Implements the following rules:
        - comments are either in {...} or (*...*) pairs; tokens do not mix (as in Borland dialects)
        - single-line comments starting with "//" (as in Delphi)
        - comments starting with '$' are directives
        - highlights a bunch of keywords from Turbo/Borland/Delphi/FreePascal dialects.
          Note that things like INTEGER or POINTER are not keywords. */
    class PascalHighlighter : public Highlighter {
     public:
        PascalHighlighter();
        ~PascalHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        afl::string::ConstStringMemory_t m_text;
    };

} }

#endif
