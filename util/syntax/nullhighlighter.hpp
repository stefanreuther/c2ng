/**
  *  \file util/syntax/nullhighlighter.hpp
  *  \brief Class util::syntax::NullHighlighter
  */
#ifndef C2NG_UTIL_SYNTAX_NULLHIGHLIGHTER_HPP
#define C2NG_UTIL_SYNTAX_NULLHIGHLIGHTER_HPP

#include "util/syntax/highlighter.hpp"

namespace util { namespace syntax {

    /** Null highlighter.
        Returns the entire string as one DefaultFormat chunk. */
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
