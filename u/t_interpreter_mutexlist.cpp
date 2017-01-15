/**
  *  \file u/t_interpreter_mutexlist.cpp
  *  \brief Test for interpreter::MutexList
  */

#include "interpreter/mutexlist.hpp"

#include "t_interpreter.hpp"
#include "interpreter/mutexcontext.hpp"

/** Test destruction order. */
void
TestInterpreterMutexList::testDestruction()
{
    {
        // Destroy MutexContext first, MutexList last.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext(testee.create("foo", "bar", 0)));
        ctx.reset();
    }
    {
        // Destroy MutexList first, MutexContext last. This will abandon the mutex in the meantime.
        std::auto_ptr<interpreter::MutexContext> ctx;
        interpreter::MutexList testee;
        ctx.reset(new interpreter::MutexContext(testee.create("foo", "bar", 0)));
    }
}

