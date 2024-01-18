/**
  *  \file test/interpreter/blobvaluetest.cpp
  *  \brief Test for interpreter::BlobValue
  */

#include "interpreter/blobvalue.hpp"

#include "afl/io/internalsink.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/vmio/nullsavecontext.hpp"
#include <memory>

/** Simple test. */
AFL_TEST("interpreter.BlobValue", a)
{
    // Prepare
    interpreter::BlobValue testee;
    testee.data().append(1);
    testee.data().append(2);
    testee.data().append(3);
    a.checkEqual("01. size", testee.data().size(), 3U);

    // Test clone()
    std::auto_ptr<interpreter::BlobValue> clone(testee.clone());
    a.checkNonNull("11. clone", clone.get());
    a.checkEqual("12. size", testee.data().size(), 3U);
    a.checkEqual("13. size", clone->data().size(), 3U);
    a.check("14. content", clone->data().equalContent(testee.data()));

    // Stringify
    a.checkEqual("21. toString", testee.toString(false).substr(0, 2), "#<");
    a.checkEqual("22. toString", testee.toString(false), testee.toString(true));
    a.checkEqual("23. toString", clone->toString(false), testee.toString(false));

    // Test store
    afl::io::InternalSink sink;
    interpreter::TagNode node;
    interpreter::vmio::NullSaveContext sc;
    clone->store(node, sink, sc);
    a.checkEqual("31. tag", node.tag, node.Tag_Blob);
    a.checkEqual("32. value", node.value, 3U);
    a.checkEqual("33. size", sink.getContent().size(), 3U);
    a.checkEqual("34. content", *sink.getContent().at(0), 1);
    a.checkEqual("35. content", *sink.getContent().at(1), 2);
    a.checkEqual("36. content", *sink.getContent().at(2), 3);
}
