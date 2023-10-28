/**
  *  \file u/t_game_spec_basichullfunctionlist.cpp
  *  \brief Test for game::spec::BasicHullFunctionList
  */

#include "game/spec/basichullfunctionlist.hpp"

#include "t_game_spec.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/loglistener.hpp"

/** Test list I/O. */
void
TestGameSpecBasicHullFunctionList::testIO()
{
    // Default-construct an object
    game::spec::BasicHullFunctionList testee;
    TS_ASSERT(testee.getFunctionById(1) == 0);

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
        afl::io::ConstMemoryStream ms(afl::string::toBytes(SAMPLE_FILE));
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    }

    // Verify content
    const game::spec::BasicHullFunction* p = testee.getFunctionById(7);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getId(), 7);
    TS_ASSERT_EQUALS(p->getName(), "Alchemy");
    TS_ASSERT_EQUALS(p->getDescription(), "3-to-1");
    TS_ASSERT_EQUALS(p->getImpliedFunctionId(), -1);
    TS_ASSERT_EQUALS(p->getExplanation(), "does this\nand that");
    TS_ASSERT_EQUALS(p->getPictureName(), "");
    TS_ASSERT_EQUALS(p->getCode(), "A");

    p = testee.getFunctionById(1);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getId(), 1);
    TS_ASSERT_EQUALS(p->getName(), "Refinery");
    TS_ASSERT_EQUALS(p->getDescription(), "2-to-1");
    TS_ASSERT_EQUALS(p->getImpliedFunctionId(), -1);
    TS_ASSERT_EQUALS(p->getPictureName(), "ref");
    TS_ASSERT_EQUALS(p->getCode(), "");

    p = testee.getFunctionById(2);
    TS_ASSERT(p != 0);
    TS_ASSERT_EQUALS(p->getId(), 2);
    TS_ASSERT_EQUALS(p->getName(), "AdvancedRefinery");
    TS_ASSERT_EQUALS(p->getDescription(), "improved!");
    TS_ASSERT_EQUALS(p->getImpliedFunctionId(), 1);
    TS_ASSERT_EQUALS(p->getPictureName(), "");
    TS_ASSERT_EQUALS(p->getCode(), "R");

    TS_ASSERT(testee.getFunctionById(3) == 0);

    // Access by name
    TS_ASSERT(testee.getFunctionByName("Alchemy", false) != 0);
    TS_ASSERT(testee.getFunctionByName("ALCHEMY", false) != 0);
    TS_ASSERT(testee.getFunctionByName("alchemy", false) != 0);
    TS_ASSERT(testee.getFunctionByName("alchemy", true) != 0);
    TS_ASSERT(testee.getFunctionByName("al", false) == 0);
    TS_ASSERT(testee.getFunctionByName("al", true) != 0);

    TS_ASSERT(testee.getFunctionByName("adv", false) == 0);
    TS_ASSERT(testee.getFunctionByName("adv", true) != 0);

    TS_ASSERT(testee.getFunctionByName("2", true) == 0);

    // Access by index
    TS_ASSERT_EQUALS(testee.getNumFunctions(), 3U);
    TS_ASSERT_EQUALS(testee.getFunctionByIndex(0)->getName(), "Alchemy");
    TS_ASSERT_EQUALS(testee.getFunctionByIndex(2)->getName(), "AdvancedRefinery");
    TS_ASSERT(testee.getFunctionByIndex(3) == 0);

    // Clear
    testee.clear();
    TS_ASSERT(testee.getFunctionByName("Alchemy", false) == 0);
    TS_ASSERT(testee.getFunctionById(1) == 0);
    TS_ASSERT_EQUALS(testee.getNumFunctions(), 0U);
    TS_ASSERT(testee.getFunctionByIndex(3) == 0);
}

/** Test matchFunction(). */
void
TestGameSpecBasicHullFunctionList::testMatch()
{
    // Build a definition list:
    game::spec::BasicHullFunctionList testee;

    // 0 is alone
    testee.addFunction(0, "Alchemy");

    // 3->4->2->1
    testee.addFunction(1, "Refinery");
    testee.addFunction(2, "AdvancedRefinery")->setImpliedFunctionId(1);
    testee.addFunction(3, "UltraAdvancedRefinery")->setImpliedFunctionId(4);
    testee.addFunction(4, "SuperAdvancedRefinery")->setImpliedFunctionId(2);

    // Self-match
    TS_ASSERT(testee.matchFunction(99, 99));

    // Nonexistant does not match
    TS_ASSERT(!testee.matchFunction(98, 1));

    // Match all functions against each other
    TS_ASSERT( testee.matchFunction(1, 1));
    TS_ASSERT( testee.matchFunction(1, 2));
    TS_ASSERT( testee.matchFunction(1, 3));
    TS_ASSERT( testee.matchFunction(1, 4));

    TS_ASSERT(!testee.matchFunction(2, 1));
    TS_ASSERT( testee.matchFunction(2, 2));
    TS_ASSERT( testee.matchFunction(2, 3));
    TS_ASSERT( testee.matchFunction(2, 4));

    TS_ASSERT(!testee.matchFunction(3, 1));
    TS_ASSERT(!testee.matchFunction(3, 2));
    TS_ASSERT( testee.matchFunction(3, 3));
    TS_ASSERT(!testee.matchFunction(3, 4));

    TS_ASSERT(!testee.matchFunction(4, 1));
    TS_ASSERT(!testee.matchFunction(4, 2));
    TS_ASSERT( testee.matchFunction(4, 3));
    TS_ASSERT( testee.matchFunction(4, 4));


}

/** Test handling of looping "implies" chains. */
void
TestGameSpecBasicHullFunctionList::testMatchLoop()
{
    game::spec::BasicHullFunctionList testee;

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
            TS_ASSERT(testee.matchFunction(src+10, dst));
            TS_ASSERT(testee.matchFunction(src+10, dst+10));
            TS_ASSERT(testee.matchFunction(src+10, dst+20));
        }
    }

    // Unsuccessful links
    // (Test failure means this hangs.)
    TS_ASSERT(!testee.matchFunction( 0, 10));
    TS_ASSERT(!testee.matchFunction( 1, 10));
    TS_ASSERT(!testee.matchFunction( 2, 10));
    TS_ASSERT(!testee.matchFunction(20, 10));
    TS_ASSERT(!testee.matchFunction( 0,  1));
}

/** Test handling of unterminated "implies" chains. */
void
TestGameSpecBasicHullFunctionList::testMatchUnterminated()
{
    game::spec::BasicHullFunctionList testee;

    // Unterminated chain
    testee.addFunction(2, "A")->setImpliedFunctionId(1);
    testee.addFunction(3, "U")->setImpliedFunctionId(4);
    testee.addFunction(4, "S")->setImpliedFunctionId(2);

    // Because we do not need to resolve the final function, these tests still work
    // (These are the same tests as in testMatch).
    TS_ASSERT( testee.matchFunction(1, 1));
    TS_ASSERT( testee.matchFunction(1, 2));
    TS_ASSERT( testee.matchFunction(1, 3));
    TS_ASSERT( testee.matchFunction(1, 4));

    TS_ASSERT(!testee.matchFunction(2, 1));
    TS_ASSERT( testee.matchFunction(2, 2));
    TS_ASSERT( testee.matchFunction(2, 3));
    TS_ASSERT( testee.matchFunction(2, 4));

    TS_ASSERT(!testee.matchFunction(3, 1));
    TS_ASSERT(!testee.matchFunction(3, 2));
    TS_ASSERT( testee.matchFunction(3, 3));
    TS_ASSERT(!testee.matchFunction(3, 4));

    TS_ASSERT(!testee.matchFunction(4, 1));
    TS_ASSERT(!testee.matchFunction(4, 2));
    TS_ASSERT( testee.matchFunction(4, 3));
    TS_ASSERT( testee.matchFunction(4, 4));

    // A nonexistant target
    TS_ASSERT(!testee.matchFunction(4, 9));
    TS_ASSERT(!testee.matchFunction(9, 4));
}

/** Test errors when loading. */
void
TestGameSpecBasicHullFunctionList::testErrors()
{
    using afl::io::ConstMemoryStream;
    using afl::string::toBytes;
    using game::spec::BasicHullFunctionList;

    afl::string::NullTranslator tx;

    // Syntax error in line
    {
        ConstMemoryStream ms(toBytes("\nhi mom\n"));
        afl::test::LogListener log;
        BasicHullFunctionList().load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
    }

    // Syntax error in line
    {
        ConstMemoryStream ms(toBytes("\n1,foo\n"));
        afl::test::LogListener log;
        BasicHullFunctionList().load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
    }

    // Invalid number
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "999999,a,improvedalchemy\n"
                                     "d=i\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
        TS_ASSERT(hfl.getFunctionById(999999) == 0);
        TS_ASSERT_EQUALS(hfl.getFunctionById(1)->getDescription(), "alchemy");
    }

    // Duplicate name
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "d=one\n"
                                     "2,a,alchemy\n"
                                     "d=two\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
        TS_ASSERT(hfl.getFunctionById(2) == 0);
        TS_ASSERT_EQUALS(hfl.getFunctionById(1)->getDescription(), "one");
    }

    // Duplicate Id
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "d=one\n"
                                     "1,a,somethingelse\n"
                                     "d=two\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
        TS_ASSERT_EQUALS(hfl.getFunctionById(1)->getDescription(), "one");
    }

    // Missing function
    {
        ConstMemoryStream ms(toBytes("d=one\n"));
        afl::test::LogListener log;
        BasicHullFunctionList().load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
    }

    // Bad implication - invalid name
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "i=foo\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
        TS_ASSERT(hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
    }

    // Bad implication - self reference
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "i=1\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(hfl.getFunctionById(1) != 0);      // This is not a warning
        TS_ASSERT(hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
    }

    // Bad implication - self reference by name
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "i=alchemy\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(hfl.getFunctionById(1) != 0);      // This is not a warning
        TS_ASSERT(hfl.getFunctionById(1)->getImpliedFunctionId() == -1);
    }

    // Bad standard assignment
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "s=x\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
    }

    // Bad standard assignment, case 2
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "s=3,4,x\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
    }

    // Bad standard assignment, case 3
    {
        ConstMemoryStream ms(toBytes("1,a,alchemy\n"
                                     "s=5,3,1,-1\n"));
        afl::test::LogListener log;
        BasicHullFunctionList hfl;
        hfl.load(ms, tx, log);
        TS_ASSERT(log.getNumMessages() > 0);
        TS_ASSERT(hfl.getFunctionById(1) != 0);
    }
}

/** Test bug 342.
    This should already be covered by the other tests. */
void
TestGameSpecBasicHullFunctionList::testBug342()
{
    // Default-construct an object
    game::spec::BasicHullFunctionList testee;

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
        afl::io::ConstMemoryStream ms(afl::string::toBytes(SAMPLE_FILE));
        afl::string::NullTranslator tx;
        afl::sys::Log log;
        TS_ASSERT_THROWS_NOTHING(testee.load(ms, tx, log));
    }

    // Verify content
    TS_ASSERT(testee.getFunctionById(1) != 0);
    TS_ASSERT(testee.getFunctionById(4) != 0);
    TS_ASSERT(testee.getFunctionById(7) != 0);
    TS_ASSERT(testee.getFunctionById(29) != 0);

    TS_ASSERT(testee.matchFunction(1, 4));
    TS_ASSERT(testee.matchFunction(4, 1));
    TS_ASSERT(!testee.matchFunction(16, 4));  // This used to hang: client asks for Cloak, having found CoolsTo50
    TS_ASSERT(!testee.matchFunction(4, 16));

    TS_ASSERT_EQUALS(testee.getFunctionById(7)->getDescription(), "Seven described");
    TS_ASSERT_EQUALS(testee.getFunctionById(29)->getDescription(), "Twentynine described");
}

