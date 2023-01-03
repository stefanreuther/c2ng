/**
  *  \file u/t_interpreter_staticcontext.cpp
  *  \brief Test for interpreter::StaticContext
  */

#include "interpreter/staticcontext.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterStaticContext::testInterface()
{
    class Tester : public interpreter::StaticContext {
     public:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& /*q*/, interpreter::Context::PropertyIndex_t& /*index*/) const
            { return 0; }
    };
    Tester t;
}

