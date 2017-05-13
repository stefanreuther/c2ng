/**
  *  \file u/t_interpreter_filevalue.cpp
  *  \brief Test for interpreter::FileValue
  */

#include <memory>
#include "interpreter/filevalue.hpp"

#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include "afl/charset/utf8charset.hpp"

/** Simple test. */
void
TestInterpreterFileValue::testIt()
{
    // Test object
    interpreter::FileValue testee(42);
    TS_ASSERT_EQUALS(testee.toString(false), "#42");
    TS_ASSERT_EQUALS(testee.getFileNumber(), 42);

    // Test clone()
    std::auto_ptr<interpreter::FileValue> copy(testee.clone());
    TS_ASSERT(copy.get() != 0);
    TS_ASSERT_EQUALS(copy->getFileNumber(), 42);

    // Test store
    afl::io::InternalSink sink;
    interpreter::TagNode node;
    interpreter::vmio::NullSaveContext sc;
    afl::charset::Utf8Charset cs;
    copy->store(node, sink, cs, sc);
    TS_ASSERT_EQUALS(node.tag, node.Tag_FileHandle);
    TS_ASSERT_EQUALS(node.value, 42U);
}
