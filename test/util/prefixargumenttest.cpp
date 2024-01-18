/**
  *  \file test/util/prefixargumenttest.cpp
  *  \brief Test for util::PrefixArgument
  */

#include "util/prefixargument.hpp"

#include "afl/string/nulltranslator.hpp"
#include "afl/test/testrunner.hpp"

/** Simple basic tests. */
AFL_TEST("util.PrefixArgument:basics", a)
{
    // Environment
    afl::string::NullTranslator tx;

    // Testee
    util::PrefixArgument testee(3);

    // Initial state
    a.checkEqual("01. getValue", testee.getValue(), 3);
    a.checkEqual("02. getText",  testee.getText(tx), "Prefix: 3");

    // Type some digits
    a.checkEqual("11. handleKey", testee.handleKey('9'), util::PrefixArgument::Accepted);
    a.checkEqual("12. getValue",  testee.getValue(), 39);
    a.checkEqual("13. handleKey", testee.handleKey('2'), util::PrefixArgument::Accepted);
    a.checkEqual("14. getValue",  testee.getValue(), 392);
    a.checkEqual("15. handleKey", testee.handleKey('1'), util::PrefixArgument::Accepted);
    a.checkEqual("16. getValue",  testee.getValue(), 3921);
    a.checkEqual("17. getText",   testee.getText(tx), "Prefix: 3921");

    // Overflow
    a.checkEqual("21. handleKey", testee.handleKey('7'), util::PrefixArgument::Accepted);
    a.checkEqual("22. getValue",  testee.getValue(), 3921);

    // Backspace
    a.checkEqual("31. handleKey", testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    a.checkEqual("32. getValue",  testee.getValue(), 392);

    // Backspace until cancel
    a.checkEqual("41. handleKey", testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    a.checkEqual("42. getValue",  testee.getValue(), 39);
    a.checkEqual("43. handleKey", testee.handleKey(util::Key_Backspace), util::PrefixArgument::Accepted);
    a.checkEqual("44. getValue",  testee.getValue(), 3);
    a.checkEqual("45. handleKey", testee.handleKey(util::Key_Backspace), util::PrefixArgument::Canceled);
    a.checkEqual("46. getValue",  testee.getValue(), 0);
}

/** Test sequences. This tests most user interactions that produce a value. */
AFL_TEST("util.PrefixArgument:sequences", a)
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
            util::PrefixArgument::Action ac;
            if (*p == 'b') {
                ac = testee.handleKey(util::Key_Backspace);
            } else {
                ac = testee.handleKey(*p);
            }
            a(sequences[i].seq).checkEqual("handleKey", ac, util::PrefixArgument::Accepted);
        }
        a(sequences[i].seq).checkEqual("getValue", testee.getValue(), sequences[i].value);
        a(sequences[i].seq).checkEqual("getText",  testee.getText(tx), sequences[i].text);
    }
}

/*
 *  Test various cancellations.
 */

// Immediate cancel
AFL_TEST("util.PrefixArgument:cancel:direct", a)
{
    util::PrefixArgument t(3);
    a.checkEqual("01. handleKey", t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
    a.checkEqual("02. getValue",  t.getValue(), 0);
}

// Cancel after operator
AFL_TEST("util.PrefixArgument:cancel:after-operator", a)
{
    util::PrefixArgument t(3);
    a.checkEqual("11. handleKey", t.handleKey('*'),              util::PrefixArgument::Accepted);
    a.checkEqual("12. handleKey", t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
    a.checkEqual("13. getValue",  t.getValue(), 0);
}

// Revive after cancel
AFL_TEST("util.PrefixArgument:cancel:revive:digit", a)
{
    util::PrefixArgument t(3);
    a.checkEqual("21. handleKey", t.handleKey('*'),              util::PrefixArgument::Accepted);
    a.checkEqual("22. handleKey", t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
    a.checkEqual("23. getValue",  t.getValue(), 0);
    a.checkEqual("24. handleKey", t.handleKey('9'),              util::PrefixArgument::Accepted);
    a.checkEqual("25. getValue",  t.getValue(), 9);
    a.checkEqual("26. handleKey", t.handleKey('1'),              util::PrefixArgument::Accepted);
    a.checkEqual("27. getValue",  t.getValue(), 91);
}

// Operator after cancel
AFL_TEST("util.PrefixArgument:cancel:revive:operator", a)
{
    util::PrefixArgument t(3);
    a.checkEqual("31. handleKey", t.handleKey(util::Key_Escape), util::PrefixArgument::Canceled);
    a.checkEqual("32. getValue",  t.getValue(), 0);
    a.checkEqual("33. handleKey", t.handleKey('*'),              util::PrefixArgument::Accepted);
    a.checkEqual("34. handleKey", t.handleKey('7'),              util::PrefixArgument::Accepted);
    a.checkEqual("35. getValue",  t.getValue(), 7);
}
