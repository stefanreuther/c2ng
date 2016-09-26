/**
  *  \file u/t_interpreter_bytecodeobject.cpp
  *  \brief Test for interpreter::BytecodeObject
  */

#include "interpreter/bytecodeobject.hpp"

#include "t_interpreter.hpp"

/** Test getter/setters. */
void
TestInterpreterBytecodeObject::testGet()
{
    interpreter::BytecodeObject testee;

    // Procedure flag: default is enabled
    TS_ASSERT(testee.isProcedure());
    testee.setIsProcedure(false);
    TS_ASSERT(!testee.isProcedure());

    // Varargs flag: default is disabled
    TS_ASSERT(!testee.isVarargs());
    testee.setIsVarargs(true);
    TS_ASSERT(testee.isVarargs());

    // Name: default is empty
    TS_ASSERT_EQUALS(testee.getName(), "");
    testee.setName("HI");
    TS_ASSERT_EQUALS(testee.getName(), "HI");

    // File name: default is empty
    TS_ASSERT_EQUALS(testee.getFileName(), "");
    testee.setFileName("test.q");
    TS_ASSERT_EQUALS(testee.getFileName(), "test.q");
}

/** Test arguments. */
void
TestInterpreterBytecodeObject::testArgs()
{
    interpreter::BytecodeObject testee;

    // Default is no args, no varargs
    TS_ASSERT_EQUALS(testee.getMinArgs(), size_t(0));
    TS_ASSERT_EQUALS(testee.getMaxArgs(), size_t(0));
    TS_ASSERT(!testee.isVarargs());

    // Add some args
    testee.addArgument("A", false);
    testee.addArgument("B", false);
    TS_ASSERT_EQUALS(testee.getMinArgs(), size_t(2));
    TS_ASSERT_EQUALS(testee.getMaxArgs(), size_t(2));
    TS_ASSERT(!testee.isVarargs());

    // Add some optional args
    testee.addArgument("C", false);
    testee.addArgument("D", false);
    testee.addArgument("E", true);
    testee.addArgument("F", true);
    TS_ASSERT_EQUALS(testee.getMinArgs(), size_t(4));
    TS_ASSERT_EQUALS(testee.getMaxArgs(), size_t(6));
    TS_ASSERT(!testee.isVarargs());

    // Varargs are local variables
    TS_ASSERT(testee.hasLocalVariable("A"));
    TS_ASSERT(testee.hasLocalVariable("B"));
    TS_ASSERT(testee.hasLocalVariable("C"));
    TS_ASSERT(testee.hasLocalVariable("D"));
    TS_ASSERT(testee.hasLocalVariable("F"));
    TS_ASSERT(testee.hasLocalVariable("F"));
}

/** Test copyLocalVariablesFrom. */
void
TestInterpreterBytecodeObject::testCopyLocalVariablesFrom()
{
    // Prepare object X with 3 names
    interpreter::BytecodeObject x;
    x.addLocalVariable("A");
    x.addLocalVariable("B");
    x.addLocalVariable("C");

    // Prepare object Y with another 3 names
    interpreter::BytecodeObject y;
    y.addLocalVariable("C");
    y.addLocalVariable("D");
    y.addLocalVariable("E");

    // Merge them
    x.copyLocalVariablesFrom(y);
    TS_ASSERT(x.hasLocalVariable("A"));
    TS_ASSERT(x.hasLocalVariable("B"));
    TS_ASSERT(x.hasLocalVariable("C"));
    TS_ASSERT(x.hasLocalVariable("D"));
    TS_ASSERT(x.hasLocalVariable("E"));

    // This copies! That is, we now have C twice.
    TS_ASSERT_EQUALS(x.getLocalNames().getNumNames(), size_t(6));
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(0), "A");
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(1), "B");
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(2), "C");
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(3), "C");
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(4), "D");
    TS_ASSERT_EQUALS(x.getLocalNames().getNameByIndex(5), "E");
}

/** Test labels. */
void
TestInterpreterBytecodeObject::testLabel()
{
    using interpreter::Opcode;
    interpreter::BytecodeObject testee;

    // Make two labels
    interpreter::BytecodeObject::Label_t a = testee.makeLabel();
    interpreter::BytecodeObject::Label_t b = testee.makeLabel();
    TS_ASSERT_DIFFERS(a, b);

    // Generate some code
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLabel(a);
    testee.addJump(Opcode::jAlways, b);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLabel(b);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addJump(Opcode::jAlways, a);
    TS_ASSERT_EQUALS(testee.getNumInstructions(), size_t(7));

    // Last jump (0,a) jumps to instruction 1
    TS_ASSERT_EQUALS(testee.getJumpTarget(testee(6).minor, testee(6).arg), size_t(1));

    // First jump (0,b) jumps to instruction 4
    TS_ASSERT_EQUALS(testee.getJumpTarget(testee(2).minor, testee(2).arg), size_t(4));

    // Relocate. Code will look like this:
    //         insn
    //   a:    j b
    //         insn
    //   b:    insn
    //         j a
    testee.relocate();
    TS_ASSERT_EQUALS(testee.getNumInstructions(), size_t(5));
    TS_ASSERT_EQUALS(testee.getJumpTarget(testee(1).minor, testee(1).arg), size_t(3));
    TS_ASSERT_EQUALS(testee.getJumpTarget(testee(4).minor, testee(4).arg), size_t(1));
}
