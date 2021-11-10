/**
  *  \file u/t_interpreter_blobvalue.cpp
  *  \brief Test for interpreter::BlobValue
  */

#include <memory>
#include "interpreter/blobvalue.hpp"

#include "t_interpreter.hpp"
#include "afl/io/internalsink.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"

/** Simple test. */
void
TestInterpreterBlobValue::testIt()
{
    // Prepare
    interpreter::BlobValue testee;
    testee.data().append(1);
    testee.data().append(2);
    testee.data().append(3);
    TS_ASSERT_EQUALS(testee.data().size(), 3U);

    // Test clone()
    std::auto_ptr<interpreter::BlobValue> clone(testee.clone());
    TS_ASSERT(clone.get() != 0);
    TS_ASSERT_EQUALS(testee.data().size(), 3U);
    TS_ASSERT_EQUALS(clone->data().size(), 3U);
    TS_ASSERT(clone->data().equalContent(testee.data()));

    // Stringify
    TS_ASSERT_EQUALS(testee.toString(false).substr(0, 2), "#<");
    TS_ASSERT_EQUALS(testee.toString(false), testee.toString(true));
    TS_ASSERT_EQUALS(clone->toString(false), testee.toString(false));

    // Test store
    afl::io::InternalSink sink;
    interpreter::TagNode node;
    interpreter::vmio::NullSaveContext sc;
    clone->store(node, sink, sc);
    TS_ASSERT_EQUALS(node.tag, node.Tag_Blob);
    TS_ASSERT_EQUALS(node.value, 3U);
    TS_ASSERT_EQUALS(sink.getContent().size(), 3U);
    TS_ASSERT_EQUALS(*sink.getContent().at(0), 1);
    TS_ASSERT_EQUALS(*sink.getContent().at(1), 2);
    TS_ASSERT_EQUALS(*sink.getContent().at(2), 3);
}

