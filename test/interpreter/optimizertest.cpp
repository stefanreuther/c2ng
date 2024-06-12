/**
  *  \file test/interpreter/optimizertest.cpp
  *  \brief Test for interpreter::Optimizer
  *
  *  In PCC2, this test was implemented as a *.qs file and a perl script.
  *  The *.qs file was compiled into a *.qc file using c2asm, loaded by the 'teststmt' application, and verified by the perl script.
  *  That needs quite a lot of other code other than just the optimizer, and more infrastructure (perl).
  *  Doing this in C++ is a little more boilerplate code, but nicely integrates in the testsuite.
  *
  *  One key difference is that the 'teststmt' application always linearized after optimization while we don't.
  */

#include "interpreter/optimizer.hpp"

#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "afl/test/testrunner.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/process.hpp"
#include "interpreter/tokenizer.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/world.hpp"
#include <iostream>

using interpreter::Opcode;
using interpreter::BytecodeObject;

namespace {
    struct Stuff {
        afl::sys::Log log;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        BytecodeObject bco;

        Stuff()
            : log(), tx(), fs(), world(log, tx, fs), bco()
            { }

        // Utility function for debugging this beast
        void dump()
            {
                for (size_t i = 0, n = bco.getNumInstructions(); i < n; ++i) {
                    std::cout << i << ": " << bco.getDisassembly(i, world) << "\n";
                }
            }
    };

    bool isLocalVariableName(const BytecodeObject& bco, uint16_t index, String_t name)
    {
        const afl::data::NameMap& names = bco.localVariables();
        return (index < names.getNumNames()
                && names.getNameByIndex(index) == name);
    }

    bool isName(const BytecodeObject& bco, uint16_t index, String_t name)
    {
        const afl::data::NameMap& names = bco.names();
        return (index < names.getNumNames()
                && names.getNameByIndex(index) == name);
    }

    bool isInstruction(const Opcode& insn, const Opcode::Major major, uint8_t minor)
    {
        return insn.major == major
            && insn.minor == minor;
    }

    bool isInstruction(const Opcode& insn, const Opcode::Major major, uint8_t minor, uint16_t arg)
    {
        return isInstruction(insn, major, minor)
            && insn.arg == arg;
    }

    void checkExpression(afl::test::Assert a, const char* expr, int32_t expectedValue, int level)
    {
        afl::sys::Log logger;
        afl::string::NullTranslator tx;
        afl::io::NullFileSystem fs;
        interpreter::World world(logger, tx, fs);
        afl::base::Deleter del;

        interpreter::Tokenizer tok(expr);
        const interpreter::expr::Node& node(interpreter::expr::Parser(tok, del).parse());
        a(expr).checkEqual("checkExpression: parse complete", tok.getCurrentToken(), tok.tEnd);

        interpreter::BCORef_t bco = interpreter::BytecodeObject::create(true);
        node.compileValue(*bco, interpreter::CompilationContext(world));

        optimize(world, *bco, level);

        interpreter::Process exec(world, "checkExpression", 9);
        exec.pushFrame(bco, false);
        exec.run();
        a(expr).checkEqual("checkExpression: execution succeeded", exec.getState(), interpreter::Process::Ended);

        const afl::data::ScalarValue* resv = dynamic_cast<const afl::data::ScalarValue*>(exec.getResult());
        a(expr).checkNonNull("checkExpression: ScalarValue", resv);
        a(expr).checkEqual("checkExpression: value", resv->getValue(), expectedValue);
    }
}

/*
 *  StoreDrop - merging store+drop -> pop
 */

/** Test storeloc a + drop 1 -> poploc a (drop removed). */
AFL_TEST("interpreter.Optimizer:store-drop", a)
{
    // ex test_opt.qs:in.storeDrop1:
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    a.check("03. isLocalVariableName", isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storeloc a, drop 2 -> poploc a, drop 1 (drop remains). */
AFL_TEST("interpreter.Optimizer:store-drop:extra", a)
{
    // ex test_opt.qs:in.storeDrop2
    // storeloc a + drop 2 -> storeloc a + drop 1
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 2);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    a.check("03. isLocalVariableName", isLocalVariableName(s.bco, s.bco(0).arg, "A"));
    a.check("04. insn 1", isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
}

/** Test storeloc a, drop 0 (removes the drop, does not create invalid drop -1). */
AFL_TEST("interpreter.Optimizer:store-drop:null", a)
{
    // ex test_opt.qs:in.storeDrop3
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStore, Opcode::sLocal));
    a.check("03. isLocalVariableName", isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storeloc a + drop 0 + drop 1 -> poploc (drops are combined, then eliminated). */
AFL_TEST("interpreter.Optimizer:store-drop:multi", a)
{
    // ex test_opt.qs:in.storeDrop4
    // storeloc + drop 0 + drop 1 -> poploc
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    a.check("03. isLocalVariableName", isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storemem + drop -> popmem (maMemref instead of maStore). */
AFL_TEST("interpreter.Optimizer:store-drop:memref", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, s.bco.addName("XY"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maMemref, Opcode::miIMPop));
    a.check("03. isName", isName(s.bco, s.bco(0).arg, "XY"));
}

/*
 *  MergeDrop - merging multiple drop statements into one
 */

/** Test merging multiple drop into one. */
AFL_TEST("interpreter.Optimizer:merge-drop", a)
{
    // ex test_opt.qs:in.mergeDrop1
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 3));
}

/** Test merging multiple drop into one, even if some of them have count 0. */
AFL_TEST("interpreter.Optimizer:merge-drop:null", a)
{
    // ex test_opt.qs:in.mergeDrop2
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 2);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 3));
}

/*
 *  NullOp - removing null operations (and preserving those that look like null ops but aren't)
 *
 *  Wrap the tests into guaranteed-unoptimizable instructions to avoid that the optimizer sees
 *  special cases at the end of the sub.
 */

/** Test removal of null operation "drop 0". */
AFL_TEST("interpreter.Optimizer:nullop:drop", a)
{
    // ex test_opt.qs:in.nullOp1
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackDrop,      0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test removal of null operation "swap 0". */
AFL_TEST("interpreter.Optimizer:nullop:swap", a)
{
    // ex test_opt.qs:in.nullOp2
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackSwap,      0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test preservation of non-null operation "dup 0". */
AFL_TEST("interpreter.Optimizer:nullop:dup", a)
{
    // ex test_opt.qs:in.nullOp3
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackDup,       0); // not a null op!
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackDup,       0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test preservation of non-null operation "swap 1". */
AFL_TEST("interpreter.Optimizer:nullop:swap:preserve", a)
{
    // ex test_opt.qs:in.nullOp4
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackSwap,      1); // not a null op!
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackSwap,      1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/*
 *  EraseUnusedLabels
 */

/** Test removal of unused labels. */
AFL_TEST("interpreter.Optimizer:erase-unused-labels", a)
{
    // ex test_opt.qs:in.eraseUnusedLabels1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jt #1, label #0, uinc, label #1, udec
    // -> remove label #0.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec,                  0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 4U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l0));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l0));
    a.check("15. insn 3", isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec,                  0));
}

/** Test removal of unused labels that enables further optimisation. */
AFL_TEST("interpreter.Optimizer:erase-unused-labels:enabler", a)
{
    // ex test_opt.qs:in.eraseUnusedLabels2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jt #1, pushint 1, label #0, add
    // -> the label would normally break the 'pushint 1/badd' pattern,
    // so we see that it has been removed because the pattern has been applied.
    s.bco.addInstruction(Opcode::maJump,   Opcode::jIfTrue | Opcode::jSymbolic, l1);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,                    1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biAdd,                  0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,                   l1);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unDec,                  0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 4U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l1));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
    a.check("15. insn 3", isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec,                  0));
}

/*
 *  InvertJumps - jump-across-jump
 */

/** Test removal of unconditional jump-across-jump.
    (Conditional jump-across-jump is testInvertJumps6). */
AFL_TEST("interpreter.Optimizer:invert-jumps", a)
{
    // ex test_opt.qs:in.InvertJumps1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // j #0, j #1, label #0: disappears completely
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test popping-jump-across-popping-jump.
    Optimisation does not apply here. */
AFL_TEST("interpreter.Optimizer:invert-jumps:pop", a)
{
    // ex test_opt.qs:in.InvertJumps2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtp #0, jfep #1, label #0: two jumps with pop; optimisation does not apply here
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 5U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l0));
    a.check("15. insn 3", isInstruction(s.bco(3), Opcode::maUnary, interpreter::unInc,                  0));
    a.check("16. insn 4", isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, inverse condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:inverse-condition", a)
{
    // ex test_opt.qs:in.InvertJumps3 (fixed, #328)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtp #0, jfe #1, label #0: two jumps with opposite condition (regular inversion case)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, similar condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:similar-condition", a)
{
    // ex test_opt.qs:in.InvertJumps4
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtfp #0, jt #1, label #0: second jump never taken, group degenerates into 'drop'
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse /*| Opcode::jPopAlways*/, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc,  0));
}

/** Test conditional-jump-across-unconditional-jump.
    This is the regular jump-inversion case. */
AFL_TEST("interpreter.Optimizer:invert-jumps:across-unconditional", a)
{
    // ex test_opt.qs:in.InvertJumps5
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtp #0, j #1, label #0: conditional followed by unconditional (common case)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, same condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:across-conditional", a)
{
    // ex test_opt.qs:in.InvertJumps6
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtf #0, jtf #1, label #0: disappears completely
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                                     0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test conditional-jump-across-jdz. Optimisation does not apply here. */
AFL_TEST("interpreter.Optimizer:invert-jumps:across-jdz", a)
{
    // ex test_opt.qs:in.InvertJumps7
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jt #0, jdz #1, label #0: optimisation does not apply here
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jDecZero, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                   0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 5U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue,  l0));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic | Opcode::jDecZero, l1));
    a.check("14. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    a.check("15. insn 3", isInstruction(s.bco(3), Opcode::maUnary, interpreter::unInc,                   0));
    a.check("16. insn 4", isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                    l1));
}

/** Test popping-conditional-jump-across-conditional-jump, inverse condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:pop-across-conditional", a)
{
    // ex test_opt.qs:in.InvertJumps3 (fixed)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtp #0, jfe #1, label #0: two jumps with opposite condition. Optimisation does not apply due to pop.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 5U);
}

/** Test conditional-jump-across-conditional-jump, similar condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:pop-across-same-conditional", a)
{
    // ex test_opt.qs:in.InvertJumps4
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtfp #0, jt #1, label #0: optimisation does not apply
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 5U);
}

/** Test conditional-jump-across-conditional-jump, same condition. */
AFL_TEST("interpreter.Optimizer:invert-jumps:across-same-condition", a)
{
    // ex test_opt.qs:in.InvertJumps6 (fixed, #328)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jtfp #0, jtf #1, label #0: optimisation does not apply due to pop.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                                     0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 5U);
}

/** Test jump-across-jump. */
AFL_TEST("interpreter.Optimizer:invert-jumps:across-jump", a)
{
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    a.checkEqual("01. makeLabel", l0, 0U);
    a.checkEqual("02. makeLabel", l1, 1U);

    // jp #0, jt #1, label #0: turns into drop
    // (This could also be achieved using a combination of dead-code-removal and jump threading.)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
}

/*
 *  ThreadJumps - optimize jump-to-jump
 */

/** Test optimisation of jump-to-jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps", a)
{
    // ex test_opt.qs:in.ThreadJumps1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();
    BytecodeObject::Label_t l3 = s.bco.makeLabel();

    // Just some convoluted jump-around
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,                     l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                       l2);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,                     l3);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                       l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                       l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,                     l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                       l3);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 4U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l2));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                                       l2));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec, 0));
}

/** Test optimisation of jump-to-jump, infinite loop. */
AFL_TEST("interpreter.Optimizer:thread-jumps:loop", a)
{
    // ex test_opt.qs:in.ThreadJumps2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();
    BytecodeObject::Label_t l3 = s.bco.makeLabel();
    BytecodeObject::Label_t l4 = s.bco.makeLabel();

    // A convoluted infinite loop: 2->4->1->3->0
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l3);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l4);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l3);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l4);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);

    optimize(s.world, s.bco, 2);

    // It's not specified which label remains. As of 20170107, label 3 remains but this is not guaranteed.
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump, Opcode::jSymbolic));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maJump, Opcode::jSymbolic | Opcode::jAlways));
    a.checkEqual("04. arg", s.bco(0).arg, s.bco(1).arg);
}

/** Test optimisation of jump-to-jump, infinite loop. */
AFL_TEST("interpreter.Optimizer:thread-jumps:complex-loop", a)
{
    // ex test_opt.qs:in.ThreadJumps3
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();
    BytecodeObject::Label_t l3 = s.bco.makeLabel();
    BytecodeObject::Label_t l4 = s.bco.makeLabel();

    // Another convoluted infinite loop: 3->1->4->2->0 (opposite of testThreadJumps2).
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l3);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l4);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l3);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l4);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);

    optimize(s.world, s.bco, 2);

    // It's not specified which label remains.
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump, Opcode::jSymbolic));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maJump, Opcode::jSymbolic | Opcode::jAlways));
    a.checkEqual("04. arg", s.bco(0).arg, s.bco(1).arg);
}

/** Test optimisation of jump-to-jump, jumping into the middle of an infinite loop. */
AFL_TEST("interpreter.Optimizer:thread-jumps:into-loop", a)
{
    // ex test_opt.qs:in.ThreadJumps4
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();

    // Jump into infinite loop: jt #0, uinc, label #0, j #0.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue,  l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 4U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l0));
}

/** Test optimisation of jump-to-jump that degenerates into no jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps:degenerate", a)
{
    // ex test_opt.qs:in.ThreadJumps5
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();

    // Indirect jump-back-here
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test optimisation of conditional-jump-to-jump that degenerates into no jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps:degenerate-conditional", a)
{
    // ex test_opt.qs:in.ThreadJumps6
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();

    // Indirect jump-back-here, with conditional jump
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test optimisation of popping-conditional-jump-to-jump that degenerates into no jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps:degenerate-conditional-pop", a)
{
    // ex test_opt.qs:in.ThreadJumps7
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();

    // Indirect jump-back-here, with popping conditional jump
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,  0));
}

/** Test jump-to-conditional-jump (not optimized). */
AFL_TEST("interpreter.Optimizer:thread-jumps:to-conditional", a)
{
    // ex test_opt.qs:in.ThreadJumps8
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();

    // Jump to conditional jump (not optimized)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l2);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 7U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic,                    l1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary, interpreter::unInc, 0));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    a.check("07. insn 5", isInstruction(s.bco(5), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1));
    a.check("08. insn 6", isInstruction(s.bco(6), Opcode::maJump,  Opcode::jSymbolic,                    l2));
}

/** Test catch-to-jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps:catch", a)
{
    (void) a;
    // ex test_opt.qs:in.ThreadJumps9
    // % FIXME: 'catch' to a jump isn't yet optimized
    // % sub in.ThreadJumps9
    // %   % catch
    // %   catch handler
    // %   uinc
    // % handler:
    // %   j skip
    // % again:
    // %   uinc
    // % skip:
    // %   uinc
    // %   j again
    // % endsub
    // % .jumps abs
    // % sub out.ThreadJumps9
    // %   catch skip
    // %   uinc
    // % handler:
    // %   j skip
    // % again:
    // %   uinc
    // % skip:
    // %   uinc
    // %   j again
    // % endsub
    // % .jumps sym
}

/** Test jdz-to-jump. */
AFL_TEST("interpreter.Optimizer:thread-jumps:jdz", a)
{
    (void) a;
    // ex test_opt.qs:in.ThreadJumps10
    // % FIXME: 'jdz' to a jump isn't yet optimized
    // % sub in.ThreadJumps10
    // %   % jdz
    // %   jdz handler
    // %   uinc
    // % handler:
    // %   j skip
    // % again:
    // %   uinc
    // % skip:
    // %   uinc
    // %   j again
    // % endsub
    // % .jumps abs
    // % sub out.ThreadJumps10
    // %   jdz skip
    // %   uinc
    // % handler:
    // %   j skip
    // % again:
    // %   uinc
    // % skip:
    // %   uinc
    // %   j again
    // % endsub
    // % .jumps sym
}

/*
 *  RemoveUnused
 */

/** Test removal of unused code (jumped across). */
AFL_TEST("interpreter.Optimizer:remove-unused:jump", a)
{
    // ex test_opt.qs:in.RemoveUnused1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();

    // j skip, uinc, uinc, skip: uinc -> uinc
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test removal of unused code (jumped across) with a label in the middle. */
AFL_TEST("interpreter.Optimizer:remove-unused:label", a)
{
    // ex test_opt.qs:in.RemoveUnused2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();

    // Skipping an unused piece of code that includes a used label
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 6U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic,                    l1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary, interpreter::unDec, 0));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maUnary, interpreter::unInc, 0));
    a.check("07. insn 5", isInstruction(s.bco(5), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1));
}

/** Test removal of unused code using jump-away instructions other than jump. */
AFL_TEST("interpreter.Optimizer:remove-unused:special", a)
{
    // ex test_opt.qs:in.RemoveUnused3
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    BytecodeObject::Label_t l2 = s.bco.makeLabel();
    BytecodeObject::Label_t l3 = s.bco.makeLabel();

    // Other termination instructions
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jDecZero, l0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jDecZero, l1);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jDecZero, l2);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jDecZero, l3);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialTerminate, 0);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic,                    l1);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic,                    l2);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 0);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic,                    l3);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 12U);
    a.check("02. insn 4", isInstruction(s.bco(4),  Opcode::maJump, Opcode::jSymbolic, l0));
    a.check("03. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialTerminate, 0));
    a.check("04. insn 6", isInstruction(s.bco(6),  Opcode::maJump, Opcode::jSymbolic, l1));
    a.check("05. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialThrow, 0));
    a.check("06. insn 8", isInstruction(s.bco(8),  Opcode::maJump, Opcode::jSymbolic, l2));
    a.check("07. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialReturn, 0));
    a.check("08. insn 10", isInstruction(s.bco(10), Opcode::maJump, Opcode::jSymbolic, l3));
    a.check("09. insn 11", isInstruction(s.bco(11), Opcode::maUnary, interpreter::unDec, 0));
}

/*
 *  MergeNegation - merge two unary operations
 *
 *  These tests use 'sprint' as an un-optimizable instruction to separate individual cases.
 */

/** Test merging of negation instruction pairs, starting with unot. */
AFL_TEST("interpreter.Optimizer:merge-negation:not", a)
{
    // ex test_opt.qs:in.MergeNegation1
    Stuff s;

    // Instruction pairs starting with unot (t->f, f->t, e->e)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 23U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 2", isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("22. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("32. insn 5", isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("33. insn 6", isInstruction(s.bco(6),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("42. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNot,     0));
    a.check("52. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 12", isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 13", isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("62. insn 14", isInstruction(s.bco(14), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 15", isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unNot,     0));
    a.check("72. insn 16", isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 17", isInstruction(s.bco(17), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot,     0));
    a.check("82. insn 19", isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 20", isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unNot,     0));
    a.check("92. insn 22", isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with ubool. */
AFL_TEST("interpreter.Optimizer:merge-negation:bool", a)
{
    // ex test_opt.qs:in.MergeNegation2
    Stuff s;

    // Instruction pairs starting with ubool (t->t, f->f, e->e)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 22U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 2", isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("22. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("32. insn 5", isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("33. insn 6", isInstruction(s.bco(6),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("42. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("52. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unBool,    0));
    a.check("53. insn 12", isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 13", isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("62. insn 14", isInstruction(s.bco(14), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 15", isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("72. insn 16", isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unBool,    0));
    a.check("82. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 19", isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unBool,    0));
    a.check("92. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with upos. */
AFL_TEST("interpreter.Optimizer:merge-negation:pos", a)
{
    // ex test_opt.qs:in.MergeNegation3
    Stuff s;

    // Instruction pairs starting with upos (+->+, -->-, e->e)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 22U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("23. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("32. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unPos,     0));
    a.check("52. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 12", isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 13", isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unPos,     0));
    a.check("62. insn 14", isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("63. insn 15", isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 16", isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unPos,     0));
    a.check("72. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 18", isInstruction(s.bco(18), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 19", isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("82. insn 20", isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uneg. */
AFL_TEST("interpreter.Optimizer:merge-negation:neg", a)
{
    // ex test_opt.qs:in.MergeNegation4
    Stuff s;

    // Instruction pairs starting with uneg (+->-, -->+, e->e)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 24U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("23. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("32. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("52. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 12", isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 13", isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("62. insn 14", isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("63. insn 15", isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 16", isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("72. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 18", isInstruction(s.bco(18), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 19", isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("82. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 21", isInstruction(s.bco(21), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 22", isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("92. insn 23", isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uzap. */
AFL_TEST("interpreter.Optimizer:merge-negation:zap", a)
{
    // ex test_opt.qs:in.MergeNegation5
    Stuff s;

    // Instruction pairs starting with uzap (t->t, f->e, e->e)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 23U);
    a.check("02. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unZap,     0));
    a.check("03. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("04. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("11. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unZap,     0));
    a.check("12. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("13. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unZap,     0));
    a.check("22. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("23. insn 8", isInstruction(s.bco(8),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unZap,     0));
    a.check("32. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("33. insn 11", isInstruction(s.bco(11), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 12", isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("42. insn 13", isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 14", isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("52. insn 15", isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 16", isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("62. insn 17", isInstruction(s.bco(17), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("72. insn 19", isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("73. insn 20", isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("82. insn 22", isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uisempty. */
AFL_TEST("interpreter.Optimizer:merge-negation:isempty", a)
{
    // ex test_opt.qs:in.MergeNegation6
    Stuff s;

    // Instruction pairs starting with uisempty (t->f, f->f, e->t)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 25U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 5", isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("32. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("33. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 10", isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("52. insn 12", isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 13", isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 14", isInstruction(s.bco(14), Opcode::maStack,   Opcode::miStackDrop,    1));
    a.check("62. insn 15", isInstruction(s.bco(15), Opcode::maPush,    Opcode::sBoolean,       0));
    a.check("63. insn 16", isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("72. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 19", isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("82. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 22", isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 23", isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("92. insn 24", isInstruction(s.bco(24), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with unot2. */
AFL_TEST("interpreter.Optimizer:merge-negation:not2", a)
{
    // ex test_opt.qs:in.MergeNegation7
    Stuff s;

    // Instruction pairs starting with unot2 (t->f, f->t, e->t)
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 25U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 5", isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("32. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("33. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 10", isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("52. insn 12", isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 13", isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 14", isInstruction(s.bco(14), Opcode::maStack,   Opcode::miStackDrop,    1));
    a.check("62. insn 15", isInstruction(s.bco(15), Opcode::maPush,    Opcode::sBoolean,       0));
    a.check("63. insn 16", isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("72. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 19", isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("82. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 22", isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 23", isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("92. insn 24", isInstruction(s.bco(24), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uinc. */
AFL_TEST("interpreter.Optimizer:merge-negation:inc", a)
{
    // ex test_opt.qs:in.MergeNegation8
    Stuff s;

    // Instruction pairs starting with uinc
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 24U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unInc,     0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unInc,     0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("23. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unInc,     0));
    a.check("32. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unInc,     0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 10", isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("52. insn 12", isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 13", isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 14", isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("62. insn 15", isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("63. insn 16", isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("72. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 19", isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("82. insn 21", isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("83. insn 22", isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 23", isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test merging of negation instruction pairs, starting with udec. */
AFL_TEST("interpreter.Optimizer:merge-negation:dec", a)
{
    // ex test_opt.qs:in.MergeNegation9
    Stuff s;

    // Instruction pairs starting with udec
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 24U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unDec,     0));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unDec,     0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    a.check("23. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("31. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unDec,     0));
    a.check("32. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("41. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unDec,     0));
    a.check("42. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("43. insn 10", isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("51. insn 11", isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("52. insn 12", isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    a.check("53. insn 13", isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("61. insn 14", isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("62. insn 15", isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    a.check("63. insn 16", isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("71. insn 17", isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("72. insn 18", isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    a.check("73. insn 19", isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("81. insn 20", isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unPos,     0));
    a.check("82. insn 21", isInstruction(s.bco(21), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    a.check("91. insn 22", isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("92. insn 23", isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unDec,     0));
}

/*
 *  UnaryCondition1 - fuse an unary operation and a conditional popping jump
 */

/** Test folding of uisempty + conditional jump. */
AFL_TEST("interpreter.Optimizer:unary-condition:isempty", a)
{
    // ex test_opt.qs:in.UnaryCondition1
    Stuff s;
    BytecodeObject::Label_t ise = s.bco.makeLabel();
    BytecodeObject::Label_t isf = s.bco.makeLabel();
    BytecodeObject::Label_t ist = s.bco.makeLabel();

    // Combinations starting with uisempty
    // uisempty, jep -> drop
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // uisempty, jfp -> jtfp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // uisempty, jtp -> jep
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // Trailer to keep things recognizable
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ise);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, isf);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ist);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 10U);

    a.check("11. insn 0", isInstruction(s.bco(0), Opcode::maStack,   Opcode::miStackDrop, 1));
    a.check("12. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("13. insn 2", isInstruction(s.bco(2), Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, isf));
    a.check("14. insn 3", isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("15. insn 4", isInstruction(s.bco(4), Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ist));
    a.check("16. insn 5", isInstruction(s.bco(5), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    // label "ise" got removed
    a.check("17. insn 6", isInstruction(s.bco(6), Opcode::maUnary,   interpreter::unInc, 0));
    a.check("18. insn 7", isInstruction(s.bco(7), Opcode::maJump,    Opcode::jSymbolic, isf));
    a.check("19. insn 8", isInstruction(s.bco(8), Opcode::maUnary,   interpreter::unDec, 0));
    a.check("20. insn 9", isInstruction(s.bco(9), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of unot + conditional jump. */
AFL_TEST("interpreter.Optimizer:unary-condition:not", a)
{
    // ex test_opt.qs:in.UnaryCondition2
    Stuff s;
    BytecodeObject::Label_t ise = s.bco.makeLabel();
    BytecodeObject::Label_t isf = s.bco.makeLabel();
    BytecodeObject::Label_t ist = s.bco.makeLabel();

    // Combinations starting with unot
    // unot, jep -> jep
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // unot, jfp -> jtp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // unot, jtp -> jfp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // Trailer to keep things recognizable
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ise);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, isf);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ist);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 11U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, isf));
    a.check("14. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("15. insn 4", isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, ist));
    a.check("16. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("17. insn 6", isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    a.check("18. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unInc, 0));
    a.check("19. insn 8", isInstruction(s.bco(8),  Opcode::maJump,    Opcode::jSymbolic, isf));
    a.check("20. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unDec, 0));
    a.check("21. insn 10", isInstruction(s.bco(10), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of uzap + conditional jump. */
AFL_TEST("interpreter.Optimizer:unary-condition:zap", a)
{
    // ex test_opt.qs:in.UnaryCondition3
    Stuff s;
    BytecodeObject::Label_t ise = s.bco.makeLabel();
    BytecodeObject::Label_t isf = s.bco.makeLabel();
    BytecodeObject::Label_t ist = s.bco.makeLabel();

    // Combinations starting with uzap
    // uzap, jep -> jfep
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // uzap, jfp -> drop
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // uzap, jtp -> jtp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // Trailer to keep things recognizable
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ise);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, isf);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ist);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 9U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maStack,   Opcode::miStackDrop, 1));
    a.check("14. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("15. insn 4", isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist));
    a.check("16. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("17. insn 6", isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    // isf got unreferenced, leaving us a uinc/udec combo merged to upos
    a.check("18. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unPos, 0));
    a.check("19. insn 8", isInstruction(s.bco(8), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of unot2 + conditional jump. */
AFL_TEST("interpreter.Optimizer:unary-condition:not2", a)
{
    // ex test_opt.qs:in.UnaryCondition4
    Stuff s;
    BytecodeObject::Label_t ise = s.bco.makeLabel();
    BytecodeObject::Label_t isf = s.bco.makeLabel();
    BytecodeObject::Label_t ist = s.bco.makeLabel();

    // Combinations starting with unot2
    // unot2, jep -> drop
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // unot2, jfp -> jtp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // unot2, jtp -> jfep
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // Trailer to keep things recognizable
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ise);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, isf);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ist);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 10U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maStack,   Opcode::miStackDrop, 1));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, isf));
    a.check("14. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("15. insn 4", isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, ist));
    a.check("16. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    // ise is unreferenced
    a.check("17. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unInc, 0));
    a.check("18. insn 7", isInstruction(s.bco(7),  Opcode::maJump,    Opcode::jSymbolic, isf));
    a.check("19. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unDec, 0));
    a.check("20. insn 9", isInstruction(s.bco(9),  Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of ubool + conditional jump. */
AFL_TEST("interpreter.Optimizer:unary-condition:bool", a)
{
    // ex test_opt.qs:in.UnaryCondition5
    Stuff s;
    BytecodeObject::Label_t ise = s.bco.makeLabel();
    BytecodeObject::Label_t isf = s.bco.makeLabel();
    BytecodeObject::Label_t ist = s.bco.makeLabel();

    // Combinations starting with ubool - these do not change the condition
    // ubool, jep -> jep
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // ubool, jfp -> jfp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // ubool, jtp -> jtp
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unBool, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // Trailer to keep things recognizable
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ise);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, isf);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maJump,    Opcode::jSymbolic, ist);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 11U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf));
    a.check("14. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("15. insn 4", isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist));
    a.check("16. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("17. insn 6", isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    a.check("18. insn 7", isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unInc, 0));
    a.check("19. insn 8", isInstruction(s.bco(8),  Opcode::maJump,    Opcode::jSymbolic, isf));
    a.check("20. insn 9", isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unDec, 0));
    a.check("21. insn 10", isInstruction(s.bco(10), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/*
 *  FoldUnary - fold unary operation following a push literal
 */

/** Test folding of push-literal + uzap. */
AFL_TEST("interpreter.Optimizer:fold-unary:zap", a)
{
    // ex test_opt.qs:in.FoldUnary1
    Stuff s;

    // Various 'zap' instructions
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   2);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   -1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 7U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPush, Opcode::sBoolean, -1));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maPush, Opcode::sInteger, 1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maPush, Opcode::sInteger, 2));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maPush, Opcode::sInteger, -1));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maPush, Opcode::sBoolean, -1));
    a.check("07. insn 5", isInstruction(s.bco(5), Opcode::maPush, Opcode::sBoolean, 1));
    a.check("08. insn 6", isInstruction(s.bco(6), Opcode::maPush, Opcode::sBoolean, -1));
}

/** Test folding of push-literal + uneg. */
AFL_TEST("interpreter.Optimizer:fold-unary:neg", a)
{
    // ex test_opt.qs:in.FoldUnary2
    Stuff s;

    // Various 'neg' instructions
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   2);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   -1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 7U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPush, Opcode::sInteger, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maPush, Opcode::sInteger, -1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maPush, Opcode::sInteger, -2));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maPush, Opcode::sInteger, 1));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maPush, Opcode::sInteger, 0));
    a.check("07. insn 5", isInstruction(s.bco(5), Opcode::maPush, Opcode::sInteger, -1));
    a.check("08. insn 6", isInstruction(s.bco(6), Opcode::maPush, Opcode::sBoolean, -1));
}

/** Test folding of push-literal + different unary operations. */
AFL_TEST("interpreter.Optimizer:fold-unary:other", a)
{
    // ex test_opt.qs:in.FoldUnary3
    Stuff s;

    // All instructions applied to an integer
    // zap/neg/pos
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unZap, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    // not/not2/bool
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unNot2, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBool, 0);

    // abs/isempty/isstr
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unAbs, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsEmpty, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsString, 0);

    // isnum/trunc/round
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unIsNum, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unTrunc, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unRound, 0);

    // inc/dec/bitnot
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   10);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unBitNot, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 15U);

    a.check("11. insn 0", isInstruction(s.bco(0),  Opcode::maPush, Opcode::sInteger, 10));
    a.check("12. insn 1", isInstruction(s.bco(1),  Opcode::maPush, Opcode::sInteger, -10));
    a.check("13. insn 2", isInstruction(s.bco(2),  Opcode::maPush, Opcode::sInteger, 10));

    a.check("21. insn 3", isInstruction(s.bco(3),  Opcode::maPush, Opcode::sBoolean, 0));
    a.check("22. insn 4", isInstruction(s.bco(4),  Opcode::maPush, Opcode::sBoolean, 0));
    a.check("23. insn 5", isInstruction(s.bco(5),  Opcode::maPush, Opcode::sBoolean, 1));

    a.check("31. insn 6", isInstruction(s.bco(6),  Opcode::maPush, Opcode::sInteger, 10));
    a.check("32. insn 7", isInstruction(s.bco(7),  Opcode::maPush, Opcode::sBoolean, 0));
    a.check("33. insn 8", isInstruction(s.bco(8),  Opcode::maPush, Opcode::sBoolean, 0));

    a.check("41. insn 9", isInstruction(s.bco(9),  Opcode::maPush, Opcode::sBoolean, 1));
    a.check("42. insn 10", isInstruction(s.bco(10), Opcode::maPush, Opcode::sInteger, 10));
    a.check("43. insn 11", isInstruction(s.bco(11), Opcode::maPush, Opcode::sInteger, 10));

    a.check("51. insn 12", isInstruction(s.bco(12), Opcode::maPush, Opcode::sInteger, 11));
    a.check("52. insn 13", isInstruction(s.bco(13), Opcode::maPush, Opcode::sInteger, 9));
    a.check("53. insn 14", isInstruction(s.bco(14), Opcode::maPush, Opcode::sInteger, -11));
}

/** Test folding of push-literal + different unary operations with boundary cases. */
AFL_TEST("interpreter.Optimizer:fold-unary:boundary-cases", a)
{
    // ex test_opt.qs:in.FoldUnary4
    Stuff s;

    // Boundary cases. We don't currently translate a pushint into a pushlit.
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   32767);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   32767);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   -32768);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,   -32768);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 6U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPush,  Opcode::sInteger, 32767));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maPush,  Opcode::sInteger, 32766));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maPush,  Opcode::sInteger, -32767));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maPush,  Opcode::sInteger, -32768));
    a.check("07. insn 5", isInstruction(s.bco(5), Opcode::maUnary, interpreter::unDec, 0));
}

/*
 *  FoldBinaryInt - pushint + binary operation -> unary operation
 */

/** Test folding of push-literal + badd. */
AFL_TEST("interpreter.Optimizer:fold-binary:add", a)
{
    // ex test_opt.qs:in.FoldBinaryInt1
    Stuff s;

    // pushint 1, badd -> uinc
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biAdd,     0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint -1, badd -> udec
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,      -1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biAdd,     0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint 0, badd -> upos
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       0);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biAdd,     0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 5U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test folding of push-literal + bsub. */
AFL_TEST("interpreter.Optimizer:fold-binary:sub", a)
{
    // ex test_opt.qs:in.FoldBinaryInt2
    Stuff s;

    // pushint 1, bsub -> udec
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biSub,     0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint -1, bsub -> uinc
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,      -1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biSub,     0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint 0, bsub -> upos
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       0);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biSub,     0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 5U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary,   interpreter::unDec,     0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unInc,     0));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("06. insn 4", isInstruction(s.bco(4), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test folding of push-literal + bmul/bdiv/bidiv. */
AFL_TEST("interpreter.Optimizer:fold-binary:mul", a)
{
    // ex test_opt.qs:in.FoldBinaryInt3
    Stuff s;

    // pushint 1, bmul -> upos
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biMult,    0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint -1, bmul -> uneg
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,      -1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biMult,    0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint 1, bdiv -> upos
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biDivide,  0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint -1, bdiv -> uneg
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,      -1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biDivide,  0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint 1, bidiv -> upos
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biIntegerDivide, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // pushint -1, bidiv -> uneg
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger,      -1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biIntegerDivide, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 11U);
    a.check("02. insn 0", isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("03. insn 1", isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("04. insn 2", isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("05. insn 3", isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("06. insn 4", isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("07. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("08. insn 6", isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unNeg,     0));
    a.check("09. insn 7", isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("10. insn 8", isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unPos,     0));
    a.check("11. insn 9", isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    a.check("12. insn 10", isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
}

/** Test folding of push-literal + bpow. */
AFL_TEST("interpreter.Optimizer:fold-binary:pow", a)
{
    // ex test_opt.qs:in.FoldBinaryInt4
    Stuff s;

    // pushint 1, bpow -> upos
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 1);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biPow, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unPos, 0));
}

/*
 *  FoldJump - Jump on constant condition
 */

/** Test folding of push-literal + conditional jump, with true condition. */
AFL_TEST("interpreter.Optimizer:fold-jump:true", a)
{
    // ex test_opt.qs:in.FoldJump1
    Stuff s;
    BytecodeObject::Label_t la = s.bco.makeLabel();
    BytecodeObject::Label_t lb = s.bco.makeLabel();
    BytecodeObject::Label_t lc = s.bco.makeLabel();
    BytecodeObject::Label_t lend = s.bco.makeLabel();

    // Jump on true conditions (=pushint 1). Starting with jdz block to make all labels referenced.
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, la);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lb);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lc);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  la);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lb);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lc);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lend);

    // Use level 1 only for now, level 2 will trigger tail merging
    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 12U);

    a.check("11. insn 3", isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    a.check("12. insn 4", isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));
    a.check("13. insn 5", isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, lend));

    a.check("21. insn 6", isInstruction(s.bco(6),  Opcode::maJump,  Opcode::jSymbolic, lb));
    a.check("22. insn 7", isInstruction(s.bco(7),  Opcode::maUnary, interpreter::unDec, 0));

    a.check("31. insn 8", isInstruction(s.bco(8),  Opcode::maJump,  Opcode::jSymbolic, lc));
    a.check("32. insn 9", isInstruction(s.bco(9),  Opcode::maUnary, interpreter::unNeg, 0));
    a.check("33. insn 10", isInstruction(s.bco(10), Opcode::maUnary, interpreter::unInc, 0));

    a.check("41. insn 11", isInstruction(s.bco(11), Opcode::maJump,  Opcode::jSymbolic, lend));

    // Now optimize again with level 2 to exercise tail merging
    optimize(s.world, s.bco, 2);

    a.checkEqual("51. getNumInstructions", s.bco.getNumInstructions(), 11U);

    a.check("61. insn 3", isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    a.check("62. insn 4", isInstruction(s.bco(4),  Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways));

    a.check("71. insn 5", isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic, lb));
    a.check("72. insn 6", isInstruction(s.bco(6),  Opcode::maUnary, interpreter::unDec, 0));

    a.check("81. insn 7", isInstruction(s.bco(7),  Opcode::maJump,  Opcode::jSymbolic, lc));
    a.check("82. insn 8", isInstruction(s.bco(8),  Opcode::maUnary, interpreter::unNeg, 0));
    a.check("83. insn 9", isInstruction(s.bco(9),  Opcode::maJump,  Opcode::jSymbolic));

    a.check("91. insn 10", isInstruction(s.bco(10), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test folding of push-literal + conditional jump, with false condition. */
AFL_TEST("interpreter.Optimizer:fold-jump:false", a)
{
    // ex test_opt.qs:in.FoldJump2
    Stuff s;
    BytecodeObject::Label_t la = s.bco.makeLabel();
    BytecodeObject::Label_t lb = s.bco.makeLabel();
    BytecodeObject::Label_t lc = s.bco.makeLabel();
    BytecodeObject::Label_t lend = s.bco.makeLabel();

    // Jump on false conditions (=pushint 0). Starting with jdz block to make all labels referenced.
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, la);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lb);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lc);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  la);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lb);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lc);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger,   0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lend);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 12U);

    a.check("11. insn 3", isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    a.check("12. insn 4", isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));

    a.check("21. insn 5", isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic, lb));
    a.check("22. insn 6", isInstruction(s.bco(6),  Opcode::maUnary, interpreter::unDec, 0));
    a.check("23. insn 7", isInstruction(s.bco(7),  Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, lend));

    a.check("31. insn 8", isInstruction(s.bco(8),  Opcode::maJump,  Opcode::jSymbolic, lc));
    a.check("32. insn 9", isInstruction(s.bco(9),  Opcode::maUnary, interpreter::unNeg, 0));
    a.check("33. insn 10", isInstruction(s.bco(10), Opcode::maUnary, interpreter::unInc, 0));

    a.check("41. insn 11", isInstruction(s.bco(11), Opcode::maJump,  Opcode::jSymbolic, lend));
}

/** Test folding of push-literal + conditional jump, with empty condition. */
AFL_TEST("interpreter.Optimizer:fold-jump:empty", a)
{
    // ex test_opt.qs:in.FoldJump3
    Stuff s;
    BytecodeObject::Label_t la = s.bco.makeLabel();
    BytecodeObject::Label_t lb = s.bco.makeLabel();
    BytecodeObject::Label_t lc = s.bco.makeLabel();
    BytecodeObject::Label_t lend = s.bco.makeLabel();

    // Jump on empty conditions (=pushbool -1). Starting with jdz block to make all labels referenced.
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, la);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lb);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lc);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  la);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lb);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lc);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, lend);

    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lend);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 9U);

    a.check("11. insn 3", isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    a.check("12. insn 4", isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));

    a.check("21. insn 5", isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic, lb));
    a.check("22. insn 6", isInstruction(s.bco(6),  Opcode::maUnary, interpreter::unDec, 0));

    a.check("31. insn 7", isInstruction(s.bco(7),  Opcode::maJump,  Opcode::jSymbolic, lc));
    a.check("32. insn 8", isInstruction(s.bco(8),  Opcode::maUnary, interpreter::unNeg, 0));
}

/** Test folding of push-literal + conditional jump, with a non-popping jump. */
AFL_TEST("interpreter.Optimizer:fold-jump:non-popping", a)
{
    // ex test_opt.qs:in.FoldJump4
    Stuff s;
    BytecodeObject::Label_t la = s.bco.makeLabel();
    BytecodeObject::Label_t lb = s.bco.makeLabel();
    BytecodeObject::Label_t lc = s.bco.makeLabel();
    BytecodeObject::Label_t lend = s.bco.makeLabel();

    // Jump on empty conditions without pop.
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, la);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lb);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jDecZero, lc);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  la);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfTrue, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lb);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unDec, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfFalse, lend);

    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lc);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unNeg, 0);
    s.bco.addInstruction(Opcode::maPush,   Opcode::sBoolean,   -1);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic | Opcode::jIfEmpty, lend);

    s.bco.addInstruction(Opcode::maUnary,  interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump,   Opcode::jSymbolic,  lend);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 12U);

    a.check("11. insn 3", isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    a.check("12. insn 4", isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));
    a.check("13. insn 5", isInstruction(s.bco(5),  Opcode::maPush,  Opcode::sBoolean, -1));

    a.check("21. insn 6", isInstruction(s.bco(6),  Opcode::maJump,  Opcode::jSymbolic, lb));
    a.check("22. insn 7", isInstruction(s.bco(7),  Opcode::maUnary, interpreter::unDec, 0));
    a.check("23. insn 8", isInstruction(s.bco(8),  Opcode::maPush,  Opcode::sBoolean, -1));

    a.check("31. insn 9", isInstruction(s.bco(9),  Opcode::maJump,  Opcode::jSymbolic, lc));
    a.check("32. insn 10", isInstruction(s.bco(10), Opcode::maUnary, interpreter::unNeg, 0));
    a.check("33. insn 11", isInstruction(s.bco(11), Opcode::maPush,  Opcode::sBoolean, -1));
}

/*
 *  PopPush - pop+push -> store if we're sure the value is preserved
 */

/** Test poploc+pushloc -> storeloc. */
AFL_TEST("interpreter.Optimizer:pop-push", a)
{
    // ex test_opt.qs:in.PopPush1
    Stuff s;

    // poploc X, pushloc X -> storeloc X
    uint16_t lv = s.bco.addLocalVariable("A");
    s.bco.addInstruction(Opcode::maPop,  Opcode::sLocal, lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sLocal, lv);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStore, Opcode::sLocal));
    a.check("03. isLocalVariableName", isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test popvar+pushvar; not optimized because it implies a type-cast. */
AFL_TEST("interpreter.Optimizer:pop-push:var", a)
{
    // ex test_opt.qs:in.PopPush2
    Stuff s;

    // pop/push using name is not optimized because it implies a type-cast
    uint16_t lv = s.bco.addName("A");
    s.bco.addInstruction(Opcode::maPop,  Opcode::sNamedVariable, lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, lv);

    optimize(s.world, s.bco, 1);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPop,  Opcode::sNamedVariable));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maPush, Opcode::sNamedVariable));
    a.check("04. isName", isName(s.bco, s.bco(0).arg, "A"));
    a.check("05. isName", isName(s.bco, s.bco(1).arg, "A"));
}

/** Test poploc+pushvar; not optimized because of different scope. */
AFL_TEST("interpreter.Optimizer:pop-push:scope", a)
{
    // ex test_opt.qs:in.PopPush3
    Stuff s;

    // pop/push using different scope
    uint16_t lv = s.bco.addLocalVariable("A");
    uint16_t gv = s.bco.addName("A");
    a.checkEqual("01", lv, gv);

    s.bco.addInstruction(Opcode::maPop,  Opcode::sLocal,       lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, gv);

    optimize(s.world, s.bco, 1);

    a.checkEqual("11. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("12. insn 0", isInstruction(s.bco(0), Opcode::maPop,  Opcode::sLocal,       lv));
    a.check("13. insn 1", isInstruction(s.bco(1), Opcode::maPush, Opcode::sNamedShared, gv));
}

/*
 *  CompareNC - drop the "NC" if we're sure it doesn't make a difference
 */

/** Test caseblind instructions that can be made case-preserving. */
AFL_TEST("interpreter.Optimizer:compare-nc:match", a)
{
    // ex test_opt.qs:in.CompareNC1
    Stuff s;
    afl::data::StringValue sv("");

    // Verify that all eligible instructions are accepted
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareEQ_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biFirstStr_NC,  0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biRestStr_NC,   0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biFindStr_NC,   0);

    optimize(s.world, s.bco, 2);

    // Note that push+binary gets fused into fusedbinary (pushlit(b)).
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 14U);
    a.check("02. insn 0", isInstruction(s.bco(0),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("03. insn 1", isInstruction(s.bco(1),  Opcode::maBinary,      interpreter::biCompareEQ, 0));
    a.check("04. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("05. insn 3", isInstruction(s.bco(3),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("06. insn 4", isInstruction(s.bco(4),  Opcode::maBinary,      interpreter::biCompareNE, 0));
    a.check("07. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("08. insn 6", isInstruction(s.bco(6),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("09. insn 7", isInstruction(s.bco(7),  Opcode::maBinary,      interpreter::biFirstStr, 0));
    a.check("10. insn 8", isInstruction(s.bco(8),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("11. insn 9", isInstruction(s.bco(9),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("12. insn 10", isInstruction(s.bco(10), Opcode::maBinary,      interpreter::biRestStr, 0));
    a.check("13. insn 11", isInstruction(s.bco(11), Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("14. insn 12", isInstruction(s.bco(12), Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("15. insn 13", isInstruction(s.bco(13), Opcode::maBinary,      interpreter::biFindStr, 0));
}

/** Test caseblind instructions that can NOT be made case-preserving. */
AFL_TEST("interpreter.Optimizer:compare-nc:mismatch", a)
{
    // ex test_opt.qs:in.CompareNC2
    Stuff s;
    afl::data::StringValue sv("[");

    // Some instructions that are not accepted.
    // "[" sorts differently depending on whether we're caseblind or not.
    // Therefore, these _NC instructions are not converted.
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareGE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareGT_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareLE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint,      0);
    s.bco.addPushLiteral(&sv);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareLT_NC, 0);

    optimize(s.world, s.bco, 2);

    // Note that push+binary gets fused into fusedbinary (pushlit(b)).
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 11U);
    a.check("02. insn 0", isInstruction(s.bco(0),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("03. insn 1", isInstruction(s.bco(1),  Opcode::maBinary,      interpreter::biCompareGE_NC, 0));
    a.check("04. insn 2", isInstruction(s.bco(2),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("05. insn 3", isInstruction(s.bco(3),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("06. insn 4", isInstruction(s.bco(4),  Opcode::maBinary,      interpreter::biCompareGT_NC, 0));
    a.check("07. insn 5", isInstruction(s.bco(5),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("08. insn 6", isInstruction(s.bco(6),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("09. insn 7", isInstruction(s.bco(7),  Opcode::maBinary,      interpreter::biCompareLE_NC, 0));
    a.check("10. insn 8", isInstruction(s.bco(8),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    a.check("11. insn 9", isInstruction(s.bco(9),  Opcode::maFusedBinary, Opcode::sLiteral));
    a.check("12. insn 10", isInstruction(s.bco(10), Opcode::maBinary,      interpreter::biCompareLT_NC, 0));
}

/** Test caseblind instructions that can be made case-preserving with the given operands. */
AFL_TEST("interpreter.Optimizer:compare-nc:match:2", a)
{
    // ex test_opt.qs:in.CompareNC3
    Stuff s;
    afl::data::StringValue emptySV("");
    afl::data::StringValue dotSV(".");
    afl::data::StringValue bracketSV("[");
    afl::data::StringValue braceSV("}");
    afl::data::FloatValue oneFV(1.0);
    afl::data::IntegerValue bigIV(999999);

    // Compare-inequal with different literals; all are accepted
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger, 1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maPush,    Opcode::sBoolean, 1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&bigIV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&oneFV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&emptySV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&dotSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&bracketSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&braceSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 23U);
    for (size_t i = 0; i < 8; ++i) {
        a.check("02", isInstruction(s.bco(1 + 3*i), Opcode::maBinary, interpreter::biCompareNE, 0));
    }
}

/** Test caseblind instructions that can NOT be made case-preserving with the given operands. */
AFL_TEST("interpreter.Optimizer:compare-nc:mismatch:2", a)
{
    // ex test_opt.qs:in.CompareNC4
    Stuff s;
    afl::data::StringValue ucSV("A");
    afl::data::StringValue lcSV("a");
    afl::data::StringValue longSV("......a......");

    // Test some operands that are not accepted
    s.bco.addInstruction(Opcode::maPush,    Opcode::sNamedVariable, s.bco.addName("A"));
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&ucSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&lcSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addPushLiteral(&longSV);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 11U);
    for (size_t i = 0; i < 3; ++i) {
        a.check("02", isInstruction(s.bco(1 + 3*i), Opcode::maBinary, interpreter::biCompareNE_NC, 0));
    }
}

/*
 *  Optimisation failures
 */

/** Test failure to optimize due to absolute jump. */
AFL_TEST("interpreter.Optimizer:error:absolute-jump", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jIfEmpty, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maJump,  Opcode::jIfEmpty,    0));
}

/** Test failure to optimize due to absolute label.
    (An absolute label is a no-op.)*/
AFL_TEST("interpreter.Optimizer:error:absolute-label", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maJump,  0,                   0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maJump,  0,                   0));
}

/** Test failure to optimize FoldUnary (un-optimizable unary operation). */
AFL_TEST("interpreter.Optimizer:error:fold-unary", a)
{
    Stuff s;

    // pushint 1, uatomstr -> not optimized, needs runtime state
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unAtomStr, 0);

    // pushint 1, uinc -> optimized, for comparison
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,     0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maPush,  Opcode::sInteger, 1));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary, interpreter::unAtomStr, 0));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maPush,  Opcode::sInteger, 2));
}

/** Test failure to optimize FoldBinary (un-optimizable operand). */
AFL_TEST("interpreter.Optimizer:error:fold-binary", a)
{
    Stuff s;

    // pushint 2, badd -> not optimized
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 2);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biAdd, 0);

    // pushint 2, bsub -> not optimized
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 2);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biSub, 0);

    // pushint 3, bmul -> not optimized
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 2);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biMult, 0);

    // pushint 2, bpow -> not optimized
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 2);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biPow, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 8U);
    a.check("02. insn 1", isInstruction(s.bco(1), Opcode::maBinary, interpreter::biAdd, 0));
    a.check("03. insn 3", isInstruction(s.bco(3), Opcode::maBinary, interpreter::biSub, 0));
    a.check("04. insn 5", isInstruction(s.bco(5), Opcode::maBinary, interpreter::biMult, 0));
    a.check("05. insn 7", isInstruction(s.bco(7), Opcode::maBinary, interpreter::biPow, 0));
}

/** Test folding of integer comparison. We had a bug here. */
AFL_TEST("interpreter.Optimizer:int-comparison", a)
{
    // Verify actual execution
    checkExpression(a, "if(instr('a', 'ba')=0, 3, 12)", 3, 2);
    checkExpression(a, "if(instr('a', 'ba')<>0, 3, 12)", 12, 2);
    checkExpression(a, "if(instr('ba', 'a')=0, 3, 12)", 12, 2);
    checkExpression(a, "if(instr('ba', 'a')<>0, 3, 12)", 3, 2);

    checkExpression(a, "if(bitand(1, 2)=0, 3, 12)", 3, 0);
    checkExpression(a, "if(bitand(1, 2)=0, 3, 12)", 3, 2);
    checkExpression(a, "if(bitand(1, 2)<>0, 3, 12)", 12, 0);
    checkExpression(a, "if(bitand(1, 2)<>0, 3, 12)", 12, 2);
    checkExpression(a, "if(bitand(z(0), 2)=0, 3, 12)", 12, 0);
    checkExpression(a, "if(bitand(z(0), 2)=0, 3, 12)", 12, 2);
    checkExpression(a, "if(bitand(z(0), 2)<>0, 3, 12)", 12, 0);
    checkExpression(a, "if(bitand(z(0), 2)<>0, 3, 12)", 12, 2);

    // Verify patterns
    Stuff s;

    // bfindstr, pushint 0, bcmpeq -> bfindstr, unot
    s.bco.addInstruction(Opcode::maBinary, interpreter::biFindStr, 0);
    s.bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareEQ, 0);

    // bfindstr, pushint 0, bcmpne -> bfindstr, ubool
    s.bco.addInstruction(Opcode::maBinary, interpreter::biFindStr, 0);
    s.bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareNE, 0);

    // bfindstr, pushint 1, bcmpne -> unchanged
    s.bco.addInstruction(Opcode::maBinary, interpreter::biFindStr, 0);
    s.bco.addInstruction(Opcode::maPush, Opcode::sInteger, 1);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareNE, 0);

    // bfindstr, pushint 0, bcmpge -> unchanged
    s.bco.addInstruction(Opcode::maBinary, interpreter::biFindStr, 0);
    s.bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareGE, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 10U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maBinary, interpreter::biFindStr, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary,  interpreter::unNot, 0));

    a.check("11. insn 2", isInstruction(s.bco(2), Opcode::maBinary, interpreter::biFindStr, 0));
    a.check("12. insn 3", isInstruction(s.bco(3), Opcode::maUnary,  interpreter::unBool, 0));

    a.check("21. insn 4", isInstruction(s.bco(4), Opcode::maBinary, interpreter::biFindStr, 0));
    a.check("22. insn 5", isInstruction(s.bco(5), Opcode::maPush,   Opcode::sInteger, 1));
    a.check("23. insn 6", isInstruction(s.bco(6), Opcode::maBinary, interpreter::biCompareNE, 0));

    a.check("31. insn 7", isInstruction(s.bco(7), Opcode::maBinary, interpreter::biFindStr, 0));
    a.check("32. insn 8", isInstruction(s.bco(8), Opcode::maPush,   Opcode::sInteger, 0));
    a.check("33. insn 9", isInstruction(s.bco(9), Opcode::maBinary, interpreter::biCompareGE, 0));
}

/** Test doTailMerge(). */
AFL_TEST("interpreter.Optimizer:tail-merge", a)
{
    // Verify actual execution
    checkExpression(a, "if(1, 4+5, 3+5)", 9, 1);
    checkExpression(a, "if(1, 4+5, 3+5)", 9, 2);
    checkExpression(a, "if(0, 4+5, 3+5)", 8, 1);
    checkExpression(a, "if(0, 4+5, 3+5)", 8, 2);

    // Verify pattern: 'if (a, b+1, c+1)'
    Stuff s;
    BytecodeObject::Label_t lElse = s.bco.makeLabel();
    BytecodeObject::Label_t lEnd = s.bco.makeLabel();

    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, s.bco.addName("A"));
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic + Opcode::jIfFalse + Opcode::jIfEmpty + Opcode::jPopAlways, lElse);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, s.bco.addName("B"));
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic + Opcode::jAlways, lEnd);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic, lElse);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, s.bco.addName("C"));
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic, lEnd);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 8U);

    a.check("11. insn 0", isInstruction(s.bco(0), Opcode::maPush, Opcode::sNamedVariable));
    a.check("12. insn 1", isInstruction(s.bco(1), Opcode::maJump, Opcode::jSymbolic + Opcode::jIfFalse + Opcode::jIfEmpty + Opcode::jPopAlways, lElse));
    a.check("13. insn 2", isInstruction(s.bco(2), Opcode::maPush, Opcode::sNamedVariable));
    a.check("14. insn 3", isInstruction(s.bco(3), Opcode::maJump, Opcode::jSymbolic + Opcode::jAlways));
    a.check("15. insn 4", isInstruction(s.bco(4), Opcode::maJump, Opcode::jSymbolic));
    a.check("16. insn 5", isInstruction(s.bco(5), Opcode::maPush, Opcode::sNamedVariable));
    a.check("17. insn 6", isInstruction(s.bco(6), Opcode::maJump, Opcode::jSymbolic));
    a.check("18. insn 7", isInstruction(s.bco(7), Opcode::maUnary, interpreter::unInc));
}

/** Test failure to optimize because of label inconsistencies:
    Verify preconditions for future tests */
AFL_TEST("interpreter.Optimizer:error:missing-label", a)
{
    Stuff s;
    s.bco.setNumLabels(20);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic, 7);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maUnary, interpreter::unPos));
}

/** Test failure to optimize because of absolute label. */
AFL_TEST("interpreter.Optimizer:error:absolute-label:2", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maJump, 0, 99);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    // Number of instructions unchanged
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
}

/** Test failure to optimize because of absolute jump */
AFL_TEST("interpreter.Optimizer:error:absolute-jump:2", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maJump, Opcode::jAlways, 2);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    // Number of instructions unchanged
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
}

/** Test failure to optimize because of out-of-range label. This used to assert. */
AFL_TEST("interpreter.Optimizer:error:out-of-range-label", a)
{
    Stuff s;
    s.bco.setNumLabels(44);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic, 44);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    // Number of instructions unchanged
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
}

/** Test failure to optimize because of out-of-range jump. This used to assert. */
AFL_TEST("interpreter.Optimizer:error:out-of-range-jump", a)
{
    Stuff s;
    s.bco.setNumLabels(44);
    s.bco.addInstruction(Opcode::maJump, Opcode::jSymbolic + Opcode::jAlways, 44);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    // Number of instructions unchanged
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
}

/** Test dead store removal: "return" case. */
AFL_TEST("interpreter.Optimizer:dead-store:return", a)
{
    Stuff s;
    BytecodeObject::Label_t label = s.bco.makeLabel();
    uint16_t var = s.bco.addLocalVariable("X");

    // Label to make stuff after return referenced
    s.bco.addJump(Opcode::jDecZero, label);

    // Return
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    s.bco.addInstruction(Opcode::maStore,   Opcode::sLocal, var);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unAbs, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);

    // After return
    s.bco.addLabel(label);
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger, 42);

    optimize(s.world, s.bco, 2);

    // 5 instructions remain
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 6U);
    a.check("02. insn 1", isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialNewHash, 0));
    a.check("03. insn 2", isInstruction(s.bco(2), Opcode::maUnary, interpreter::unAbs,         0));
    a.check("04. insn 3", isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialReturn,  1));
}

/** Test dead store removal: "return at end of function" case. */
AFL_TEST("interpreter.Optimizer:dead-store:return-at-end", a)
{
    Stuff s;
    uint16_t var = s.bco.addLocalVariable("X");

    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    s.bco.addInstruction(Opcode::maPop,     Opcode::sLocal, var);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unAbs, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);

    optimize(s.world, s.bco, 2);

    // 4 instructions remain, pop has been converted into drop
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 4U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialNewHash, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackDrop,      1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unAbs,       0));
    a.check("05. insn 3", isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialReturn,  1));
}

/** Test dead store removal: "end of function" case. */
AFL_TEST("interpreter.Optimizer:dead-store:end", a)
{
    Stuff s;
    uint16_t var = s.bco.addLocalVariable("X");

    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
    s.bco.addInstruction(Opcode::maPop,     Opcode::sLocal, var);
    s.bco.addInstruction(Opcode::maUnary,   interpreter::unAbs, 0);

    optimize(s.world, s.bco, 2);

    // 4 instructions remain, pop has been converted into drop
    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 3U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialNewHash, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackDrop,      1));
    a.check("04. insn 2", isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unAbs,       0));
}

/** Test type check removal for binary operations, boolean case. */
AFL_TEST("interpreter.Optimizer:fold-typecheck:bool", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareGE, 0);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unBool, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maBinary, interpreter::biCompareGE, 0));
}

/** Test type check removal for binary operations, integer case. */
AFL_TEST("interpreter.Optimizer:fold-typecheck:int", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maBinary, interpreter::biBitAnd, 0);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 1U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maBinary, interpreter::biBitAnd, 0));
}

/** Test type check removal for binary operations, negative case.
    A type check that actually changes the type needs to remain. */
AFL_TEST("interpreter.Optimizer:fold-typecheck:mismatch", a)
{
    Stuff s;
    s.bco.addInstruction(Opcode::maBinary, interpreter::biCompareGE, 0);
    s.bco.addInstruction(Opcode::maUnary,  interpreter::unPos, 0);

    optimize(s.world, s.bco, 2);

    a.checkEqual("01. getNumInstructions", s.bco.getNumInstructions(), 2U);
    a.check("02. insn 0", isInstruction(s.bco(0), Opcode::maBinary, interpreter::biCompareGE, 0));
    a.check("03. insn 1", isInstruction(s.bco(1), Opcode::maUnary,  interpreter::unPos,       0));
}
