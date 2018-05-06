/**
  *  \file u/t_util_instructionlist.cpp
  *  \brief Test for util::InstructionList
  */

#include "util/instructionlist.hpp"

#include "t_util.hpp"

/** Simple test.
    Written instructions must be retrievable. */
void
TestUtilInstructionList::testIt()
{
    util::InstructionList testee;
    util::InstructionList::Instruction_t i = 0;
    util::InstructionList::Parameter_t p = 0;

    // Initial state
    TS_ASSERT_EQUALS(testee.size(), 0U);
    TS_ASSERT_EQUALS(testee.read().readInstruction(i), false);
    TS_ASSERT_EQUALS(testee.read().readParameter(p), false);

    // Add two instructions
    testee.addInstruction(12);
    testee.addParameter(3);
    testee.addInstruction(99);
    testee.addParameter(12);
    testee.addParameter(7);

    // Read
    TS_ASSERT(testee.size() >= 2U);
    util::InstructionList::Iterator it = testee.read();

    // - readParameter() before readInstruction() fails
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - read first instruction
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 12);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 3);
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - read second instruction
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 99);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 12);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 7);
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - end
    TS_ASSERT_EQUALS(it.readInstruction(i), false);
}

/** Simple test, read instructions only.
    Written instructions must be retrievable even if parameters are not read. */
void
TestUtilInstructionList::testReadInsnOnly()
{
    util::InstructionList testee;
    util::InstructionList::Instruction_t i = 0;

    // Add two instructions
    testee.addInstruction(12);
    testee.addParameter(3);
    testee.addInstruction(99);
    testee.addParameter(12);
    testee.addParameter(7);

    // Read
    util::InstructionList::Iterator it = testee.read();
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 12);
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 99);
    TS_ASSERT_EQUALS(it.readInstruction(i), false);
}

/** Test append(). */
void
TestUtilInstructionList::testAppend()
{
    // Two lists
    util::InstructionList a;
    util::InstructionList b;
    a.addInstruction(3);
    a.addParameter(4);
    b.addInstruction(7);
    b.addParameter(6);
    b.addInstruction(9);

    // Append
    a.append(b);

    // Verify
    util::InstructionList::Instruction_t i = 0;
    util::InstructionList::Parameter_t p = 0;
    util::InstructionList::Iterator it = a.read();

    // - read first instruction
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 3);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 4);
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - read second instruction
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 7);
    TS_ASSERT_EQUALS(it.readParameter(p), true);
    TS_ASSERT_EQUALS(p, 6);
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - read third instruction
    TS_ASSERT_EQUALS(it.readInstruction(i), true);
    TS_ASSERT_EQUALS(i, 9);
    TS_ASSERT_EQUALS(it.readParameter(p), false);

    // - end
    TS_ASSERT_EQUALS(it.readInstruction(i), false);
}

