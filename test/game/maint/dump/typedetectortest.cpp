/**
  *  \file test/game/maint/dump/typedetectortest.cpp
  *  \brief Test for game::maint::dump::TypeDetector
  */

#include "game/maint/dump/typedetector.hpp"

#include "afl/test/testrunner.hpp"

using game::maint::dump::Input;
using game::maint::dump::Output;
using game::maint::dump::TypeDetector;

namespace {
    // Parser functions for testing
    void parserOne(Input&,Output&) { }
    void parserTwo(Input&,Output&) { }
    void parserThree(Input&,Output&) { }
    void parserFour(Input&,Output&) { }

    void doMatch(TypeDetector& testee)
    {
        testee.start();
        testee.match("s", "ship",  parserOne);
        testee.match("b", "bdata", parserTwo);
        testee.match("p", "pdata", parserThree);
        testee.match("q", 0,       parserFour);
    }
}

/* Baseline test */
AFL_TEST("game.maint.dump.TypeDetector:base", a)
{
    TypeDetector testee;
    testee.setFileBaseName("bdata3.dat");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 1U);
    a.checkEqual("getType", testee.getType(), "b");
    a.check("getParser", testee.getParser() == parserTwo);
    a.check("hasRequiredType", !testee.hasRequiredType());
}

/* Test with required type set */
AFL_TEST("game.maint.dump.TypeDetector:setRequiredType", a)
{
    TypeDetector testee;
    testee.setFileBaseName("bdata3.dat");
    testee.setRequiredType("s");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 1U);
    a.checkEqual("getType", testee.getType(), "s");
    a.check("getParser", testee.getParser() == parserOne);
    a.check("hasRequiredType", testee.hasRequiredType());
    a.checkEqual("getRequiredType", testee.getRequiredType(), "s");
}

/* Test with required type set; file has no basename */
AFL_TEST("game.maint.dump.TypeDetector:setRequiredType:no-basename", a)
{
    TypeDetector testee;
    testee.setFileBaseName("bdata3.dat");
    testee.setRequiredType("q");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 1U);
    a.checkEqual("getType", testee.getType(), "q");
    a.check("getParser", testee.getParser() == parserFour);
    a.check("hasRequiredType", testee.hasRequiredType());
    a.checkEqual("getRequiredType", testee.getRequiredType(), "q");
}

/* Mismatch with no required type set */
AFL_TEST("game.maint.dump.TypeDetector:fail", a)
{
    TypeDetector testee;
    testee.setFileBaseName("qdata3.dat");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 0U);
    a.checkEqual("getType", testee.getType(), "");
    a.check("getParser", testee.getParser() == 0);
    a.check("hasRequiredType", !testee.hasRequiredType());
}

/* Mismatch with required type set */
AFL_TEST("game.maint.dump.TypeDetector:fail:setRequiredType", a)
{
    TypeDetector testee;
    testee.setFileBaseName("bdata3.dat");
    testee.setRequiredType("x");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 0U);
    a.checkEqual("getType", testee.getType(), "");
    a.check("getParser", testee.getParser() == 0);
    a.check("hasRequiredType", testee.hasRequiredType());
    a.checkEqual("getRequiredType", testee.getRequiredType(), "x");
}

/* Near mismatch */
AFL_TEST("game.maint.dump.TypeDetector:fail:near", a)
{
    TypeDetector testee;
    testee.setFileBaseName("bdataxyz.dat");
    doMatch(testee);

    a.checkEqual("getNumPossibleTypes", testee.getNumPossibleTypes(), 0U);
    a.checkEqual("getType", testee.getType(), "");
    a.check("getParser", testee.getParser() == 0);
    a.check("hasRequiredType", !testee.hasRequiredType());
}

