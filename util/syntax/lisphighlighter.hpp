/**
  *  \file util/syntax/lisphighlighter.hpp
  *  \brief Class util::syntax::LispHighlighter
  */
#ifndef C2NG_UTIL_SYNTAX_LISPHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_LISPHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    /** Lisp highlighter.
        Implements the following rules:
        - comments start with ";"
        - strings enclosed in "", with no restrictions (i.e. can span multiple lines)
        - "\" quotes everything */
    class LispHighlighter : public Highlighter {
     public:
        LispHighlighter();
        ~LispHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        afl::string::ConstStringMemory_t m_text;
    };

} }

#endif
