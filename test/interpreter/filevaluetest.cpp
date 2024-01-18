/**
  *  \file test/interpreter/filevaluetest.cpp
  *  \brief Test for interpreter::FileValue
  */

#include "interpreter/filevalue.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("interpreter.FileValue", a)
{
    // Test object
    interpreter::FileValue testee(42);
    a.checkEqual("01. toString", testee.toString(false), "#42");
    a.checkEqual("02. getFileNumber", testee.getFileNumber(), 42);

    // Test clone()
    std::auto_ptr<interpreter::FileValue> copy(testee.clone());
    a.checkNonNull("11. clone", copy.get());
    a.checkEqual("12. getFileNumber", copy->getFileNumber(), 42);

    // Test store
    afl::io::InternalSink sink;
    interpreter::TagNode node;
    interpreter::vmio::NullSaveContext sc;
    copy->store(node, sink, sc);
    a.checkEqual("21. tag", node.tag, node.Tag_FileHandle);
    a.checkEqual("22. value", node.value, 42U);
}
