/**
  *  \file u/t_util_syntax_highlighter.cpp
  *  \brief Test for util::syntax::Highlighter
  */

#include "util/syntax/highlighter.hpp"

#include "t_util_syntax.hpp"

/** Interface test. */
void
TestUtilSyntaxHighlighter::testInterface()
{
    class Tester : public util::syntax::Highlighter {
     public:
        virtual void init(afl::string::ConstStringMemory_t /*text*/)
            { }
        virtual bool scan(util::syntax::Segment& /*result*/)
            { return false; }
    };
    Tester t;
}

