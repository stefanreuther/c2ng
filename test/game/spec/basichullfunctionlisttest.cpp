/**
  *  \file test/game/spec/basichullfunctionlisttest.cpp
  *  \brief Test for game::spec::BasicHullFunctionList
  */

#include "game/spec/basichullfunctionlist.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"
#include "afl/test/testrunner.hpp"

using afl::io::ConstMemoryStream;
using afl::string::NullTranslator;
using afl::string::toBytes;
using afl::sys::Log;
using game::spec::BasicHullFunctionList;

/** Test list I/O. */
AFL_TEST("game.spec.BasicHullFunctionList:load:success", a)
{
    // Default-construct an object
    BasicHullFunctionList testee;
    a.checkNull("01", testee.getFunctionById(1));

    // Load an example file
    static const char SAMPLE_FILE[] =
        "; Comment\n"
        "7,a,Alchemy\n"
        "c = A\n"
        "d = 3-to-1\n"
        "e = does this\n"
        "s = 105\n"
        "e = and that\n"           // explanation continuation
        "1,a,Refinery\n"
        "d = 2-to-1\n"
        "s = 104\n"
        "p = ref\n"
        "2,a,AdvancedRefinery\n"
        "c = R\n"
        "d = 1-to-1\n"
        " s = 97 \n"
        " i = 1\n"
        "d = improved!\n";         // description replacement
    {
        ConstMemoryStream ms(toBytes(SAMPLE_FILE));
        NullTranslator tx;
        Log log;
        AFL_CHECK_SUCCEEDS(a("11. load"), testee.load(ms, tx, log));
    }

    // Verify content
    const game::spec::BasicHullFunction* p = testee.getFunctionById(7);
    a.checkNonNull("21. getFunctionById", p);
    a.checkEqual("22. getId",                p->getId(), 7);
    a.checkEqual("23. getName",              p->getName(), "Alchemy");
    a.checkEqual("24. getDescription",       p->getDescription(), "3-to-1");
    a.checkEqual("25. getImpliedFunctionId", p->getImpliedFunctionId(), -1);
    a.checkEqual("26. getExplanation",       p->getExplanation(), "does this\nand that");
    a.checkEqual("27. getPictureName",       p->getPictureName(), "");
    a.checkEqual("28. getCode",              p->getCode(), "A");

    p = testee.getFunctionById(1);
    a.checkNonNull("31. getFunctionById", p);
    a.checkEqual("32. getId",                p->getId(), 1);
    a.checkEqual("33. getName",              p->getName(), "Refinery");
    a.checkEqual("34. getDescription",       p->getDescription(), "2-to-1");
    a.checkEqual("35. getImpliedFunctionId", p->getImpliedFunctionId(), -1);
    a.checkEqual("36. getPictureName",       p->getPictureName(), "ref");
    a.checkEqual("37. getCode",              p->getCode(), "");

    p = testee.getFunctionById(2);
    a.checkNonNull("41. getFunctionById", p);
    a.checkEqual("42. getId",                p->getId(), 2);
    a.checkEqual("43. getName",              p->getName(), "AdvancedRefinery");
    a.checkEqual("44. getDescription",       p->getDescription(), "improved!");
    a.checkEqual("45. getImpliedFunctionId", p->getImpliedFunctionId(), 1);
    a.checkEqual("46. getPictureName",       p->getPictureName(), "");
    a.checkEqual("47. getCode",              p->getCode(), "R");

    a.checkNull("51. getFunctionById", testee.getFunctionById(3));

    // Access by name
    a.checkNonNull("61. getFunctionByName", testee.getFunctionByName("Alchemy", false));
    a.checkNonNull("62. getFunctionByName", testee.getFunctionByName("ALCHEMY", false));
    a.checkNonNull("63. getFunctionByName", testee.getFunctionByName("alchemy", false));
    a.checkNonNull("64. getFunctionByName", testee.getFunctionByName("alchemy", true));
    a.checkNull   ("65. getFunctionByName", testee.getFunctionByName("al", false));
    a.checkNonNull("66. getFunctionByName", testee.getFunctionByName("al", true));

    a.checkNull   ("71. getFunctionByName", testee.getFunctionByName("adv", false));
    a.checkNonNull("72. getFunctionByName", testee.getFunctionByName("adv", true));

    a.checkNull   ("81. getFunctionByName", testee.getFunctionByName("2", true));

    // Access by index
    a.checkEqual("91. getNumFunctions", testee.getNumFunctions(), 3U);
    a.checkEqual("92. getFunctionByIndex", testee.getFunctionByIndex(0)->getName(), "Alchemy");
    a.checkEqual("93. getFunctionByIndex", testee.getFunctionByIndex(2)->getName(), "AdvancedRefinery");
    a.checkNull ("94. getFunctionByIndex", testee.getFunctionByIndex(3));

    // Clear
    testee.clear();
    a.checkNull("101. getFunctionByName", testee.getFunctionByName("Alchemy", false));
    a.checkNull("102. getFunctionById", testee.getFunctionById(1));
    a.checkEqual("103. getNumFunctions", testee.getNumFunctions(), 0U);
    a.checkNull("104. getFunctionByIndex", testee.getFunctionByIndex(3));
}

/** Test matchFunction(). */
AFL_TEST("game.spec.BasicHullFunctionList:matchFunction", a)
{
    // Build a definition list:
    BasicHullFunctionList testee;

    // 0 is alone
    testee.addFunction(0, "Alchemy");

    // 3->4->2->1
    testee.addFunction(1, "Refinery");
    testee.addFunction(2, "AdvancedRefinery")->setImpliedFunctionId(1);
    testee.addFunction(3, "UltraAdvancedRefinery")->setImpliedFunctionId(4);
    testee.addFunction(4, "SuperAdvancedRefinery")->setImpliedFunctionId(2);

    // Self-match
    a.check("01", testee.matchFunction(99, 99));

    // Nonexistant does not match
    a.check("11", !testee.matchFunction(98, 1));

    // Match all functions against each other
    a.check("21",  testee.matchFunction(1, 1));
    a.check("22",  testee.matchFunction(1, 2));
    a.check("23",  testee.matchFunction(1, 3));
    a.check("24",  testee.matchFunction(1, 4));

    a.check("31", !testee.matchFunction(2, 1));
    a.check("32",  testee.matchFunction(2, 2));
    a.check("33",  testee.matchFunction(2, 3));
    a.check("34",  testee.matchFunction(2, 4));

    a.check("41", !testee.matchFunction(3, 1));
    a.check("42", !testee.matchFunction(3, 2));
    a.check("43",  testee.matchFunction(3, 3));
    a.check("44", !testee.matchFunction(3, 4));

    a.check("51", !testee.matchFunction(4, 1));
    a.check("52", !testee.matchFunction(4, 2));
    a.check("53",  testee.matchFunction(4, 3));
    a.check("54",  testee.matchFunction(4, 4));
}

/** Test handling of looping "implies" chains. */
AFL_TEST("game.spec.BasicHullFunctionList:matchFunction:loop", a)
{
    BasicHullFunctionList testee;

    // Loop 10->11->12
    testee.addFunction(10, "X")->setImpliedFunctionId(12);
    testee.addFunction(11, "Y")->setImpliedFunctionId(10);
    testee.addFunction(12, "Z")->setImpliedFunctionId(11);

    // Entry points
    testee.addFunction(20, "A")->setImpliedFunctionId(10);
    testee.addFunction(21, "B")->setImpliedFunctionId(11);
    testee.addFunction(22, "C")->setImpliedFunctionId(12);

    testee.addFunction(0, "M")->setImpliedFunctionId(10);
    testee.addFunction(1, "N")->setImpliedFunctionId(11);
    testee.addFunction(2, "O")->setImpliedFunctionId(12);

    // Successful links
    for (int src = 0; src < 3; ++src) {
        for (int dst = 0; dst < 3; ++dst) {
            a.check("01", testee.matchFunction(src+10, dst));
            a.check("02", testee.matchFunction(src+10, dst+10));
            a.check("03", testee.matchFunction(src+10, dst+20));
        }
    }

    // Unsuccessful links
    // (Test failure means this hangs.)
    a.check("11", !testee.matchFunction( 0, 10));
    a.check("12", !testee.matchFunction( 1, 10));
    a.check("13", !testee.matchFunction( 2, 10));
    a.check("14", !testee.matchFunction(20, 10));
    a.check("15", !testee.matchFunction( 0,  1));
}

/** Test handling of unterminated "implies" chains. */
AFL_TEST("game.spec.BasicHullFunctionList:matchFunction:dead-link", a)
{
    BasicHullFunctionList testee;

    // Unterminated chain
    testee.addFunction(2, "A")->setImpliedFunctionId(1);
    testee.addFunction(3, "U")->setImpliedFunctionId(4);
    testee.addFunction(4, "S")->setImpliedFunctionId(2);

    // Because we do not need to resolve the final function, these tests still work
    // (These are the same tests as in testMatch).
    a.check("01",  testee.matchFunction(1, 1));
    a.check("02",  testee.matchFunction(1, 2));
    a.check("03",  testee.matchFunction(1, 3));
    a.check("04",  testee.matchFunction(1, 4));

    a.check("11", !testee.matchFunction(2, 1));
    a.check("12",  testee.matchFunction(2, 2));
    a.check("13",  testee.matchFunction(2, 3));
    a.check("14",  testee.matchFunction(2, 4));

    a.check("21", !testee.matchFunction(3, 1));
    a.check("22", !testee.matchFunction(3, 2));
    a.check("23",  testee.matchFunction(3, 3));
    a.check("24", !testee.matchFunction(3, 4));

    a.check("31", !testee.matchFunction(4, 1));
    a.check("32", !testee.matchFunction(4, 2));
    a.check("33",  testee.matchFunction(4, 3));
    a.check("34",  testee.matchFunction(4, 4));

    // A nonexistant target
    a.check("41", !testee.matchFunction(4, 9));
    a.check("42", !testee.matchFunction(9, 4));
}

/** Test errors when loading. */

// Syntax error in line
AFL_TEST("game.spec.BasicHullFunctionList:load:error:syntax-error", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("\nhi mom\n"));
    afl::test::LogListener log;
    BasicHullFunctionList().load(ms, tx, log);
    a.check("getNumMessages", log.getNumMessages() > 0);
}

// Syntax error in line
AFL_TEST("game.spec.BasicHullFunctionList:load:error:syntax-error:2", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("\n1,foo\n"));
    afl::test::LogListener log;
    BasicHullFunctionList().load(ms, tx, log);
    a.check("getNumMessages", log.getNumMessages() > 0);
}

// Invalid number
AFL_TEST("game.spec.BasicHullFunctionList:load:error:invalid-number", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "999999,a,improvedalchemy\n"
                                 "d=i\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
    a.checkNull("03. getFunctionById", hfl.getFunctionById(999999));
    a.checkEqual("04. getDescription", hfl.getFunctionById(1)->getDescription(), "alchemy");
}

// Duplicate name
AFL_TEST("game.spec.BasicHullFunctionList:load:error:duplicate-name", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "d=one\n"
                                 "2,a,alchemy\n"
                                 "d=two\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
    a.checkNull("03. getFunctionById", hfl.getFunctionById(2));
    a.checkEqual("04. getDescription", hfl.getFunctionById(1)->getDescription(), "one");
}

// Duplicate Id
AFL_TEST("game.spec.BasicHullFunctionList:load:error:duplicate-id", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "d=one\n"
                                 "1,a,somethingelse\n"
                                 "d=two\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
    a.checkEqual("03. getDescription", hfl.getFunctionById(1)->getDescription(), "one");
}

// Missing function
AFL_TEST("game.spec.BasicHullFunctionList:load:error:missing-function", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("d=one\n"));
    afl::test::LogListener log;
    BasicHullFunctionList().load(ms, tx, log);
    a.check("getNumMessages", log.getNumMessages() > 0);
}

// Bad implication - invalid name
AFL_TEST("game.spec.BasicHullFunctionList:load:error:implication:bad-name", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "i=foo\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
    a.check("63. getImpliedFunctionId", hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
}

// Bad implication - self reference
AFL_TEST("game.spec.BasicHullFunctionList:load:error:implication:self-reference", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "i=1\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.checkNonNull("01. getFunctionById", hfl.getFunctionById(1));      // This is not an error
    a.check("02. getImpliedFunctionId", hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
}

// Bad implication - self reference by name
AFL_TEST("game.spec.BasicHullFunctionList:load:error:implication:self-reference-by-name", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "i=alchemy\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.checkNonNull("01. getFunctionById", hfl.getFunctionById(1));      // This is not an error
    a.check("02. getImpliedFunctionId", hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
}

// Bad standard assignment
AFL_TEST("game.spec.BasicHullFunctionList:load:error:standard:1", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "s=x\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
}

// Bad standard assignment, case 2
AFL_TEST("game.spec.BasicHullFunctionList:load:error:standard:2", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "s=3,4,x\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
}

// Bad standard assignment, case 3
AFL_TEST("game.spec.BasicHullFunctionList:load:error:standard:3", a)
{
    NullTranslator tx;
    ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                 "s=5,3,1,-1\n"));
    afl::test::LogListener log;
    BasicHullFunctionList hfl;
    hfl.load(ms, tx, log);
    a.check("01. getNumMessages", log.getNumMessages() > 0);
    a.checkNonNull("02. getFunctionById", hfl.getFunctionById(1));
}


/** Test bug 342.
    This should already be covered by the other tests. */
AFL_TEST("game.spec.BasicHullFunctionList:bug:342", a)
{
    // Default-construct an object
    BasicHullFunctionList testee;

    // Load an example file
    static const char SAMPLE_FILE[] =
        "1,a,Looper\n"
        "i = 4\n"
        "4,a,Loopzor\n"
        "i = 1\n"
        "7,,Seven\n"
        "d = Seven described\n"
        "29,,Twentynine\n"
        "d=Twentynine described\n";
    {
        ConstMemoryStream ms(toBytes(SAMPLE_FILE));
        NullTranslator tx;
        Log log;
        AFL_CHECK_SUCCEEDS(a("01. load"), testee.load(ms, tx, log));
    }

    // Verify content
    a.checkNonNull("11. getFunctionById", testee.getFunctionById(1));
    a.checkNonNull("12. getFunctionById", testee.getFunctionById(4));
    a.checkNonNull("13. getFunctionById", testee.getFunctionById(7));
    a.checkNonNull("14. getFunctionById", testee.getFunctionById(29));

    a.check("21. matchFunction", testee.matchFunction(1, 4));
    a.check("22. matchFunction", testee.matchFunction(4, 1));
    a.check("23. matchFunction", !testee.matchFunction(16, 4));  // This used to hang: client asks for Cloak, having found CoolsTo50
    a.check("24. matchFunction", !testee.matchFunction(4, 16));

    a.checkEqual("31. getDescription", testee.getFunctionById(7)->getDescription(), "Seven described");
    a.checkEqual("32. getDescription", testee.getFunctionById(29)->getDescription(), "Twentynine described");
}
