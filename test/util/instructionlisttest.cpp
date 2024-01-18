/**
  *  \file test/util/instructionlisttest.cpp
  *  \brief Test for util::InstructionList
  */

#include "util/instructionlist.hpp"
#include "afl/test/testrunner.hpp"

/** Simple test.
    Written instructions must be retrievable. */
AFL_TEST("util.InstructionList:basics", a)
{
    util::InstructionList testee;
    util::InstructionList::Instruction_t i = 0;
    util::InstructionList::Parameter_t p = 0;

    // Initial state
    a.checkEqual("01. size",            testee.size(), 0U);
    a.checkEqual("02. readInstruction", testee.read().readInstruction(i), false);
    a.checkEqual("03. readParameter",   testee.read().readParameter(p), false);

    // Add two instructions
    testee.addInstruction(12);
    testee.addParameter(3);
    testee.addInstruction(99);
    testee.addParameter(12);
    testee.addParameter(7);

    // Read
    a.check("11. size", testee.size() >= 2U);
    util::InstructionList::Iterator it = testee.read();

    // - readParameter() before readInstruction() fails
    a.checkEqual("21", it.readParameter(p), false);

    // - read first instruction
    a.checkEqual("31", it.readInstruction(i), true);
    a.checkEqual("32", i, 12);
    a.checkEqual("33", it.readParameter(p), true);
    a.checkEqual("34", p, 3);
    a.checkEqual("35", it.readParameter(p), false);

    // - read second instruction
    a.checkEqual("41", it.readInstruction(i), true);
    a.checkEqual("42", i, 99);
    a.checkEqual("43", it.readParameter(p), true);
    a.checkEqual("44", p, 12);
    a.checkEqual("45", it.readParameter(p), true);
    a.checkEqual("46", p, 7);
    a.checkEqual("47", it.readParameter(p), false);

    // - end
    a.checkEqual("51", it.readInstruction(i), false);
}

/** Simple test, read instructions only.
    Written instructions must be retrievable even if parameters are not read. */
AFL_TEST("util.InstructionList:readInstruction:ignore-parameters", a)
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
    a.checkEqual("01", it.readInstruction(i), true);
    a.checkEqual("02", i, 12);
    a.checkEqual("03", it.readInstruction(i), true);
    a.checkEqual("04", i, 99);
    a.checkEqual("05", it.readInstruction(i), false);
}

/** Test append(). */
AFL_TEST("util.InstructionList:append", a)
{
    // Two lists
    util::InstructionList ia;
    util::InstructionList ib;
    ia.addInstruction(3);
    ia.addParameter(4);
    ib.addInstruction(7);
    ib.addParameter(6);
    ib.addInstruction(9);

    // Append
    ia.append(ib);

    // Verify
    util::InstructionList::Instruction_t i = 0;
    util::InstructionList::Parameter_t p = 0;
    util::InstructionList::Iterator it = ia.read();

    // - read first instruction
    a.checkEqual("01", it.readInstruction(i), true);
    a.checkEqual("02", i, 3);
    a.checkEqual("03", it.readParameter(p), true);
    a.checkEqual("04", p, 4);
    a.checkEqual("05", it.readParameter(p), false);

    // - read second instruction
    a.checkEqual("11", it.readInstruction(i), true);
    a.checkEqual("12", i, 7);
    a.checkEqual("13", it.readParameter(p), true);
    a.checkEqual("14", p, 6);
    a.checkEqual("15", it.readParameter(p), false);

    // - read third instruction
    a.checkEqual("21", it.readInstruction(i), true);
    a.checkEqual("22", i, 9);
    a.checkEqual("23", it.readParameter(p), false);

    // - end
    a.checkEqual("31", it.readInstruction(i), false);
}
