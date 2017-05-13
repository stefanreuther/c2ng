/**
  *  \file u/t_interpreter_memorycommandsource.cpp
  *  \brief Test for interpreter::MemoryCommandSource
  */

#include "interpreter/memorycommandsource.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"

/** Simple test. */
void
TestInterpreterMemoryCommandSource::testIt()
{
    // Empty
    {
        interpreter::MemoryCommandSource testee;
        testee.readNextLine();
        TS_ASSERT(testee.isEOF());
    }

    // One line
    {
        interpreter::MemoryCommandSource testee("'a'");
        testee.readNextLine();
        TS_ASSERT(!testee.isEOF());
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "a");

        testee.readNextLine();
        TS_ASSERT(testee.isEOF());
    }

    // Two lines
    {
        interpreter::MemoryCommandSource testee("'a'");
        testee.addLine("'b'");
        testee.readNextLine();
        TS_ASSERT(!testee.isEOF());
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "a");

        testee.readNextLine();
        TS_ASSERT(!testee.isEOF());
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
        TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "b");

        testee.readNextLine();
        TS_ASSERT(testee.isEOF());
    }

    // Charset
    {
        interpreter::MemoryCommandSource testee;
        TS_ASSERT(!testee.setCharsetNew(new afl::charset::Utf8Charset()));
    }

    // Error
    {
        interpreter::MemoryCommandSource testee;
        interpreter::Error err("boom");
        TS_ASSERT(err.getTrace().empty());

        afl::string::NullTranslator tx;
        testee.addTraceTo(err, tx);
        TS_ASSERT(err.getTrace().empty());
    }
}

