/**
  *  \file u/t_interpreter_memorycommandsource.cpp
  *  \brief Test for interpreter::MemoryCommandSource
  */

#include "interpreter/memorycommandsource.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"

/** Test default-initialisation.
    Result is empty MemoryCommandSource. */
void
TestInterpreterMemoryCommandSource::testIt()
{
    interpreter::MemoryCommandSource testee;
    testee.readNextLine();
    TS_ASSERT(testee.isEOF());
}

/** Test initialisation with single line. */
void
TestInterpreterMemoryCommandSource::testOneLine()
{
    interpreter::MemoryCommandSource testee("'a'");
    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "a");

    testee.readNextLine();
    TS_ASSERT(testee.isEOF());
}

/** Test initialisation with single line, plus addLine(). */
void
TestInterpreterMemoryCommandSource::testTwoLines()
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

/** Test setCharsetNew(). MemoryCommandSource does not support charsets. */
void
TestInterpreterMemoryCommandSource::testCharset()
{
    interpreter::MemoryCommandSource testee;
    TS_ASSERT(!testee.setCharsetNew(new afl::charset::Utf8Charset()));
}

/** Test addTraceTo. */
void
TestInterpreterMemoryCommandSource::testError()
{
    interpreter::MemoryCommandSource testee;
    interpreter::Error err("boom");
    TS_ASSERT(err.getTrace().empty());

    afl::string::NullTranslator tx;
    testee.addTraceTo(err, tx);
    TS_ASSERT(err.getTrace().empty());
}

/** Test addLines() with empty area. */
void
TestInterpreterMemoryCommandSource::testAddLinesEmpty()
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::ConstStringMemory_t());
    testee.readNextLine();
    TS_ASSERT(testee.isEOF());
}

/** Test addLines() with one line. */
void
TestInterpreterMemoryCommandSource::testAddLinesOne()
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::toMemory("a\n"));

    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "A");

    testee.readNextLine();
    TS_ASSERT(testee.isEOF());
}

/** Test addLines() with multiple lines. */
void
TestInterpreterMemoryCommandSource::testAddLinesMulti()
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::toMemory("a\nb\nc\n"));

    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "A");

    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "B");

    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "C");

    testee.readNextLine();
    TS_ASSERT(testee.isEOF());
}

