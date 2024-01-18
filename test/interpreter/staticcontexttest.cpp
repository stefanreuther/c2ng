/**
  *  \file test/interpreter/staticcontexttest.cpp
  *  \brief Test for interpreter::StaticContext
  */

#include "interpreter/staticcontext.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("interpreter.StaticContext")
{
    class Tester : public interpreter::StaticContext {
     public:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& /*q*/, interpreter::Context::PropertyIndex_t& /*index*/) const
            { return 0; }
    };
    Tester t;
}
