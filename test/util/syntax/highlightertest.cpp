/**
  *  \file test/util/syntax/highlightertest.cpp
  *  \brief Test for util::syntax::Highlighter
  */

#include "util/syntax/highlighter.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("util.syntax.Highlighter")
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
