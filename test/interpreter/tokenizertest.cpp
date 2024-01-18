/**
  *  \file test/interpreter/tokenizertest.cpp
  *  \brief Test for interpreter::Tokenizer
  */

#include "interpreter/tokenizer.hpp"

#include "afl/base/countof.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/error.hpp"

AFL_TEST("interpreter.Tokenizer:basics", a)
{
    // ex IntLexTestSuite::testTokenizer
    // All single tokens
    {
        interpreter::Tokenizer tok(" & # + - * / \\ ^ ( ) , = < > : ; . % ");
        a.check("01", tok.checkAdvance(tok.tAmpersand));
        a.check("02", tok.checkAdvance(tok.tHash));
        a.check("03", tok.checkAdvance(tok.tPlus));
        a.check("04", tok.checkAdvance(tok.tMinus));
        a.check("05", tok.checkAdvance(tok.tMultiply));
        a.check("06", tok.checkAdvance(tok.tSlash));
        a.check("07", tok.checkAdvance(tok.tBackslash));
        a.check("08", tok.checkAdvance(tok.tCaret));
        a.check("09", tok.checkAdvance(tok.tLParen));
        a.check("10", tok.checkAdvance(tok.tRParen));
        a.check("11", tok.checkAdvance(tok.tComma));
        a.check("12", tok.checkAdvance(tok.tEQ));
        a.check("13", tok.checkAdvance(tok.tLT));
        a.check("14", tok.checkAdvance(tok.tGT));
        a.check("15", tok.checkAdvance(tok.tColon));
        a.check("16", tok.checkAdvance(tok.tSemicolon));
        a.check("17", tok.checkAdvance(tok.tDot));
        a.check("18", tok.checkAdvance(tok.tEnd));
        a.check("19", tok.checkAdvance(tok.tEnd));
        a.check("20", tok.checkAdvance(tok.tEnd));
    }

    // Same thing, no whitespace
    {
        interpreter::Tokenizer tok("&#+-*/\\^(),=<>:;.%&#");
        a.check("21", tok.checkAdvance(tok.tAmpersand));
        a.check("22", tok.checkAdvance(tok.tHash));
        a.check("23", tok.checkAdvance(tok.tPlus));
        a.check("24", tok.checkAdvance(tok.tMinus));
        a.check("25", tok.checkAdvance(tok.tMultiply));
        a.check("26", tok.checkAdvance(tok.tSlash));
        a.check("27", tok.checkAdvance(tok.tBackslash));
        a.check("28", tok.checkAdvance(tok.tCaret));
        a.check("29", tok.checkAdvance(tok.tLParen));
        a.check("30", tok.checkAdvance(tok.tRParen));
        a.check("31", tok.checkAdvance(tok.tComma));
        a.check("32", tok.checkAdvance(tok.tEQ));
        a.check("33", tok.checkAdvance(tok.tNE));       // !
        a.check("34", tok.checkAdvance(tok.tColon));
        a.check("35", tok.checkAdvance(tok.tSemicolon));
        a.check("36", tok.checkAdvance(tok.tDot));
        a.check("37", tok.checkAdvance(tok.tEnd));
        a.check("38", tok.checkAdvance(tok.tEnd));
        a.check("39", tok.checkAdvance(tok.tEnd));
    }

    // Pairs
    {
        interpreter::Tokenizer tok("<> <= >= := < > < = > = : = -> ..");
        a.check("41", tok.checkAdvance(tok.tNE));
        a.check("42", tok.checkAdvance(tok.tLE));
        a.check("43", tok.checkAdvance(tok.tGE));
        a.check("44", tok.checkAdvance(tok.tAssign));
        a.check("45", tok.checkAdvance(tok.tLT));
        a.check("46", tok.checkAdvance(tok.tGT));
        a.check("47", tok.checkAdvance(tok.tLT));
        a.check("48", tok.checkAdvance(tok.tEQ));
        a.check("49", tok.checkAdvance(tok.tGT));
        a.check("50", tok.checkAdvance(tok.tEQ));
        a.check("51", tok.checkAdvance(tok.tColon));
        a.check("52", tok.checkAdvance(tok.tEQ));
        a.check("53", tok.checkAdvance(tok.tArrow));
        a.check("54", tok.checkAdvance(tok.tDot));
        a.check("55", tok.checkAdvance(tok.tDot));
        a.check("56", tok.checkAdvance(tok.tEnd));
    }

    // Possible pairs at end
    {
        interpreter::Tokenizer tok("<");
        a.check("61", tok.checkAdvance(tok.tLT));
        a.check("62", tok.checkAdvance(tok.tEnd));
    }
    {
        interpreter::Tokenizer tok(">");
        a.check("63", tok.checkAdvance(tok.tGT));
        a.check("64", tok.checkAdvance(tok.tEnd));
    }
    {
        interpreter::Tokenizer tok(":");
        a.check("65", tok.checkAdvance(tok.tColon));
        a.check("66", tok.checkAdvance(tok.tEnd));
    }

    // Keywords
    {
        interpreter::Tokenizer tok("and or xor not mod. AND Or Xor nOt moD");
        a.check("71", tok.checkAdvance(tok.tAND));
        a.check("72", tok.checkAdvance(tok.tOR));
        a.check("73", tok.checkAdvance(tok.tXOR));
        a.check("74", tok.checkAdvance(tok.tNOT));
        a.check("75", tok.checkAdvance(tok.tMOD));
        a.check("76", tok.checkAdvance(tok.tDot));
        a.check("77", tok.checkAdvance(tok.tAND));
        a.check("78", tok.checkAdvance(tok.tOR));
        a.check("79", tok.checkAdvance(tok.tXOR));
        a.check("80", tok.checkAdvance(tok.tNOT));
        a.check("81", tok.checkAdvance(tok.tMOD));
        a.check("82", tok.checkAdvance(tok.tEnd));
    }

    // Identifiers
    {
        interpreter::Tokenizer tok("true false cc$notify $foo _foo.bar foo_bar$ f99_ foo.bar2 foo. haha%hehe ");
        a.checkEqual("91. getCurrentToken", tok.getCurrentToken(), tok.tBoolean);
        a.checkEqual("92. getCurrentInteger", tok.getCurrentInteger(), 1);

        a.checkEqual("101. readNextToken", tok.readNextToken(), tok.tBoolean);
        a.checkEqual("102. getCurrentToken", tok.getCurrentToken(), tok.tBoolean);
        a.checkEqual("103. getCurrentInteger", tok.getCurrentInteger(), 0);

        a.checkEqual("111. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("112. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("113. getCurrentString", tok.getCurrentString(), "CC$NOTIFY");

        a.checkEqual("121. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("122. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("123. getCurrentString", tok.getCurrentString(), "$FOO");

        a.checkEqual("131. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("132. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("133. getCurrentString", tok.getCurrentString(), "_FOO.BAR");

        a.checkEqual("141. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("142. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("143. getCurrentString", tok.getCurrentString(), "FOO_BAR$");

        a.checkEqual("151. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("152. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("153. getCurrentString", tok.getCurrentString(), "F99_");

        a.checkEqual("161. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("162. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("163. getCurrentString", tok.getCurrentString(), "FOO.BAR2");

        a.checkEqual("171. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("172. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("173. getCurrentString", tok.getCurrentString(), "FOO");

        a.checkEqual("181. readNextToken", tok.readNextToken(), tok.tDot);
        a.checkEqual("182. getCurrentToken", tok.getCurrentToken(), tok.tDot);

        a.checkEqual("191. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("192. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("193. getCurrentString", tok.getCurrentString(), "HAHA");

        a.checkEqual("201. readNextToken", tok.readNextToken(), tok.tEnd);
    }

    // Invalid
    {
        interpreter::Tokenizer tok("a`b");
        a.checkEqual("211. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("212. getCurrentString", tok.getCurrentString(), "A");

        a.checkEqual("221. readNextToken", tok.readNextToken(), tok.tInvalid);
        a.checkEqual("222. getCurrentToken", tok.getCurrentToken(), tok.tInvalid);
        a.checkEqual("223. getCurrentString", tok.getCurrentString(), "`");

        a.checkEqual("231. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("232. getCurrentToken", tok.getCurrentToken(), tok.tIdentifier);
        a.checkEqual("233. getCurrentString", tok.getCurrentString(), "B");

        a.checkEqual("241. readNextToken", tok.readNextToken(), tok.tEnd);
    }
}

AFL_TEST("interpreter.Tokenizer:integers", a)
{
    // ex IntLexTestSuite::testIntegers
    static const struct { const char* text; int32_t value; } integers[] = {
        { "0", 0 },
        { "000000000000000000", 0 },
        { "1", 1 },
        { "2", 2 },
        { "3", 3 },
        { "4", 4 },
        { "5", 5 },
        { "6", 6 },
        { "7", 7 },
        { "8", 8 },
        { "9", 9 },
        { "10", 10 },
        { "010", 10 },
        { "1000000", 1000000 },
        { "1000000000", 1000000000 },
        { "2147483634", 2147483634 },
        { "2147483635", 2147483635 },
        { "2147483636", 2147483636 },
        { "2147483637", 2147483637 },
        { "2147483638", 2147483638 },
        { "2147483639", 2147483639 },
        { "2147483640", 2147483640 },
        { "2147483641", 2147483641 },
        { "2147483642", 2147483642 },
        { "2147483643", 2147483643 },
        { "2147483644", 2147483644 },
        { "2147483645", 2147483645 },
        { "2147483646", 2147483646 },
        { "2147483647", 2147483647 },
        { "2147483647     ", 2147483647 },
        { "2147483647%99", 2147483647 },
    };

    for (size_t i = 0; i < sizeof(integers)/sizeof(integers[0]); ++i) {
        interpreter::Tokenizer tok(integers[i].text);
        a(integers[i].text).checkEqual("getCurrentToken",   tok.getCurrentToken(), tok.tInteger);
        a(integers[i].text).checkEqual("getCurrentInteger", tok.getCurrentInteger(), integers[i].value);
        a(integers[i].text).checkEqual("readNextToken",     tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("124foo");
        a.checkEqual("01. getCurrentToken", tok.getCurrentToken(), tok.tInteger);
        a.checkEqual("02. getCurrentInteger", tok.getCurrentInteger(), 124);

        a.checkEqual("11. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("12. getCurrentString", tok.getCurrentString(), "FOO");

        a.checkEqual("21. readNextToken", tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("124 5");
        a.checkEqual("31. getCurrentToken", tok.getCurrentToken(), tok.tInteger);
        a.checkEqual("32. getCurrentInteger", tok.getCurrentInteger(), 124);

        a.checkEqual("41. readNextToken", tok.readNextToken(), tok.tInteger);
        a.checkEqual("42. getCurrentInteger", tok.getCurrentInteger(), 5);

        a.checkEqual("51. readNextToken", tok.readNextToken(), tok.tEnd);
    }
}

AFL_TEST("interpreter.Tokenizer:floats", a)
{
    // ex IntLexTestSuite::testFloats
    static const struct { const char* text; double value; } floats[] = {
        { "0.", 0 },
        { ".0", 0 },
        { "1.", 1 },
        { "2.", 2 },
        { "3.", 3 },
        { "4.", 4 },
        { "5.", 5 },
        { "6.", 6 },
        { "7.0", 7 },
        { "8.0", 8 },
        { "9.0", 9 },
        { "10.0", 10 },
        { "010.0", 10.0 },
        { "1000000.0", 1000000.0 },
        { "1000000000.0", 1000000000.0 },
        { "2147483648", 2147483648.0 },
        { "2147483649", 2147483649.0 },
        { "2147483650", 2147483650.0 },
        { "10000000000", 10000000000.0 },
        { "18446744073709551616", 18446744073709551616.0 },
        { "0.5", 0.5 },
        { "0.75", 0.75 },
        { "0.125", 0.125 },
        { "0.3", 0.3 },
        { ".1", 0.1 },
        { "1.2%99", 1.2 },
    };

    for (size_t i = 0; i < sizeof(floats)/sizeof(floats[0]); ++i) {
        interpreter::Tokenizer tok(floats[i].text);
        a(floats[i].text).checkEqual("getCurrentToken", tok.getCurrentToken(), tok.tFloat);
        a(floats[i].text).checkEqual("getCurrentFloat", tok.getCurrentFloat(), floats[i].value);
        a(floats[i].text).checkEqual("readNextToken",   tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("1.2.3.foo");
        a.checkEqual("01. getCurrentToken", tok.getCurrentToken(), tok.tFloat);
        a.checkEqual("02. getCurrentFloat", tok.getCurrentFloat(), 1.2);

        a.checkEqual("11. readNextToken", tok.readNextToken(), tok.tFloat);
        a.checkEqual("12. getCurrentFloat", tok.getCurrentFloat(), 0.3);

        a.checkEqual("21. readNextToken", tok.readNextToken(), tok.tDot);

        a.checkEqual("31. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("32. getCurrentString", tok.getCurrentString(), "FOO");

        a.checkEqual("41. readNextToken", tok.readNextToken(), tok.tEnd);
    }
}

AFL_TEST("interpreter.Tokenizer:strings", a)
{
    // ex IntLexTestSuite::testStrings
    static const struct { const char* text; const char* value; } strings[] = {
        { "''", "" },
        { "\"\"", "" },
        { "'\"'", "\"" },
        { "\"'\"", "'" },
        { "'foo'", "foo" },
        { "\"foo\"", "foo" },
        { "\"fo\\\"o\"", "fo\"o" },
        { "\"fo\\\\o\"", "fo\\o" },
        { "\"hi\\n\"", "hi\n" },
        { "'hi\\n'", "hi\\n" },
        { "\"hi\\t\"", "hi\t" },
        { "'hi\\t'", "hi\\t" },
    };

    for (size_t i = 0; i < sizeof(strings)/sizeof(strings[0]); ++i) {
        interpreter::Tokenizer tok(strings[i].text);
        a(strings[i].text).checkEqual("getCurrentToken",  tok.getCurrentToken(), tok.tString);
        a(strings[i].text).checkEqual("getCurrentString", tok.getCurrentString(), strings[i].value);
        a(strings[i].text).checkEqual("readNextToken",    tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("'foo'bar");
        a.checkEqual("01. getCurrentToken", tok.getCurrentToken(), tok.tString);
        a.checkEqual("02. getCurrentString", tok.getCurrentString(), "foo");

        a.checkEqual("11. readNextToken", tok.readNextToken(), tok.tIdentifier);
        a.checkEqual("12. getCurrentString", tok.getCurrentString(), "BAR");

        a.checkEqual("21. readNextToken", tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("'a''b'");
        a.checkEqual("31. getCurrentToken", tok.getCurrentToken(), tok.tString);
        a.checkEqual("32. getCurrentString", tok.getCurrentString(), "a");

        a.checkEqual("41. readNextToken", tok.readNextToken(), tok.tString);
        a.checkEqual("42. getCurrentString", tok.getCurrentString(), "b");

        a.checkEqual("51. readNextToken", tok.readNextToken(), tok.tEnd);
    }
}

/** Test isIdentifierCharacter. */
AFL_TEST("interpreter.Tokenizer:isIdentifierCharacter", a)
{
    a.check("01", interpreter::Tokenizer::isIdentifierCharacter('.'));
    a.check("02", interpreter::Tokenizer::isIdentifierCharacter('_'));
    a.check("03", interpreter::Tokenizer::isIdentifierCharacter('$'));
    a.check("04", interpreter::Tokenizer::isIdentifierCharacter('I'));
    a.check("05", interpreter::Tokenizer::isIdentifierCharacter('A'));
    a.check("06", interpreter::Tokenizer::isIdentifierCharacter('Z'));
    a.check("07", interpreter::Tokenizer::isIdentifierCharacter('a'));
    a.check("08", interpreter::Tokenizer::isIdentifierCharacter('z'));
    a.check("09", interpreter::Tokenizer::isIdentifierCharacter('0'));
    a.check("10", interpreter::Tokenizer::isIdentifierCharacter('9'));

    a.check("11", !interpreter::Tokenizer::isIdentifierCharacter(':'));
    a.check("12", !interpreter::Tokenizer::isIdentifierCharacter(' '));
    a.check("13", !interpreter::Tokenizer::isIdentifierCharacter('\0'));
    a.check("14", !interpreter::Tokenizer::isIdentifierCharacter('\xf6'));
}

/** Test isValidUppercaseIdentifier(). */
AFL_TEST("interpreter.Tokenizer:isValidUppercaseIdentifier", a)
{
    a.check("01", !interpreter::Tokenizer::isValidUppercaseIdentifier(""));
    a.check("02",  interpreter::Tokenizer::isValidUppercaseIdentifier("X"));
    a.check("03",  interpreter::Tokenizer::isValidUppercaseIdentifier("X9"));
    a.check("04",  interpreter::Tokenizer::isValidUppercaseIdentifier("X.Y"));
    a.check("05",  interpreter::Tokenizer::isValidUppercaseIdentifier("X$"));
    a.check("06",  interpreter::Tokenizer::isValidUppercaseIdentifier("X_"));
    a.check("07",  interpreter::Tokenizer::isValidUppercaseIdentifier("_X"));
    a.check("08",  interpreter::Tokenizer::isValidUppercaseIdentifier("X1"));
    a.check("09", !interpreter::Tokenizer::isValidUppercaseIdentifier("1X"));
    a.check("10", !interpreter::Tokenizer::isValidUppercaseIdentifier("$X"));
    a.check("11", !interpreter::Tokenizer::isValidUppercaseIdentifier("x"));
    a.check("12", !interpreter::Tokenizer::isValidUppercaseIdentifier("Xx"));
}

/** Test bad strings. */
AFL_TEST("interpreter.Tokenizer:bad-strings", a)
{
    const char*const strings[] = {
        "'foo",
        "\"foo",
        "\"foo\\",
    };
    for (size_t i = 0; i < countof(strings); ++i) {
        AFL_CHECK_THROWS(a, (interpreter::Tokenizer(strings[i])), interpreter::Error);
    }
}
