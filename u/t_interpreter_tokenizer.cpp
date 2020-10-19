/**
  *  \file u/t_interpreter_tokenizer.cpp
  *  \brief Test for interpreter::Tokenizer
  */

#include "interpreter/tokenizer.hpp"

#include "t_interpreter.hpp"
#include "afl/base/countof.hpp"
#include "interpreter/error.hpp"

void
TestInterpreterTokenizer::testTokenizer()
{
    // ex IntLexTestSuite::testTokenizer
    // All single tokens
    {
        interpreter::Tokenizer tok(" & # + - * / \\ ^ ( ) , = < > : ; . % ");
        TS_ASSERT(tok.checkAdvance(tok.tAmpersand));
        TS_ASSERT(tok.checkAdvance(tok.tHash));
        TS_ASSERT(tok.checkAdvance(tok.tPlus));
        TS_ASSERT(tok.checkAdvance(tok.tMinus));
        TS_ASSERT(tok.checkAdvance(tok.tMultiply));
        TS_ASSERT(tok.checkAdvance(tok.tSlash));
        TS_ASSERT(tok.checkAdvance(tok.tBackslash));
        TS_ASSERT(tok.checkAdvance(tok.tCaret));
        TS_ASSERT(tok.checkAdvance(tok.tLParen));
        TS_ASSERT(tok.checkAdvance(tok.tRParen));
        TS_ASSERT(tok.checkAdvance(tok.tComma));
        TS_ASSERT(tok.checkAdvance(tok.tEQ));
        TS_ASSERT(tok.checkAdvance(tok.tLT));
        TS_ASSERT(tok.checkAdvance(tok.tGT));
        TS_ASSERT(tok.checkAdvance(tok.tColon));
        TS_ASSERT(tok.checkAdvance(tok.tSemicolon));
        TS_ASSERT(tok.checkAdvance(tok.tDot));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }

    // Same thing, no whitespace
    {
        interpreter::Tokenizer tok("&#+-*/\\^(),=<>:;.%&#");
        TS_ASSERT(tok.checkAdvance(tok.tAmpersand));
        TS_ASSERT(tok.checkAdvance(tok.tHash));
        TS_ASSERT(tok.checkAdvance(tok.tPlus));
        TS_ASSERT(tok.checkAdvance(tok.tMinus));
        TS_ASSERT(tok.checkAdvance(tok.tMultiply));
        TS_ASSERT(tok.checkAdvance(tok.tSlash));
        TS_ASSERT(tok.checkAdvance(tok.tBackslash));
        TS_ASSERT(tok.checkAdvance(tok.tCaret));
        TS_ASSERT(tok.checkAdvance(tok.tLParen));
        TS_ASSERT(tok.checkAdvance(tok.tRParen));
        TS_ASSERT(tok.checkAdvance(tok.tComma));
        TS_ASSERT(tok.checkAdvance(tok.tEQ));
        TS_ASSERT(tok.checkAdvance(tok.tNE));       // !
        TS_ASSERT(tok.checkAdvance(tok.tColon));
        TS_ASSERT(tok.checkAdvance(tok.tSemicolon));
        TS_ASSERT(tok.checkAdvance(tok.tDot));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }

    // Pairs
    {
        interpreter::Tokenizer tok("<> <= >= := < > < = > = : = -> ..");
        TS_ASSERT(tok.checkAdvance(tok.tNE));
        TS_ASSERT(tok.checkAdvance(tok.tLE));
        TS_ASSERT(tok.checkAdvance(tok.tGE));
        TS_ASSERT(tok.checkAdvance(tok.tAssign));
        TS_ASSERT(tok.checkAdvance(tok.tLT));
        TS_ASSERT(tok.checkAdvance(tok.tGT));
        TS_ASSERT(tok.checkAdvance(tok.tLT));
        TS_ASSERT(tok.checkAdvance(tok.tEQ));
        TS_ASSERT(tok.checkAdvance(tok.tGT));
        TS_ASSERT(tok.checkAdvance(tok.tEQ));
        TS_ASSERT(tok.checkAdvance(tok.tColon));
        TS_ASSERT(tok.checkAdvance(tok.tEQ));
        TS_ASSERT(tok.checkAdvance(tok.tArrow));
        TS_ASSERT(tok.checkAdvance(tok.tDot));
        TS_ASSERT(tok.checkAdvance(tok.tDot));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }

    // Possible pairs at end
    {
        interpreter::Tokenizer tok("<");
        TS_ASSERT(tok.checkAdvance(tok.tLT));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }
    {
        interpreter::Tokenizer tok(">");
        TS_ASSERT(tok.checkAdvance(tok.tGT));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }
    {
        interpreter::Tokenizer tok(":");
        TS_ASSERT(tok.checkAdvance(tok.tColon));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }

    // Keywords
    {
        interpreter::Tokenizer tok("and or xor not mod. AND Or Xor nOt moD");
        TS_ASSERT(tok.checkAdvance(tok.tAND));
        TS_ASSERT(tok.checkAdvance(tok.tOR));
        TS_ASSERT(tok.checkAdvance(tok.tXOR));
        TS_ASSERT(tok.checkAdvance(tok.tNOT));
        TS_ASSERT(tok.checkAdvance(tok.tMOD));
        TS_ASSERT(tok.checkAdvance(tok.tDot));
        TS_ASSERT(tok.checkAdvance(tok.tAND));
        TS_ASSERT(tok.checkAdvance(tok.tOR));
        TS_ASSERT(tok.checkAdvance(tok.tXOR));
        TS_ASSERT(tok.checkAdvance(tok.tNOT));
        TS_ASSERT(tok.checkAdvance(tok.tMOD));
        TS_ASSERT(tok.checkAdvance(tok.tEnd));
    }

    // Identifiers
    {
        interpreter::Tokenizer tok("true false cc$notify $foo _foo.bar foo_bar$ f99_ foo.bar2 foo. haha%hehe ");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tBoolean);
        TS_ASSERT_EQUALS(tok.getCurrentInteger(), 1);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tBoolean);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tBoolean);
        TS_ASSERT_EQUALS(tok.getCurrentInteger(), 0);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "CC$NOTIFY");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "$FOO");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "_FOO.BAR");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "FOO_BAR$");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "F99_");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "FOO.BAR2");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "FOO");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tDot);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tDot);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "HAHA");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }

    // Invalid
    {
        interpreter::Tokenizer tok("a`b");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "A");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tInvalid);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tInvalid);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "`");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "B");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }
}

void
TestInterpreterTokenizer::testIntegers()
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
        TSM_ASSERT_EQUALS(integers[i].text, tok.getCurrentToken(), tok.tInteger);
        TSM_ASSERT_EQUALS(integers[i].text, tok.getCurrentInteger(), integers[i].value);
        TSM_ASSERT_EQUALS(integers[i].text, tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("124foo");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tInteger);
        TS_ASSERT_EQUALS(tok.getCurrentInteger(), 124);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "FOO");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("124 5");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tInteger);
        TS_ASSERT_EQUALS(tok.getCurrentInteger(), 124);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tInteger);
        TS_ASSERT_EQUALS(tok.getCurrentInteger(), 5);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }
}

void
TestInterpreterTokenizer::testFloats()
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
        TSM_ASSERT_EQUALS(floats[i].text, tok.getCurrentToken(), tok.tFloat);
        TSM_ASSERT_EQUALS(floats[i].text, tok.getCurrentFloat(), floats[i].value);
        TSM_ASSERT_EQUALS(floats[i].text, tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("1.2.3.foo");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tFloat);
        TS_ASSERT_EQUALS(tok.getCurrentFloat(), 1.2);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tFloat);
        TS_ASSERT_EQUALS(tok.getCurrentFloat(), 0.3);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tDot);

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "FOO");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }
}

void
TestInterpreterTokenizer::testStrings()
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
        TSM_ASSERT_EQUALS(strings[i].text, tok.getCurrentToken(), tok.tString);
        TSM_ASSERT_EQUALS(strings[i].text, tok.getCurrentString(), strings[i].value);
        TSM_ASSERT_EQUALS(strings[i].text, tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("'foo'bar");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tString);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "foo");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tIdentifier);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "BAR");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }

    {
        interpreter::Tokenizer tok("'a''b'");
        TS_ASSERT_EQUALS(tok.getCurrentToken(), tok.tString);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "a");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tString);
        TS_ASSERT_EQUALS(tok.getCurrentString(), "b");

        TS_ASSERT_EQUALS(tok.readNextToken(), tok.tEnd);
    }
}

/** Test isIdentifierCharacter. */
void
TestInterpreterTokenizer::testIsIdentifierCharacter()
{
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('.'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('_'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('$'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('I'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('A'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('Z'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('a'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('z'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('0'));
    TS_ASSERT(interpreter::Tokenizer::isIdentifierCharacter('9'));

    TS_ASSERT(!interpreter::Tokenizer::isIdentifierCharacter(':'));
    TS_ASSERT(!interpreter::Tokenizer::isIdentifierCharacter(' '));
    TS_ASSERT(!interpreter::Tokenizer::isIdentifierCharacter('\0'));
    TS_ASSERT(!interpreter::Tokenizer::isIdentifierCharacter('\xf6'));
}

/** Test isValidUppercaseIdentifier(). */
void
TestInterpreterTokenizer::testIsValidUppercaseIdentifier()
{
    TS_ASSERT(!interpreter::Tokenizer::isValidUppercaseIdentifier(""));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X9"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X.Y"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X$"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X_"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("_X"));
    TS_ASSERT( interpreter::Tokenizer::isValidUppercaseIdentifier("X1"));
    TS_ASSERT(!interpreter::Tokenizer::isValidUppercaseIdentifier("1X"));
    TS_ASSERT(!interpreter::Tokenizer::isValidUppercaseIdentifier("$X"));
    TS_ASSERT(!interpreter::Tokenizer::isValidUppercaseIdentifier("x"));
    TS_ASSERT(!interpreter::Tokenizer::isValidUppercaseIdentifier("Xx"));
}

/** Test bad strings. */
void
TestInterpreterTokenizer::testBadStrings()
{
    const char*const strings[] = {
        "'foo",
        "\"foo",
        "\"foo\\",
    };
    for (size_t i = 0; i < countof(strings); ++i) {
        TS_ASSERT_THROWS((interpreter::Tokenizer(strings[i])), interpreter::Error);
    }
}

