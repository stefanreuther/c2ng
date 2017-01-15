/**
  *  \file u/t_util_prefixargument.cpp
  *  \brief Test for util::PrefixArgument
  */

#include "util/prefixargument.hpp"

#include "t_util.hpp"
#include "afl/string/nulltranslator.hpp"

/** Simple basic tests. */
void
TestUtilPrefixArgument::testIt()
{
    // Environment
    afl::string::NullTranslator tx;

    // Testee
    util::PrefixArgument testee(3);

    // Initial state
    TS_ASSERT_EQUALS(testee.getValue(), 3);
    TS_ASSERT_EQUALS(testee.getText(tx), "Prefix: 3");

    // Type some digits
    TS_ASSERT_EQUALS(testee.handleKey('9'), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 39);
    TS_ASSERT_EQUALS(testee.handleKey('2'), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 392);
    TS_ASSERT_EQUALS(testee.handleKey('1'), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 3921);
    TS_ASSERT_EQUALS(testee.getText(tx), "Prefix: 3921");

    // Overflow
    TS_ASSERT_EQUALS(testee.handleKey('7'), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 3921);

    // Backspace
    TS_ASSERT_EQUALS(testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 392);

    // Backspace until cancel
    TS_ASSERT_EQUALS(testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 39);
    TS_ASSERT_EQUALS(testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    TS_ASSERT_EQUALS(testee.getValue(), 3);
    TS_ASSERT_EQUALS(testee.handleKey(util::Key_Backspace), util::PrefixArgument::Canceled);
    TS_ASSERT_EQUALS(testee.getValue(), 0);
}

/** Test sequences. This tests most user interactions that produce a value. */
void
TestUtilPrefixArgument::testSequences()
{
    afl::string::NullTranslator tx;
    struct Sequence {
        const char* seq;
        int value;
        const char* text;
    };
    // Note all sequences are initialized with a "1" in front.
    static const Sequence sequences[] = {
        { "1",       11, "Prefix: 11" },          // Normal input
        { "0*8",     80, "Prefix: 10*8" },        // Multiply
        { "0*81",   810, "Prefix: 10*81" },       // Multiply multiple digits
        { "0*8b",    10, "Prefix: 10*" },         // Cancel multiplicant
        { "0*8b7",   70, "Prefix: 10*7" },        // Cancel multiplicant and enter new one
        { "0*8bb",   10, "Prefix: 10" },          // Cancel multiplicant + operator
        { "0*8bbb",   1, "Prefix: 1" },           // Cancel multiplicant + operator + input
        { "5*0*0*5", 75, "Prefix: 15*5" },        // Multiply by zero is ignored
        { "5/0/0/5",  3, "Prefix: 15/5" },        // Divide by zero is ignored
        { "5/0/0*5", 75, "Prefix: 15*5" },        // Ignore by-zero, execute last
        { "0/20",     5, "Prefix: 10/2" },        // The "0" input is ignored because it would make the result 0
        { "5/163",    1, "Prefix: 15/13" },       // The "6" input is ignored because it would make the result 0
    };
    for (size_t i = 0; i < sizeof(sequences)/sizeof(sequences[0]); ++i) {
        util::PrefixArgument testee(1);
        for (const char* p = sequences[i].seq; *p; ++p) {
            util::PrefixArgument::Action a;
            if (*p == 'b') {
                a = testee.handleKey(util::Key_Backspace);
            } else {
                a = testee.handleKey(*p);
            }
            TSM_ASSERT_EQUALS(sequences[i].seq, a, util::PrefixArgument::Accepted);
        }
        TSM_ASSERT_EQUALS(sequences[i].seq, testee.getValue(), sequences[i].value);
        TSM_ASSERT_EQUALS(sequences[i].seq, testee.getText(tx), sequences[i].text);
    }
}

/** Test various cancellations. */
void
TestUtilPrefixArgument::testCancel()
{
    // Immediate cancel
    {
        util::PrefixArgument t(3);
        TS_ASSERT_EQUALS(t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
        TS_ASSERT_EQUALS(t.getValue(), 0);
    }

    // Cancel after operand
    {
        util::PrefixArgument t(3);
        TS_ASSERT_EQUALS(t.handleKey('*'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
        TS_ASSERT_EQUALS(t.getValue(), 0);
    }

    // Revive after cancel
    {
        util::PrefixArgument t(3);
        TS_ASSERT_EQUALS(t.handleKey('*'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
        TS_ASSERT_EQUALS(t.getValue(), 0);
        TS_ASSERT_EQUALS(t.handleKey('9'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.getValue(), 9);
        TS_ASSERT_EQUALS(t.handleKey('1'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.getValue(), 91);
    }

    // Operator after cancel
    {
        util::PrefixArgument t(3);
        TS_ASSERT_EQUALS(t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
        TS_ASSERT_EQUALS(t.getValue(), 0);
        TS_ASSERT_EQUALS(t.handleKey('*'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.handleKey('7'),              util::PrefixArgument::Accepted);
        TS_ASSERT_EQUALS(t.getValue(), 7);
    }
}
