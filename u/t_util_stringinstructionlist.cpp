/**
  *  \file u/t_util_stringinstructionlist.cpp
  *  \brief Test for util::StringInstructionList
  */

#include "util/stringinstructionlist.hpp"

#include "t_util.hpp"

/** Simple test. */
void
TestUtilStringInstructionList::testIt()
{
    util::StringInstructionList testee;
    testee.addInstruction(3);
    testee.addParameter(77);
    testee.addStringParameter("hi");
    testee.addInstruction(12);

    // Verify
    TS_ASSERT(testee.size() >= 2U);

    // Read it
    util::StringInstructionList::Iterator it = testee.read();
    util::StringInstructionList::Instruction_t i = 0;
    util::StringInstructionList::Parameter_t p = 0;
    String_t s;

    // - first
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 3);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 77);
    TS_ASSERT_EQUALS(it.readStringParameter(s), true);
    TS_ASSERT_EQUALS(s, "hi");

    // - second
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 12);
    TS_ASSERT_EQUALS(it.readParameter(p), false);
    TS_ASSERT_EQUALS(it.readStringParameter(s), false);

    // - end
    TS_ASSERT_EQUALS(it.readInstruction(i), false);
}

/** Test reading wrong type. */
void
TestUtilStringInstructionList::testReadWrong()
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
    TS_ASSERT_EQUALS(it.readStringParameter(s), false);

    // - attempt to read a string when it's a number
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 3);
    TS_ASSERT_EQUALS(it.readStringParameter(s), false);
}

void
TestUtilStringInstructionList::testSwap()
{
    util::StringInstructionList a;
    a.addInstruction(1);
    a.addStringParameter("foo");

    util::StringInstructionList b;
    b.addInstruction(99);
    b.addStringParameter("xyzzy");
    b.addStringParameter("q");

    a.swap(b);

    util::StringInstructionList::Instruction_t i = 0;
    String_t s;

    // Read a
    {
        util::StringInstructionList::Iterator it = a.read();
        TS_ASSERT_EQUALS(it.readInstruction(i), true);
        TS_ASSERT_EQUALS(i, 99);
        TS_ASSERT_EQUALS(it.readStringParameter(s), true);
        TS_ASSERT_EQUALS(s, "xyzzy");
        TS_ASSERT_EQUALS(it.readStringParameter(s), true);
        TS_ASSERT_EQUALS(s, "q");
    }

    // Read b
    {
        util::StringInstructionList::Iterator it = b.read();
        TS_ASSERT_EQUALS(it.readInstruction(i), true);
        TS_ASSERT_EQUALS(i, 1);
        TS_ASSERT_EQUALS(it.readStringParameter(s), true);
        TS_ASSERT_EQUALS(s, "foo");
    }
}
