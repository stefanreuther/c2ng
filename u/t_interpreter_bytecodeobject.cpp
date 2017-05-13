/**
  *  \file u/t_interpreter_bytecodeobject.cpp
  *  \brief Test for interpreter::BytecodeObject
  */

#include "interpreter/bytecodeobject.hpp"

#include "t_interpreter.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "afl/data/integervalue.hpp"
#include "interpreter/error.hpp"

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

/** Test addPushLiteral(). */
void
TestInterpreterBytecodeObject::testLiteral()
{
    interpreter::BytecodeObject testee;

    // Add 1000 = 100x10 string literals.
    // These should be re-used, generating a pool of only 10.
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 10; ++j) {
            afl::data::StringValue sv(afl::string::Format("literal %d", j));
            testee.addPushLiteral(&sv);
        }
    }
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 1000U);
    TS_ASSERT_EQUALS(testee.getLiterals().size(), 10U);
}

/** Test addPushLiteral() with integer literals. */
void
TestInterpreterBytecodeObject::testIntLiteral()
{
    interpreter::BytecodeObject testee;

    // Add 1000 = 100x10 huge integer literals.
    // These should be re-used, generating a pool of only 10.
    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 10; ++j) {
            afl::data::IntegerValue sv(1000000 + j);
            testee.addPushLiteral(&sv);
        }
    }
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 1000U);
    TS_ASSERT_EQUALS(testee.getLiterals().size(), 10U);

    // Add 1000 small integer literals.
    // These should not affect the literal pool
    for (int j = 0; j < 1000; ++j) {
        afl::data::IntegerValue sv(j);
        testee.addPushLiteral(&sv);
    }
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 2000U);
    TS_ASSERT_EQUALS(testee.getLiterals().size(), 10U);
}

/** Test overflow in makeLabel(). */
void
TestInterpreterBytecodeObject::testMakeLabelOverflow()
{
    interpreter::BytecodeObject testee;

    // The limit is 65535, because we need to express "number of labels" as a uint16_t.
    TS_ASSERT_EQUALS(testee.getNumLabels(), 0U);
    for (int32_t i = 0; i < 65535; ++i) {
        testee.makeLabel();
    }
    TS_ASSERT_EQUALS(testee.getNumLabels(), 65535U);
    TS_ASSERT_THROWS(testee.makeLabel(), interpreter::Error);
}

/** Test overflow in makeLabel(). Shortcut with setting the starting point. */
void
TestInterpreterBytecodeObject::testMakeLabelOverflow2()
{
    interpreter::BytecodeObject testee;

    // The limit is 65535, because we need to express "number of labels" as a uint16_t.
    TS_ASSERT_EQUALS(testee.getNumLabels(), 0U);
    testee.setNumLabels(20000);
    TS_ASSERT_EQUALS(testee.getNumLabels(), 20000U);
    for (int32_t i = 20000; i < 65535; ++i) {
        testee.makeLabel();
    }
    TS_ASSERT_EQUALS(testee.getNumLabels(), 65535U);
    TS_ASSERT_THROWS(testee.makeLabel(), interpreter::Error);
}

/** Test overflow in addName(). */
void
TestInterpreterBytecodeObject::testAddNameOverflow()
{
    interpreter::BytecodeObject testee;

    // The limit is 65536, because valid name indexes are [0,65535].
    TS_ASSERT_EQUALS(testee.getNames().getNumNames(), 0U);
    for (int32_t i = 0; i < 65536; ++i) {
        String_t name = afl::string::Format("NAME%d", i);
        TS_ASSERT(!testee.hasName(name));
        testee.addName(name);
        TS_ASSERT(testee.hasName(name));
    }
    TS_ASSERT_EQUALS(testee.getNames().getNumNames(), 65536U);
    TS_ASSERT_THROWS(testee.addName("FOO"), interpreter::Error);
}

/** Test general name access. */
void
TestInterpreterBytecodeObject::testNames()
{
    interpreter::BytecodeObject testee;

    uint16_t a = testee.addName("A");
    uint16_t b = testee.addName("B");
    TS_ASSERT_DIFFERS(a, b);
    TS_ASSERT_EQUALS(testee.addName("A"), a);
    TS_ASSERT_EQUALS(testee.addName("B"), b);
    TS_ASSERT(testee.hasName("A"));
    TS_ASSERT(testee.hasName("B"));
    TS_ASSERT(!testee.hasName("C"));
    TS_ASSERT_EQUALS(testee.getName(a), "A");
    TS_ASSERT_EQUALS(testee.getName(b), "B");
}

/** Test line number handling. */
void
TestInterpreterBytecodeObject::testLineNumbers()
{
    using interpreter::Opcode;
    interpreter::BytecodeObject testee;

    // Generate some code.
    testee.addLineNumber(10);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(11);
    testee.addLineNumber(12);
    testee.addLineNumber(13);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(13);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(14);

    // Result should be: line 10 at 0, 13 at 2, 14 at 5
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 5U);
    TS_ASSERT_EQUALS(testee.getLineNumber(0), 10U);
    TS_ASSERT_EQUALS(testee.getLineNumber(1), 10U);
    TS_ASSERT_EQUALS(testee.getLineNumber(2), 13U);
    TS_ASSERT_EQUALS(testee.getLineNumber(3), 13U);
    TS_ASSERT_EQUALS(testee.getLineNumber(5), 14U);
    TS_ASSERT_EQUALS(testee.getLineNumber(6), 14U);
    TS_ASSERT_EQUALS(testee.getLineNumber(100), 14U);

    // Check storage format
    const std::vector<uint32_t>& rep = testee.getLineNumbers();
    TS_ASSERT_EQUALS(rep.size(), 6U);
    TS_ASSERT_EQUALS(rep[0],  0U);
    TS_ASSERT_EQUALS(rep[1], 10U);
    TS_ASSERT_EQUALS(rep[2],  2U);
    TS_ASSERT_EQUALS(rep[3], 13U);
    TS_ASSERT_EQUALS(rep[4],  5U);
    TS_ASSERT_EQUALS(rep[5], 14U);
}

/** Test line number handling, second case. */
void
TestInterpreterBytecodeObject::testLineNumbers2()
{
    interpreter::BytecodeObject testee;
    using interpreter::Opcode;

    // Generate some code.
    // [Same thing as above, but missing the "line 10" entry.]
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(11);
    testee.addLineNumber(12);
    testee.addLineNumber(13);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(13);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(14);

    // Result should be: no line before 2, 13 at 2, 14 at 5
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 5U);
    TS_ASSERT_EQUALS(testee.getLineNumber(0), 0U);
    TS_ASSERT_EQUALS(testee.getLineNumber(1), 0U);
    TS_ASSERT_EQUALS(testee.getLineNumber(2), 13U);
    TS_ASSERT_EQUALS(testee.getLineNumber(3), 13U);
    TS_ASSERT_EQUALS(testee.getLineNumber(5), 14U);
    TS_ASSERT_EQUALS(testee.getLineNumber(6), 14U);
    TS_ASSERT_EQUALS(testee.getLineNumber(100), 14U);

    // Check storage format
    const std::vector<uint32_t>& rep = testee.getLineNumbers();
    TS_ASSERT_EQUALS(rep.size(), 4U);
    TS_ASSERT_EQUALS(rep[0],  2U);
    TS_ASSERT_EQUALS(rep[1], 13U);
    TS_ASSERT_EQUALS(rep[2],  5U);
    TS_ASSERT_EQUALS(rep[3], 14U);
}

/** Test line number handling, restore from storage. */
void
TestInterpreterBytecodeObject::testLineNumbers3()
{
    interpreter::BytecodeObject testee;

    // Restore from storage format
    testee.addLineNumber(10, 2);
    testee.addLineNumber(14, 5);
    testee.addLineNumber(15, 6);

    // Verify access
    TS_ASSERT_EQUALS(testee.getLineNumber(0), 0U);
    TS_ASSERT_EQUALS(testee.getLineNumber(1), 0U);
    TS_ASSERT_EQUALS(testee.getLineNumber(2), 10U);
    TS_ASSERT_EQUALS(testee.getLineNumber(3), 10U);
    TS_ASSERT_EQUALS(testee.getLineNumber(5), 14U);
    TS_ASSERT_EQUALS(testee.getLineNumber(6), 15U);
    TS_ASSERT_EQUALS(testee.getLineNumber(7), 15U);
    TS_ASSERT_EQUALS(testee.getLineNumber(100), 15U);
}

/** Test hasUserCall(). */
void
TestInterpreterBytecodeObject::testHasUserCall()
{
    using interpreter::Opcode;

    // Boundary case: empty
    {
        interpreter::BytecodeObject t;
        TS_ASSERT(!t.hasUserCall());
    }

    // Some uncritical instructions
    {
        interpreter::BytecodeObject t;
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0); 
        t.addInstruction(Opcode::maPush,    Opcode::sInteger, 3); 
        t.addInstruction(Opcode::maPop,     Opcode::sLocal, t.addLocalVariable("X"));
        TS_ASSERT(!t.hasUserCall());
    }

    // maIndirect counts as user call because we don't know where it ends up
    {
        interpreter::BytecodeObject t;
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
        t.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
        TS_ASSERT(t.hasUserCall());
    }

    // miSpecialEvalStatement
    {
        interpreter::BytecodeObject t;
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 0);
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
        TS_ASSERT(t.hasUserCall());
    }

    // miSpecialEvalExpr
    {
        interpreter::BytecodeObject t;
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        TS_ASSERT(t.hasUserCall());
    }

    // miSpecialRunHook
    {
        interpreter::BytecodeObject t;
        t.addInstruction(Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
        TS_ASSERT(t.hasUserCall());
    }
}

// FIXME: Untested:
// void     setMinArgs(size_t n);
// void     setMaxArgs(size_t n);

// void     addVariableReferenceInstruction(Opcode::Major major, const String_t& name, const CompilationContext& cc);
// void     compact();
// void     append(const BytecodeObject& other);
// const Opcode& operator()(PC_t index) const;
// String_t getDisassembly(PC_t index, const World& w) const;
// afl::data::Value* getLiteral(uint16_t index) const;
// const afl::data::NameMap& getLocalNames() const;  // FIXME: rename to getLocalVariableNames
// afl::data::NameMap& getLocalNames()
//     { return m_localNames; }
// const afl::data::NameMap& getNames() const
// afl::data::NameMap& getNames()
// const afl::data::Segment& getLiterals() const
// const std::vector<Opcode>& getCode() const
