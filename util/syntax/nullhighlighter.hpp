/**
  *  \file util/syntax/nullhighlighter.hpp
  */
#ifndef C2NG_UTIL_SYNTAX_NULLHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_NULLHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    class NullHighlighter : public Highlighter {
     public:
        NullHighlighter();
        ~NullHighlighter();

        virtual void init(afl::string::ConstStringMemory_t text);
        virtual bool scan(Segment& result);

     private:
        afl::string::ConstStringMemory_t m_text;
    };

} }

#endif
