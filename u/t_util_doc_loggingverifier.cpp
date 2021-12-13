/**
  *  \file u/t_util_doc_loggingverifier.cpp
  *  \brief Test for util::doc::LoggingVerifier
  */

#include "util/doc/loggingverifier.hpp"

#include "t_util_doc.hpp"
#include "afl/io/internaltextwriter.hpp"
#include "afl/string/nulltranslator.hpp"

using afl::io::InternalTextWriter;
using afl::io::TextWriter;
using afl::string::NullTranslator;
using afl::string::Translator;
using util::doc::Index;
using util::doc::Verifier;

/** Simple test.
    A: Create a LoggingVerifier. Write a message.
    E: Text arrives on the TextWriter; refers to the given content */
void
TestUtilDocLoggingVerifier::testIt()
{
    class Spy : public util::doc::LoggingVerifier {
     public:
        Spy(Translator& tx, TextWriter& out)
            : LoggingVerifier(tx, out)
            { }

        using LoggingVerifier::reportMessage;
    };

    NullTranslator tx;
    InternalTextWriter out;
    Spy testee(tx, out);

    // Initially empty
    TS_ASSERT(out.getContent().empty());

    // Write a message
    Index idx;
    testee.reportMessage(Verifier::Warn_UnresolvableContent, idx, idx.root(), "xyzzy");

    // Message is now present
    TS_ASSERT_EQUALS(afl::string::fromMemory(out.getContent()), "(root): Warning: node content cannot be resolved (blob does not exist): xyzzy\n");
}

