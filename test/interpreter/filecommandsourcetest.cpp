/**
  *  \file test/interpreter/filecommandsourcetest.cpp
  *  \brief Test for interpreter::FileCommandSource
  */

#include "interpreter/filecommandsource.hpp"

#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/utf8charset.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/textfile.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

/** Simple test. */
AFL_TEST("interpreter.FileCommandSource", a)
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
    a.check("01. isEOF", !testee.isEOF());
    a.checkEqual("02. getCurrentToken", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("03. getCurrentString", testee.tokenizer().getCurrentString(), "latin=\xC3\xB6");

    testee.setCharsetNew(new afl::charset::CodepageCharset(afl::charset::g_codepage437));
    testee.readNextLine();
    a.check("11. isEOF", !testee.isEOF());
    a.checkEqual("12. getCurrentToken", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("13. getCurrentString", testee.tokenizer().getCurrentString(), "cp437=\xC3\xB6");

    testee.setCharsetNew(new afl::charset::Utf8Charset());
    testee.readNextLine();
    a.check("21. isEOF", !testee.isEOF());
    a.checkEqual("22. getCurrentToken", testee.tokenizer().getCurrentToken(), interpreter::Tokenizer::tString);
    a.checkEqual("23. getCurrentString", testee.tokenizer().getCurrentString(), "utf8=\xC3\xB6");

    testee.readNextLine();
    a.check("31. isEOF", testee.isEOF());

    // Line number
    a.checkEqual("41. getLineNumber", testee.getLineNumber(), 3);

    // Error message annotation
    {
        interpreter::Error err("boom");
        a.check("51. getTrace", err.getTrace().empty());

        afl::string::NullTranslator tx;
        testee.addTraceTo(err, tx);
        a.check("61. getTrace", !err.getTrace().empty());
    }
}
