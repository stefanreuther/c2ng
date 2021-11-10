/**
  *  \file u/t_interpreter_genericvalue.cpp
  *  \brief Test for interpreter::GenericValue
  */

#include "interpreter/genericvalue.hpp"

#include <memory>
#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

/** Simple test. */
void
TestInterpreterGenericValue::testIt()
{
    // Simple methods
    interpreter::GenericValue<int> testee(42);
    TS_ASSERT_EQUALS(testee.get(), 42);
    TS_ASSERT_EQUALS(testee.toString(false), "#<builtin>");
    TS_ASSERT_EQUALS(testee.toString(true), "#<builtin>");

    // Store
    {
        interpreter::TagNode n;
        afl::io::InternalSink sink;
        interpreter::vmio::NullSaveContext sc;
        TS_ASSERT_THROWS(testee.store(n, sink, sc), interpreter::Error);
    }

    // Clone, receiving base class
    std::auto_ptr<interpreter::BaseValue> c1(testee.clone());
    TS_ASSERT_EQUALS(c1->toString(false), "#<builtin>");

    // Clone, receiving derived class
    std::auto_ptr<interpreter::GenericValue<int> > c2(testee.clone());
    TS_ASSERT_EQUALS(c2->get(), 42);
}

