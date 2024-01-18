/**
  *  \file test/util/stringinstructionlisttest.cpp
  *  \brief Test for util::StringInstructionList
  */

#include "util/stringinstructionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test. */
AFL_TEST("util.StringInstructionList:basics", a)
{
    util::StringInstructionList testee;
    testee.addInstruction(3);
    testee.addParameter(77);
    testee.addStringParameter("hi");
    testee.addInstruction(12);

    // Verify
    a.check("01", testee.size() >= 2U);

    // Read it
    util::StringInstructionList::Iterator it = testee.read();
    util::StringInstructionList::Instruction_t i = 0;
    util::StringInstructionList::Parameter_t p = 0;
    String_t s;

    // - first
    a.checkEqual("11", it.readInstruction(i), true);
    a.checkEqual("12", i, 3);
    a.checkEqual("13", it.readParameter(p), true);
    a.checkEqual("14", p, 77);
    a.checkEqual("15", it.readStringParameter(s), true);
    a.checkEqual("16", s, "hi");

    // - second
    a.checkEqual("21", it.readInstruction(i), true);
    a.checkEqual("22", i, 12);
    a.checkEqual("23", it.readParameter(p), false);
    a.checkEqual("24", it.readStringParameter(s), false);

    // - end
    a.checkEqual("31", it.readInstruction(i), false);
}

/** Test reading wrong type. */
AFL_TEST("util.StringInstructionList:read-wrong-type", a)
{
    util::StringInstructionList testee;
    testee.addInstruction(3);
    testee.addParameter(77);
    testee.addStringParameter("hi");

    // Read it
    util::StringInstructionList::Iterator it = testee.read();
    util::StringInstructionList::Instruction_t i = 0;
    String_t s;

    // - cannot read strings now
    a.checkEqual("01", it.readStringParameter(s), false);

    // - attempt to read a string when it's a number
    a.checkEqual("11", it.readInstruction(i), true);
    a.checkEqual("12", i, 3);
    a.checkEqual("13", it.readStringParameter(s), false);
}

AFL_TEST("util.StringInstructionList:swap", a)
{
    util::StringInstructionList la;
    la.addInstruction(1);
    la.addStringParameter("foo");

    util::StringInstructionList lb;
    lb.addInstruction(99);
    lb.addStringParameter("xyzzy");
    lb.addStringParameter("q");

    la.swap(lb);

    util::StringInstructionList::Instruction_t i = 0;
    String_t s;

    // Read a
    {
        util::StringInstructionList::Iterator it = la.read();
        a.checkEqual("01", it.readInstruction(i), true);
        a.checkEqual("02", i, 99);
        a.checkEqual("03", it.readStringParameter(s), true);
        a.checkEqual("04", s, "xyzzy");
        a.checkEqual("05", it.readStringParameter(s), true);
        a.checkEqual("06", s, "q");
    }

    // Read b
    {
        util::StringInstructionList::Iterator it = lb.read();
        a.checkEqual("11", it.readInstruction(i), true);
        a.checkEqual("12", i, 1);
        a.checkEqual("13", it.readStringParameter(s), true);
        a.checkEqual("14", s, "foo");
    }
}
