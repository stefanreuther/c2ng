/**
  *  \file u/t_interpreter_bytecodeobject.cpp
  *  \brief Test for interpreter::BytecodeObject
  */

#include "interpreter/bytecodeobject.hpp"

#include "t_interpreter.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "interpreter/process.hpp"

using interpreter::Opcode;

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
    TS_ASSERT_EQUALS(testee.getSubroutineName(), "");
    testee.setSubroutineName("HI");
    TS_ASSERT_EQUALS(testee.getSubroutineName(), "HI");

    // Origin: default is empty
    TS_ASSERT_EQUALS(testee.getOrigin(), "");
    testee.setOrigin("oh!");
    TS_ASSERT_EQUALS(testee.getOrigin(), "oh!");

    // File name: default is empty
    TS_ASSERT_EQUALS(testee.getFileName(), "");
    testee.setFileName("test.q");
    TS_ASSERT_EQUALS(testee.getFileName(), "test.q");

    // Arguments: default is none
    TS_ASSERT_EQUALS(testee.getMinArgs(), 0U);
    TS_ASSERT_EQUALS(testee.getMaxArgs(), 0U);
    testee.setMinArgs(3);
    testee.setMaxArgs(9);
    TS_ASSERT_EQUALS(testee.getMinArgs(), 3U);
    TS_ASSERT_EQUALS(testee.getMaxArgs(), 9U);
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
    uint16_t a = testee.addArgument("A", false);
    uint16_t b = testee.addArgument("B", false);
    TS_ASSERT_EQUALS(testee.getMinArgs(), size_t(2));
    TS_ASSERT_EQUALS(testee.getMaxArgs(), size_t(2));
    TS_ASSERT(!testee.isVarargs());
    TS_ASSERT_EQUALS(a, 0U);
    TS_ASSERT_EQUALS(b, 1U);

    // Add some optional args
    testee.addArgument("C", false);
    testee.addArgument("D", false);
    testee.addArgument("E", true);
    uint16_t f = testee.addArgument("F", true);
    TS_ASSERT_EQUALS(testee.getMinArgs(), size_t(4));
    TS_ASSERT_EQUALS(testee.getMaxArgs(), size_t(6));
    TS_ASSERT(!testee.isVarargs());
    TS_ASSERT_EQUALS(f, 5U);

    // Varargs are local variables
    TS_ASSERT(testee.hasLocalVariable("A"));
    TS_ASSERT(testee.hasLocalVariable("B"));
    TS_ASSERT(testee.hasLocalVariable("C"));
    TS_ASSERT(testee.hasLocalVariable("D"));
    TS_ASSERT(testee.hasLocalVariable("E"));
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
    TS_ASSERT_EQUALS(x.localVariables().getNumNames(), size_t(6));
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(0), "A");
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(1), "B");
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(2), "C");
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(3), "C");
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(4), "D");
    TS_ASSERT_EQUALS(x.localVariables().getNameByIndex(5), "E");

    // const vs. mutable
    TS_ASSERT_EQUALS(&x.localVariables(), &const_cast<interpreter::BytecodeObject&>(x).localVariables());
}

/** Test labels. */
void
TestInterpreterBytecodeObject::testLabel()
{
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
    TS_ASSERT_EQUALS(testee.literals().size(), 10U);

    // Check literals
    TS_ASSERT(testee.getLiteral(0) != 0);
    TS_ASSERT(dynamic_cast<afl::data::StringValue*>(testee.getLiteral(0)) != 0);
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
    TS_ASSERT_EQUALS(testee.literals().size(), 10U);

    // Add 1000 small integer literals.
    // These should not affect the literal pool
    for (int j = 0; j < 1000; ++j) {
        afl::data::IntegerValue sv(j);
        testee.addPushLiteral(&sv);
    }
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 2000U);
    TS_ASSERT_EQUALS(testee.literals().size(), 10U);
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
    TS_ASSERT_EQUALS(testee.names().getNumNames(), 0U);
    for (int32_t i = 0; i < 65536; ++i) {
        String_t name = afl::string::Format("NAME%d", i);
        TS_ASSERT(!testee.hasName(name));
        testee.addName(name);
        TS_ASSERT(testee.hasName(name));
    }
    TS_ASSERT_EQUALS(testee.names().getNumNames(), 65536U);
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

    // const vs. mutable
    TS_ASSERT_EQUALS(&testee.names(), &const_cast<interpreter::BytecodeObject&>(testee).names());
}

/** Test line number handling. */
void
TestInterpreterBytecodeObject::testLineNumbers()
{
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
    const std::vector<uint32_t>& rep = testee.lineNumbers();
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
    const std::vector<uint32_t>& rep = testee.lineNumbers();
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

/** Test addVariableReferenceInstruction. */
void
TestInterpreterBytecodeObject::testVariableReference()
{
    using interpreter::CompilationContext;

    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    world.setNewGlobalValue("G", interpreter::makeIntegerValue(1));
    world.setNewGlobalValue("S", interpreter::makeIntegerValue(2));

    // Testee
    interpreter::BytecodeObject testee;
    testee.addLocalVariable("L");
    testee.addLocalVariable("S");

    // Add variable with freestanding context. This will generate a 'pushvar'.
    CompilationContext freeContext(world);
    testee.addVariableReferenceInstruction(Opcode::maPush, "L", freeContext);

    // Add variable with local context. This will generate a 'pushloc'.
    CompilationContext localContext(world);
    localContext.withFlag(CompilationContext::LocalContext);
    testee.addVariableReferenceInstruction(Opcode::maPush, "L", localContext);

    // Add variable with local and global context. This will still generate a 'pushloc' due to shadowing.
    CompilationContext globalContext(world);
    globalContext.withFlag(CompilationContext::LocalContext);
    globalContext.withFlag(CompilationContext::AlsoGlobalContext);
    testee.addVariableReferenceInstruction(Opcode::maPush, "S", globalContext);

    // Same thing, but this time we get the global
    testee.addVariableReferenceInstruction(Opcode::maPush, "G", globalContext);

    // Add variable with just global context. This has no effect, we still get 'pushvar'.
    CompilationContext onlyGlobalContext(world);
    onlyGlobalContext.withFlag(CompilationContext::AlsoGlobalContext);
    testee.addVariableReferenceInstruction(Opcode::maPush, "S", onlyGlobalContext);

    // Verify
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 5U);
    TS_ASSERT_EQUALS(testee.code().size(), 5U);

    for (size_t i = 0; i < 5; ++i) {
        TS_ASSERT_EQUALS(testee(i).major, Opcode::maPush);
        TS_ASSERT_EQUALS(const_cast<const interpreter::BytecodeObject&>(testee)(i).major, Opcode::maPush);
    }

    TS_ASSERT_EQUALS(testee(0).minor, Opcode::sNamedVariable);
    TS_ASSERT_EQUALS(testee(1).minor, Opcode::sLocal);
    TS_ASSERT_EQUALS(testee(2).minor, Opcode::sLocal);
    TS_ASSERT_EQUALS(testee(3).minor, Opcode::sShared);
    TS_ASSERT_EQUALS(testee(4).minor, Opcode::sNamedVariable);
}

/** Test compact(). */
void
TestInterpreterBytecodeObject::testCompact()
{
    interpreter::BytecodeObject testee;

    // Generate some code
    //  line 100:  insn
    //             (dummy)
    //             insn
    //  line 200:  insn
    //             (dummy)
    //  line 300:  (dummy)
    //             insn
    testee.addLineNumber(100);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maJump, Opcode::jLabel, 7);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLineNumber(200);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addInstruction(Opcode::maJump, Opcode::jLabel, 7);
    testee.addLineNumber(300);
    testee.addInstruction(Opcode::maJump, Opcode::jLabel, 7);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);

    // Action
    testee.compact();

    // Verify
    TS_ASSERT_EQUALS(testee.getNumInstructions(), 4U);
    TS_ASSERT_EQUALS(testee.code().size(), 4U);
    TS_ASSERT_EQUALS(testee.getLineNumber(0), 100U);
    TS_ASSERT_EQUALS(testee.getLineNumber(1), 100U);
    TS_ASSERT_EQUALS(testee.getLineNumber(2), 200U);
    TS_ASSERT_EQUALS(testee.getLineNumber(3), 300U);
    for (size_t i = 0; i < 4; ++i) {
        TS_ASSERT_EQUALS(testee(i).major, Opcode::maSpecial);
        TS_ASSERT_EQUALS(testee(i).minor, Opcode::miSpecialNewHash);
    }
}

void
TestInterpreterBytecodeObject::testAppend()
{
    // Set up copy target:
    //   1 instruction
    //   2 labels
    //   1 name
    //   2 locals
    interpreter::BytecodeObject a;
    a.addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    a.makeLabel(); a.makeLabel();
    a.addName("HI");
    a.addLocalVariable("L1");
    a.addLocalVariable("L2");

    // Set up copy source:
    interpreter::BytecodeObject b;

    // 1. pushvar (name copied)
    b.addInstruction(Opcode::maPush, Opcode::sNamedVariable, b.addName("N"));

    // 2. pushloc (local transformed)
    b.addInstruction(Opcode::maPush, Opcode::sLocal, b.addLocalVariable("L2"));

    // 3. pushlit (literal copied)
    afl::data::StringValue sv("hi");
    b.addPushLiteral(&sv);

    // 4. pushint (copied)
    b.addInstruction(Opcode::maPush, Opcode::sInteger, 444);

    // 5. uzap (copied)
    b.addInstruction(Opcode::maUnary, interpreter::unZap, 0);

    // 6. symbolic label
    b.addLabel(b.makeLabel());

    // 7. absolute jump
    b.addInstruction(Opcode::maJump, Opcode::jIfEmpty, 1);

    // 8. loadmem (name copied)
    b.addInstruction(Opcode::maMemref, Opcode::miIMLoad, b.addName("HI"));

    // 9. snewhash (copied)
    b.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);

    // 10. sdefshipp (name copied)
    b.addInstruction(Opcode::maSpecial, Opcode::miSpecialDefShipProperty, b.addName("HO"));

    // Do it
    a.append(b);

    // Verify
    TS_ASSERT_EQUALS(a.getNumInstructions(), 11U);
    TS_ASSERT_EQUALS(a.getNumLabels(), 3U);

    TS_ASSERT_EQUALS(a(0).major, Opcode::maPush);
    TS_ASSERT_EQUALS(a(0).minor, Opcode::sInteger);

    TS_ASSERT_EQUALS(a(1).major, Opcode::maPush);
    TS_ASSERT_EQUALS(a(1).minor, Opcode::sNamedVariable);
    TS_ASSERT_EQUALS(a.getName(a(1).arg), "N");

    TS_ASSERT_EQUALS(a(2).major, Opcode::maPush);
    TS_ASSERT_EQUALS(a(2).minor, Opcode::sLocal);
    TS_ASSERT_EQUALS(a.localVariables().getNameByIndex(a(2).arg), "L2");

    TS_ASSERT_EQUALS(a(3).major, Opcode::maPush);
    TS_ASSERT_EQUALS(a(3).minor, Opcode::sLiteral);
    TS_ASSERT_EQUALS(interpreter::toString(a.literals().get(a(3).arg), false), "hi");

    TS_ASSERT_EQUALS(a(4).major, Opcode::maPush);
    TS_ASSERT_EQUALS(a(4).minor, Opcode::sInteger);
    TS_ASSERT_EQUALS(a(4).arg, 444);

    TS_ASSERT_EQUALS(a(5).major, Opcode::maUnary);
    TS_ASSERT_EQUALS(a(5).minor, interpreter::unZap);

    TS_ASSERT_EQUALS(a(6).major, Opcode::maJump);
    TS_ASSERT_EQUALS(a(6).arg, b(5).arg + 2);         // offset 2 (labels)

    TS_ASSERT_EQUALS(a(7).major, Opcode::maJump);
    TS_ASSERT_EQUALS(a(7).arg, b(6).arg + 1);         // offset 1 (instructions)

    TS_ASSERT_EQUALS(a(8).major, Opcode::maMemref);
    TS_ASSERT_EQUALS(a(8).minor, Opcode::miIMLoad);
    TS_ASSERT_EQUALS(a.getName(a(8).arg), "HI");

    TS_ASSERT_EQUALS(a(9).major, Opcode::maSpecial);
    TS_ASSERT_EQUALS(a(9).minor, Opcode::miSpecialNewHash);

    TS_ASSERT_EQUALS(a(10).major, Opcode::maSpecial);
    TS_ASSERT_EQUALS(a(10).minor, Opcode::miSpecialDefShipProperty);
    TS_ASSERT_EQUALS(a.getName(a(10).arg), "HO");
}

void
TestInterpreterBytecodeObject::testDisassembly()
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    TS_ASSERT_EQUALS(world.globalPropertyNames().getIndexByName("A"), 0U);

    // Set up testee
    interpreter::BytecodeObject a;
    TS_ASSERT_EQUALS(a.addLocalVariable("X"), 0U);
    TS_ASSERT_EQUALS(a.addLocalVariable("Y"), 1U);

    // 0. Literal
    afl::data::StringValue sv("hi");
    a.addPushLiteral(&sv);

    // 1. Name
    a.addInstruction(Opcode::maPush, Opcode::sNamedVariable, a.addName("N"));

    // 2. Shared
    a.addInstruction(Opcode::maPop, Opcode::sShared, 0);  // global 'A'

    // 3. Local
    a.addInstruction(Opcode::maPush, Opcode::sLocal, 1);  // local 'Y'

    // 4. Negative integer
    a.addInstruction(Opcode::maPush, Opcode::sInteger, uint16_t(-5));

    // 5. Unsigned integer
    a.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 3);

    // Disassemble
    TS_ASSERT_EQUALS(a.getDisassembly(0, world), "pushlit     0 <\"hi\">");
    TS_ASSERT_EQUALS(a.getDisassembly(1, world), "pushvar     0 <N>");
    TS_ASSERT_EQUALS(a.getDisassembly(2, world), "popglob     0 <A>");
    TS_ASSERT_EQUALS(a.getDisassembly(3, world), "pushloc     1 <Y>");
    TS_ASSERT_EQUALS(a.getDisassembly(4, world), "pushint     -5");
    TS_ASSERT_EQUALS(a.getDisassembly(5, world), "sreturn     3");
}

void
TestInterpreterBytecodeObject::testMergeByteCodeObjects()
{
    // A BCO that increments a variable
    interpreter::BCORef_t p = interpreter::BytecodeObject::create(true);
    p->addInstruction(Opcode::maUnary, interpreter::unInc, 0);

    // Check different counts, including zero and one
    for (int i = 0; i < 10; ++i) {
        // Build object-under-test
        std::vector<interpreter::BCOPtr_t> vec;
        for (int j = 0; j < i; ++j) {
            vec.push_back(p.asPtr());
        }
        interpreter::BCORef_t bco = interpreter::mergeByteCodeObjects(vec);

        // Run it
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world(log, tx, fs);
        interpreter::Process proc(world, "test", 99);

        proc.pushNewValue(interpreter::makeIntegerValue(0));
        proc.pushFrame(bco, false);
        proc.run();
        TS_ASSERT_EQUALS(proc.getState(), interpreter::Process::Ended);

        // Result must equal the number of iterations
        TS_ASSERT_EQUALS(interpreter::mustBeScalarValue(proc.getResult()), i);
    }
}

