/**
  *  \file test/interpreter/memorycommandsourcetest.cpp
  *  \brief Test for interpreter::MemoryCommandSource
  */

#include "interpreter/memorycommandsource.hpp"

#include "afl/charset/utf8charset.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

/** Test default-initialisation.
    Result is empty MemoryCommandSource. */
AFL_TEST("interpreter.MemoryCommandSource:empty", a)
{
    interpreter::MemoryCommandSource testee;
    testee.readNextLine();
    a.check("01. isEOF", testee.isEOF());
}

/** Test initialisation with single line. */
AFL_TEST("interpreter.MemoryCommandSource:one-line", a)
{
    interpreter::MemoryCommandSource testee("'a'");
    testee.readNextLine();
    a.check("01. isEOF", !testee.isEOF());
    a.checkEqual("02. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("03. string", testee.tokenizer().getCurrentString(), "a");

    testee.readNextLine();
    a.check("11. isEOF", testee.isEOF());
}

/** Test initialisation with single line, plus addLine(). */
AFL_TEST("interpreter.MemoryCommandSource:two-lines", a)
{
    interpreter::MemoryCommandSource testee("'a'");
    testee.addLine("'b'");
    testee.readNextLine();
    a.check("01. isEOF", !testee.isEOF());
    a.checkEqual("02. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("03. string", testee.tokenizer().getCurrentString(), "a");

    testee.readNextLine();
    a.check("11. isEOF", !testee.isEOF());
    a.checkEqual("12. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("13. string", testee.tokenizer().getCurrentString(), "b");

    testee.readNextLine();
    a.check("21. isEOF", testee.isEOF());
}

/** Test setCharsetNew(). MemoryCommandSource does not support charsets. */
AFL_TEST("interpreter.MemoryCommandSource:setCharsetNew", a)
{
    interpreter::MemoryCommandSource testee;
    a.check("01", !testee.setCharsetNew(new afl::charset::Utf8Charset()));
}

/** Test addTraceTo. */
AFL_TEST("interpreter.MemoryCommandSource:addTraceTo", a)
{
    interpreter::MemoryCommandSource testee;
    interpreter::Error err("boom");
    a.check("01", err.getTrace().empty());

    afl::string::NullTranslator tx;
    testee.addTraceTo(err, tx);
    a.check("11", err.getTrace().empty());
}

/** Test addLines() with empty area. */
AFL_TEST("interpreter.MemoryCommandSource:addLines:empty", a)
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::ConstStringMemory_t());
    testee.readNextLine();
    a.check("01. isEOF", testee.isEOF());
}

/** Test addLines() with one line. */
AFL_TEST("interpreter.MemoryCommandSource:addLines:one", a)
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::toMemory("a\n"));

    testee.readNextLine();
    a.check("01. isEOF", !testee.isEOF());
    a.checkEqual("02. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    a.checkEqual("03. string", testee.tokenizer().getCurrentString(), "A");

    testee.readNextLine();
    a.check("11. isEOF", testee.isEOF());
}

/** Test addLines() with multiple lines. */
AFL_TEST("interpreter.MemoryCommandSource:addLines:multi", a)
{
    interpreter::MemoryCommandSource testee;
    testee.addLines(afl::string::toMemory("a\nb\nc\n"));

    testee.readNextLine();
    a.check("01. isEOF", !testee.isEOF());
    a.checkEqual("02. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    a.checkEqual("03. string", testee.tokenizer().getCurrentString(), "A");

    testee.readNextLine();
    a.check("11. isEOF", !testee.isEOF());
    a.checkEqual("12. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    a.checkEqual("13. string", testee.tokenizer().getCurrentString(), "B");

    testee.readNextLine();
    a.check("21. isEOF", !testee.isEOF());
    a.checkEqual("22. token", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tIdentifier);
    a.checkEqual("23. string", testee.tokenizer().getCurrentString(), "C");

    testee.readNextLine();
    a.check("31. isEOF", testee.isEOF());
}
