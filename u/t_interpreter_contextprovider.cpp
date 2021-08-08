/**
  *  \file u/t_interpreter_contextprovider.cpp
  *  \brief Test for interpreter::ContextProvider
  */

#include "interpreter/contextprovider.hpp"

#include "t_interpreter.hpp"

/** Interface test. */
void
TestInterpreterContextProvider::testInterface()
{
    class Tester : public interpreter::ContextProvider {
     public:
        virtual interpreter::Context::PropertyAccessor* lookup(const afl::data::NameQuery& /*q*/, interpreter::Context::PropertyIndex_t& /*index*/)
            { return 0; }
    };
    Tester t;
}

