/**
  *  \file test/interpreter/bytecodeobjecttest.cpp
  *  \brief Test for interpreter::BytecodeObject
  */

#include "interpreter/bytecodeobject.hpp"

#include <stdexcept>
#include "afl/data/integervalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/format.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/compilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/process.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"

using interpreter::Opcode;

/** Test getter/setters. */
AFL_TEST("interpreter.BytecodeObject:basics", a)
{
    interpreter::BytecodeObject testee;

    // Procedure flag: default is enabled
    a.check("01. isProcedure", testee.isProcedure());
    testee.setIsProcedure(false);
    a.check("02. isProcedure", !testee.isProcedure());

    // Varargs flag: default is disabled
    a.check("11. isVarargs", !testee.isVarargs());
    testee.setIsVarargs(true);
    a.check("12. isVarargs", testee.isVarargs());

    // Name: default is empty
    a.checkEqual("21. getSubroutineName", testee.getSubroutineName(), "");
    testee.setSubroutineName("HI");
    a.checkEqual("22. getSubroutineName", testee.getSubroutineName(), "HI");

    // Origin: default is empty
    a.checkEqual("31. getOrigin", testee.getOrigin(), "");
    testee.setOrigin("oh!");
    a.checkEqual("32. getOrigin", testee.getOrigin(), "oh!");

    // File name: default is empty
    a.checkEqual("41. getFileName", testee.getFileName(), "");
    testee.setFileName("test.q");
    a.checkEqual("42. getFileName", testee.getFileName(), "test.q");

    // Arguments: default is none
    a.checkEqual("51. getMinArgs", testee.getMinArgs(), 0U);
    a.checkEqual("52. getMaxArgs", testee.getMaxArgs(), 0U);
    testee.setMinArgs(3);
    testee.setMaxArgs(9);
    a.checkEqual("53. getMinArgs", testee.getMinArgs(), 3U);
    a.checkEqual("54. getMaxArgs", testee.getMaxArgs(), 9U);
}

/** Test arguments. */
AFL_TEST("interpreter.BytecodeObject:args", a)
{
    interpreter::BytecodeObject testee;

    // Default is no args, no varargs
    a.checkEqual("01. getMinArgs", testee.getMinArgs(), size_t(0));
    a.checkEqual("02. getMaxArgs", testee.getMaxArgs(), size_t(0));
    a.check("03. isVarargs", !testee.isVarargs());

    // Add some args
    uint16_t aa = testee.addArgument("A", false);
    uint16_t bb = testee.addArgument("B", false);
    a.checkEqual("11. getMinArgs", testee.getMinArgs(), size_t(2));
    a.checkEqual("12. getMaxArgs", testee.getMaxArgs(), size_t(2));
    a.check("13. isVarargs", !testee.isVarargs());
    a.checkEqual("14. first arg", aa, 0U);
    a.checkEqual("15. second arg", bb, 1U);

    // Add some optional args
    testee.addArgument("C", false);
    testee.addArgument("D", false);
    testee.addArgument("E", true);
    uint16_t ff = testee.addArgument("F", true);
    a.checkEqual("21. getMinArgs", testee.getMinArgs(), size_t(4));
    a.checkEqual("22. getMaxArgs", testee.getMaxArgs(), size_t(6));
    a.check("23. isVarargs", !testee.isVarargs());
    a.checkEqual("24. sixth arg", ff, 5U);

    // Varargs are local variables
    a.check("31. hasLocalVariable", testee.hasLocalVariable("A"));
    a.check("32. hasLocalVariable", testee.hasLocalVariable("B"));
    a.check("33. hasLocalVariable", testee.hasLocalVariable("C"));
    a.check("34. hasLocalVariable", testee.hasLocalVariable("D"));
    a.check("35. hasLocalVariable", testee.hasLocalVariable("E"));
    a.check("36. hasLocalVariable", testee.hasLocalVariable("F"));
}

/** Test addLocalVariable(). */
AFL_TEST("interpreter.BytecodeObject:addLocalVariable", a)
{
    interpreter::BytecodeObject testee;
    uint16_t x = testee.addLocalVariable("X");
    uint16_t y = testee.addLocalVariable("Y");
    a.checkDifferent("01", x, y);
    a.check("02", testee.hasLocalVariable("X"));
    a.check("03", testee.hasLocalVariable("Y"));
}

/** Test addLocalVariable(), overflow. */
AFL_TEST("interpreter.BytecodeObject:addLocalVariable:overflow", a)
{
    interpreter::BytecodeObject testee;

    // The limit is 65536, but out-of-memory or size restrictions may mean we need to stop earlier
    // (This requires at least 448k, most likely around 1.5 to 2M, for the NameMap.)
    try {
        for (int32_t i = 0; i < 65536; ++i) {
            testee.addLocalVariable(afl::string::Format("V%d", i));
        }
    }
    catch (...) {
    }

    // Adding next one must fail - either due to overflow,
    // or due to getting into the same out-of-memory situation as before.
    AFL_CHECK_THROWS(a, testee.addLocalVariable("X"), std::exception);
}

/** Test copyLocalVariablesFrom. */
AFL_TEST("interpreter.BytecodeObject:copyLocalVariablesFrom", a)
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
    a.check("01. hasLocalVariable", x.hasLocalVariable("A"));
    a.check("02. hasLocalVariable", x.hasLocalVariable("B"));
    a.check("03. hasLocalVariable", x.hasLocalVariable("C"));
    a.check("04. hasLocalVariable", x.hasLocalVariable("D"));
    a.check("05. hasLocalVariable", x.hasLocalVariable("E"));

    // This copies! That is, we now have C twice.
    a.checkEqual("11. getNumNames", x.localVariables().getNumNames(), size_t(6));
    a.checkEqual("12. getNameByIndex", x.localVariables().getNameByIndex(0), "A");
    a.checkEqual("13. getNameByIndex", x.localVariables().getNameByIndex(1), "B");
    a.checkEqual("14. getNameByIndex", x.localVariables().getNameByIndex(2), "C");
    a.checkEqual("15. getNameByIndex", x.localVariables().getNameByIndex(3), "C");
    a.checkEqual("16. getNameByIndex", x.localVariables().getNameByIndex(4), "D");
    a.checkEqual("17. getNameByIndex", x.localVariables().getNameByIndex(5), "E");

    // const vs. mutable
    a.checkEqual("21. localVariables", &x.localVariables(), &const_cast<interpreter::BytecodeObject&>(x).localVariables());
}

/** Test labels. */
AFL_TEST("interpreter.BytecodeObject:labels", a)
{
    interpreter::BytecodeObject testee;

    // Make two labels
    interpreter::BytecodeObject::Label_t aa = testee.makeLabel();
    interpreter::BytecodeObject::Label_t bb = testee.makeLabel();
    a.checkDifferent("01. makeLabel", aa, bb);

    // Generate some code
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLabel(aa);
    testee.addJump(Opcode::jAlways, bb);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addLabel(bb);
    testee.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    testee.addJump(Opcode::jAlways, aa);
    a.checkEqual("11. getNumInstructions", testee.getNumInstructions(), size_t(7));

    // Last jump (0,a) jumps to instruction 1
    a.checkEqual("21. getJumpTarget", testee.getJumpTarget(testee(6).minor, testee(6).arg), size_t(1));

    // First jump (0,b) jumps to instruction 4
    a.checkEqual("31. getJumpTarget", testee.getJumpTarget(testee(2).minor, testee(2).arg), size_t(4));

    // Relocate. Code will look like this:
    //         insn
    //   a:    j b
    //         insn
    //   b:    insn
    //         j a
    testee.relocate();
    a.checkEqual("41. getNumInstructions", testee.getNumInstructions(), size_t(5));
    a.checkEqual("42. getJumpTarget", testee.getJumpTarget(testee(1).minor, testee(1).arg), size_t(3));
    a.checkEqual("43. getJumpTarget", testee.getJumpTarget(testee(4).minor, testee(4).arg), size_t(1));
}

/** Test addPushLiteral(). */
AFL_TEST("interpreter.BytecodeObject:addPushLiteral:string", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 1000U);
    a.checkEqual("02. literals", testee.literals().size(), 10U);

    // Check literals
    a.checkNonNull("11. getLiteral", testee.getLiteral(0));
    a.checkNonNull("12. getLiteral", dynamic_cast<afl::data::StringValue*>(testee.getLiteral(0)));
}

/** Test addPushLiteral() with integer literals. */
AFL_TEST("interpreter.BytecodeObject:addPushLiteral:int", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 1000U);
    a.checkEqual("02. literals", testee.literals().size(), 10U);

    // Add 1000 small integer literals.
    // These should not affect the literal pool
    for (int j = 0; j < 1000; ++j) {
        afl::data::IntegerValue sv(j);
        testee.addPushLiteral(&sv);
    }
    a.checkEqual("11. getNumInstructions", testee.getNumInstructions(), 2000U);
    a.checkEqual("12. literal", testee.literals().size(), 10U);
}

/** Test overflow in makeLabel(). */
AFL_TEST("interpreter.BytecodeObject:makeLabel:overflow", a)
{
    interpreter::BytecodeObject testee;

    // The limit is 65535, because we need to express "number of labels" as a uint16_t.
    a.checkEqual("01. getNumLabels", testee.getNumLabels(), 0U);
    for (int32_t i = 0; i < 65535; ++i) {
        testee.makeLabel();
    }
    a.checkEqual("02. getNumLabels", testee.getNumLabels(), 65535U);
    AFL_CHECK_THROWS(a("03. makeLabel"), testee.makeLabel(), interpreter::Error);
}

/** Test overflow in makeLabel(). Shortcut with setting the starting point. */
AFL_TEST("interpreter.BytecodeObject:makeLabel:overflow:preset", a)
{
    interpreter::BytecodeObject testee;

    // The limit is 65535, because we need to express "number of labels" as a uint16_t.
    a.checkEqual("01. getNumLabels", testee.getNumLabels(), 0U);
    testee.setNumLabels(20000);
    a.checkEqual("02. getNumLabels", testee.getNumLabels(), 20000U);
    for (int32_t i = 20000; i < 65535; ++i) {
        testee.makeLabel();
    }
    a.checkEqual("03. getNumLabels", testee.getNumLabels(), 65535U);
    AFL_CHECK_THROWS(a("04. makeLabel"), testee.makeLabel(), interpreter::Error);
}

/** Test overflow in addName(). */
AFL_TEST("interpreter.BytecodeObject:addName:overflow", a)
{
    interpreter::BytecodeObject testee;

    // The limit is 65536, because valid name indexes are [0,65535].
    a.checkEqual("01. num names", testee.names().getNumNames(), 0U);
    for (int32_t i = 0; i < 65536; ++i) {
        String_t name = afl::string::Format("NAME%d", i);
        a.check("02. hasName", !testee.hasName(name));
        testee.addName(name);
        a.check("03. hasName", testee.hasName(name));
    }
    a.checkEqual("04. getNumNames", testee.names().getNumNames(), 65536U);
    AFL_CHECK_THROWS(a("05. addName"), testee.addName("FOO"), interpreter::Error);
}

/** Test general name access. */
AFL_TEST("interpreter.BytecodeObject:addName:normal", a)
{
    interpreter::BytecodeObject testee;

    uint16_t aa = testee.addName("A");
    uint16_t bb = testee.addName("B");
    a.checkDifferent("01. addName different", aa, bb);
    a.checkEqual("02. addName", testee.addName("A"), aa);
    a.checkEqual("03. addName", testee.addName("B"), bb);
    a.check("04. hasName", testee.hasName("A"));
    a.check("05. hasName", testee.hasName("B"));
    a.check("06. hasName", !testee.hasName("C"));
    a.checkEqual("07. getName", testee.getName(aa), "A");
    a.checkEqual("08. getName", testee.getName(bb), "B");

    // const vs. mutable
    a.checkEqual("11. names", &testee.names(), &const_cast<interpreter::BytecodeObject&>(testee).names());
}

/** Test line number handling. */
AFL_TEST("interpreter.BytecodeObject:addLineNumber", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 5U);
    a.checkEqual("02. getLineNumber", testee.getLineNumber(0), 10U);
    a.checkEqual("03. getLineNumber", testee.getLineNumber(1), 10U);
    a.checkEqual("04. getLineNumber", testee.getLineNumber(2), 13U);
    a.checkEqual("05. getLineNumber", testee.getLineNumber(3), 13U);
    a.checkEqual("06. getLineNumber", testee.getLineNumber(5), 14U);
    a.checkEqual("07. getLineNumber", testee.getLineNumber(6), 14U);
    a.checkEqual("08. getLineNumber", testee.getLineNumber(100), 14U);

    // Check storage format
    const std::vector<uint32_t>& rep = testee.lineNumbers();
    a.checkEqual("11. size", rep.size(), 6U);
    a.checkEqual("12. rep", rep[0],  0U);
    a.checkEqual("13. rep", rep[1], 10U);
    a.checkEqual("14. rep", rep[2],  2U);
    a.checkEqual("15. rep", rep[3], 13U);
    a.checkEqual("16. rep", rep[4],  5U);
    a.checkEqual("17. rep", rep[5], 14U);
}

/** Test line number handling, second case. */
AFL_TEST("interpreter.BytecodeObject:addLineNumber:2", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 5U);
    a.checkEqual("02. getLineNumber", testee.getLineNumber(0), 0U);
    a.checkEqual("03. getLineNumber", testee.getLineNumber(1), 0U);
    a.checkEqual("04. getLineNumber", testee.getLineNumber(2), 13U);
    a.checkEqual("05. getLineNumber", testee.getLineNumber(3), 13U);
    a.checkEqual("06. getLineNumber", testee.getLineNumber(5), 14U);
    a.checkEqual("07. getLineNumber", testee.getLineNumber(6), 14U);
    a.checkEqual("08. getLineNumber", testee.getLineNumber(100), 14U);

    // Check storage format
    const std::vector<uint32_t>& rep = testee.lineNumbers();
    a.checkEqual("11. size", rep.size(), 4U);
    a.checkEqual("12. rep", rep[0],  2U);
    a.checkEqual("13. rep", rep[1], 13U);
    a.checkEqual("14. rep", rep[2],  5U);
    a.checkEqual("15. rep", rep[3], 14U);
}

/** Test line number handling, restore from storage. */
AFL_TEST("interpreter.BytecodeObject:addLineNumber:storage", a)
{
    interpreter::BytecodeObject testee;

    // Restore from storage format
    testee.addLineNumber(10, 2);
    testee.addLineNumber(14, 5);
    testee.addLineNumber(15, 6);

    // Verify access
    a.checkEqual("01. getLineNumber", testee.getLineNumber(0), 0U);
    a.checkEqual("02. getLineNumber", testee.getLineNumber(1), 0U);
    a.checkEqual("03. getLineNumber", testee.getLineNumber(2), 10U);
    a.checkEqual("04. getLineNumber", testee.getLineNumber(3), 10U);
    a.checkEqual("05. getLineNumber", testee.getLineNumber(5), 14U);
    a.checkEqual("06. getLineNumber", testee.getLineNumber(6), 15U);
    a.checkEqual("07. getLineNumber", testee.getLineNumber(7), 15U);
    a.checkEqual("08. getLineNumber", testee.getLineNumber(100), 15U);
}

/*
 *  hasUserCall()
 */

// Boundary case: empty
AFL_TEST("interpreter.BytecodeObject:hasUserCall:empty", a)
{
    interpreter::BytecodeObject t;
    a.check("", !t.hasUserCall());
}

// Some uncritical instructions
AFL_TEST("interpreter.BytecodeObject:hasUserCall:normal", a)
{
    interpreter::BytecodeObject t;
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    t.addInstruction(Opcode::maPush,    Opcode::sInteger, 3);
    t.addInstruction(Opcode::maPop,     Opcode::sLocal, t.addLocalVariable("X"));
    a.check("", !t.hasUserCall());
}

// maIndirect counts as user call because we don't know where it ends up
AFL_TEST("interpreter.BytecodeObject:hasUserCall:maIndirect", a)
{
    interpreter::BytecodeObject t;
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    t.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 1);
    a.check("", t.hasUserCall());
}

// miSpecialEvalStatement
AFL_TEST("interpreter.BytecodeObject:hasUserCall:miSpecialEvalStatement", a)
{
    interpreter::BytecodeObject t;
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 0);
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    a.check("", t.hasUserCall());
}

// miSpecialEvalExpr
AFL_TEST("interpreter.BytecodeObject:hasUserCall:miSpecialEvalExpr", a)
{
    interpreter::BytecodeObject t;
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    a.check("", t.hasUserCall());
}

// miSpecialRunHook
AFL_TEST("interpreter.BytecodeObject:hasUserCall:miSpecialRunHook", a)
{
    interpreter::BytecodeObject t;
    t.addInstruction(Opcode::maSpecial, Opcode::miSpecialRunHook, 0);
    a.check("", t.hasUserCall());
}


/** Test addVariableReferenceInstruction. */
AFL_TEST("interpreter.BytecodeObject:addVariableReferenceInstruction", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 5U);
    a.checkEqual("02. size", testee.code().size(), 5U);

    for (size_t i = 0; i < 5; ++i) {
        a.checkEqual("11. major", testee(i).major, Opcode::maPush);
        a.checkEqual("12. major", const_cast<const interpreter::BytecodeObject&>(testee)(i).major, Opcode::maPush);
    }

    a.checkEqual("21. minor", testee(0).minor, Opcode::sNamedVariable);
    a.checkEqual("22. minor", testee(1).minor, Opcode::sLocal);
    a.checkEqual("23. minor", testee(2).minor, Opcode::sLocal);
    a.checkEqual("24. minor", testee(3).minor, Opcode::sShared);
    a.checkEqual("25. minor", testee(4).minor, Opcode::sNamedVariable);
}

/** Test compact(). */
AFL_TEST("interpreter.BytecodeObject:compact", a)
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
    a.checkEqual("01. getNumInstructions", testee.getNumInstructions(), 4U);
    a.checkEqual("02. size", testee.code().size(), 4U);
    a.checkEqual("03. getLineNumber", testee.getLineNumber(0), 100U);
    a.checkEqual("04. getLineNumber", testee.getLineNumber(1), 100U);
    a.checkEqual("05. getLineNumber", testee.getLineNumber(2), 200U);
    a.checkEqual("06. getLineNumber", testee.getLineNumber(3), 300U);
    for (size_t i = 0; i < 4; ++i) {
        a.checkEqual("07. major", testee(i).major, Opcode::maSpecial);
        a.checkEqual("08. minor", testee(i).minor, Opcode::miSpecialNewHash);
    }
}

AFL_TEST("interpreter.BytecodeObject:append", a)
{
    // Set up copy target:
    //   1 instruction
    //   2 labels
    //   1 name
    //   2 locals
    interpreter::BytecodeObject aa;
    aa.addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    aa.makeLabel(); aa.makeLabel();
    aa.addName("HI");
    aa.addLocalVariable("L1");
    aa.addLocalVariable("L2");

    // Set up copy source:
    interpreter::BytecodeObject bb;

    // 1. pushvar (name copied)
    bb.addInstruction(Opcode::maPush, Opcode::sNamedVariable, bb.addName("N"));

    // 2. pushloc (local transformed)
    bb.addInstruction(Opcode::maPush, Opcode::sLocal, bb.addLocalVariable("L2"));

    // 3. pushlit (literal copied)
    afl::data::StringValue sv("hi");
    bb.addPushLiteral(&sv);

    // 4. pushint (copied)
    bb.addInstruction(Opcode::maPush, Opcode::sInteger, 444);

    // 5. uzap (copied)
    bb.addInstruction(Opcode::maUnary, interpreter::unZap, 0);

    // 6. symbolic label
    bb.addLabel(bb.makeLabel());

    // 7. absolute jump
    bb.addInstruction(Opcode::maJump, Opcode::jIfEmpty, 1);

    // 8. loadmem (name copied)
    bb.addInstruction(Opcode::maMemref, Opcode::miIMLoad, bb.addName("HI"));

    // 9. snewhash (copied)
    bb.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);

    // 10. sdefshipp (name copied)
    bb.addInstruction(Opcode::maSpecial, Opcode::miSpecialDefShipProperty, bb.addName("HO"));

    // Do it
    aa.append(bb);

    // Verify
    a.checkEqual("01. getNumInstructions", aa.getNumInstructions(), 11U);
    a.checkEqual("02. getNumLabels", aa.getNumLabels(), 3U);

    a.checkEqual("11", aa(0).major, Opcode::maPush);
    a.checkEqual("12", aa(0).minor, Opcode::sInteger);

    a.checkEqual("21", aa(1).major, Opcode::maPush);
    a.checkEqual("22", aa(1).minor, Opcode::sNamedVariable);
    a.checkEqual("23", aa.getName(aa(1).arg), "N");

    a.checkEqual("31", aa(2).major, Opcode::maPush);
    a.checkEqual("32", aa(2).minor, Opcode::sLocal);
    a.checkEqual("33", aa.localVariables().getNameByIndex(aa(2).arg), "L2");

    a.checkEqual("41", aa(3).major, Opcode::maPush);
    a.checkEqual("42", aa(3).minor, Opcode::sLiteral);
    a.checkEqual("43", interpreter::toString(aa.literals().get(aa(3).arg), false), "hi");

    a.checkEqual("51", aa(4).major, Opcode::maPush);
    a.checkEqual("52", aa(4).minor, Opcode::sInteger);
    a.checkEqual("53", aa(4).arg, 444);

    a.checkEqual("61", aa(5).major, Opcode::maUnary);
    a.checkEqual("62", aa(5).minor, interpreter::unZap);

    a.checkEqual("71", aa(6).major, Opcode::maJump);
    a.checkEqual("72", aa(6).arg, bb(5).arg + 2);         // offset 2 (labels)

    a.checkEqual("81", aa(7).major, Opcode::maJump);
    a.checkEqual("82", aa(7).arg, bb(6).arg + 1);         // offset 1 (instructions)

    a.checkEqual("91", aa(8).major, Opcode::maMemref);
    a.checkEqual("92", aa(8).minor, Opcode::miIMLoad);
    a.checkEqual("93", aa.getName(aa(8).arg), "HI");

    a.checkEqual("101", aa(9).major, Opcode::maSpecial);
    a.checkEqual("102", aa(9).minor, Opcode::miSpecialNewHash);

    a.checkEqual("111", aa(10).major, Opcode::maSpecial);
    a.checkEqual("112", aa(10).minor, Opcode::miSpecialDefShipProperty);
    a.checkEqual("113", aa.getName(aa(10).arg), "HO");
}

AFL_TEST("interpreter.BytecodeObject:getDisassembly", a)
{
    // Environment
    afl::sys::Log log;
    afl::string::NullTranslator tx;
    afl::io::NullFileSystem fs;
    interpreter::World world(log, tx, fs);
    a.checkEqual("01. getIndexByName", world.globalPropertyNames().getIndexByName("A"), 0U);

    // Set up testee
    interpreter::BytecodeObject aa;
    a.checkEqual("11. addLocalVariable", aa.addLocalVariable("X"), 0U);
    a.checkEqual("12. addLocalVariable", aa.addLocalVariable("Y"), 1U);

    // 0. Literal
    afl::data::StringValue sv("hi");
    aa.addPushLiteral(&sv);

    // 1. Name
    aa.addInstruction(Opcode::maPush, Opcode::sNamedVariable, aa.addName("N"));

    // 2. Shared
    aa.addInstruction(Opcode::maPop, Opcode::sShared, 0);  // global 'AA'

    // 3. Local
    aa.addInstruction(Opcode::maPush, Opcode::sLocal, 1);  // local 'Y'

    // 4. Negative integer
    aa.addInstruction(Opcode::maPush, Opcode::sInteger, uint16_t(-5));

    // 5. Unsigned integer
    aa.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 3);

    // Disassemble
    a.checkEqual("21. getDisassembly", aa.getDisassembly(0, world), "pushlit     0 <\"hi\">");
    a.checkEqual("22. getDisassembly", aa.getDisassembly(1, world), "pushvar     0 <N>");
    a.checkEqual("23. getDisassembly", aa.getDisassembly(2, world), "popglob     0 <A>");
    a.checkEqual("24. getDisassembly", aa.getDisassembly(3, world), "pushloc     1 <Y>");
    a.checkEqual("25. getDisassembly", aa.getDisassembly(4, world), "pushint     -5");
    a.checkEqual("26. getDisassembly", aa.getDisassembly(5, world), "sreturn     3");
}

AFL_TEST("interpreter.BytecodeObject:merge", a)
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
        a.checkEqual("01. getState", proc.getState(), interpreter::Process::Ended);

        // Result must equal the number of iterations
        a.checkEqual("11. result", interpreter::mustBeScalarValue(proc.getResult(), interpreter::Error::ExpectInteger), i);
    }
}
