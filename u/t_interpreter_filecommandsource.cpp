/**
  *  \file u/t_interpreter_filecommandsource.cpp
  *  \brief Test for interpreter::FileCommandSource
  */

#include "interpreter/filecommandsource.hpp"

#include "t_interpreter.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "interpreter/error.hpp"

/** Simple test. */
void
TestInterpreterFileCommandSource::testIt()
{
    // Each of these lines contains LATIN SMALL LETTER O WITH DIARRHOE.
    afl::io::ConstMemoryStream ms(afl::string::toBytes("'latin=\xF6'\n"
                                                       "'cp437=\x94'\n"
                                                       "'utf8=\xC3\xB6'\n"));
    afl::io::TextFile tf(ms);
    interpreter::FileCommandSource testee(tf);

    // Test
    // - latin 1 is TextFile's default
    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "latin=\xC3\xB6");

    testee.setCharsetNew(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "cp437=\xC3\xB6");

    testee.setCharsetNew(new afl::charset::Utf8Charset());
    testee.readNextLine();
    TS_ASSERT(!testee.isEOF());
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    TS_ASSERT_EQUALS(testee.tokenizer().getCurrentString(), "utf8=\xC3\xB6");

    testee.readNextLine();
    TS_ASSERT(testee.isEOF());

    // Line number
    TS_ASSERT_EQUALS(testee.getLineNumber(), 3);

    // Error message annotation
    {
        interpreter::Error err("boom");
        TS_ASSERT(err.getTrace().empty());

        afl::string::NullTranslator tx;
        testee.addTraceTo(err, tx);
        TS_ASSERT(!err.getTrace().empty());
    }
}
