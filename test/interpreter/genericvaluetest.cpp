/**
  *  \file test/interpreter/genericvaluetest.cpp
  *  \brief Test for interpreter::GenericValue
  */

#include "interpreter/genericvalue.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"
#include "interpreter/tagnode.hpp"
#include "interpreter/test/valueverifier.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("interpreter.GenericValue", a)
{
    // Simple methods
    interpreter::GenericValue<int> testee(42);
    a.checkEqual("01. get", testee.get(), 42);
    a.checkEqual("02. toString", testee.toString(false), "#<builtin>");
    a.checkEqual("03. toString", testee.toString(true), "#<builtin>");

    interpreter::test::ValueVerifier verif(testee, a);
    verif.verifyBasics();
    verif.verifyNotSerializable();

    // Clone, receiving base class
    std::auto_ptr<interpreter::BaseValue> c1(testee.clone());
    a.checkEqual("11. clone", c1->toString(false), "#<builtin>");

    // Clone, receiving derived class
    std::auto_ptr<interpreter::GenericValue<int> > c2(testee.clone());
    a.checkEqual("21. get", c2->get(), 42);
}
