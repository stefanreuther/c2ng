/**
  *  \file u/t_util_doc_summarizingverifier.cpp
  *  \brief Test for util::doc::SummarizingVerifier
  */

#include "util/doc/summarizingverifier.hpp"

#include "t_util_doc.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/string/nulltranslator.hpp"

using afl::io::InternalTextWriter;
using afl::string::NullTranslator;
using util::doc::Index;
using util::doc::Verifier;

/** Simple test.
    A: create a SummarizingVerifier. Write one message. Write 1000 messages.
    E: message logged correctly in printMessage(). Output size does not increase significantly for multiple messages. */
void
TestUtilDocSummarizingVerifier::testIt()
{
    class Spy : public util::doc::SummarizingVerifier {
     public:
        using SummarizingVerifier::reportMessage;
    };
    Spy testee;

    // Message not present
    TS_ASSERT(!testee.hasMessage(Verifier::Warn_UnresolvableContent));

    // Write a message
    Index idx;
    testee.reportMessage(Verifier::Warn_UnresolvableContent, idx, idx.root(), "xyzzy");

    // Print message
    {
        NullTranslator tx;
        InternalTextWriter out;
        testee.printMessage(Verifier::Warn_UnresolvableContent, idx, true, tx, out);
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                         "Warning: node content cannot be resolved (blob does not exist)\n"
                         "  xyzzy\n");
    }

    // Same thing, non-brief
    {
        NullTranslator tx;
        InternalTextWriter out;
        testee.printMessage(Verifier::Warn_UnresolvableContent, idx, false, tx, out);
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                         "Warning: node content cannot be resolved (blob does not exist)\n"
                         "  (root): xyzzy\n");
    }

    // Write many more messages; output size does not increase
    for (int i = 0; i < 1000; ++i) {
        testee.reportMessage(Verifier::Warn_UnresolvableContent, idx, idx.root(), "narf");
    }
    {
        NullTranslator tx;
        InternalTextWriter out;
        testee.printMessage(Verifier::Warn_UnresolvableContent, idx, false, tx, out);
        TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()),
                         "Warning: node content cannot be resolved (blob does not exist)\n"
                         "  (root) (+999): narf\n"
                         "  (root): xyzzy\n");
    }
}

