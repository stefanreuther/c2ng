/**
  *  \file u/t_interpreter_optimizer.cpp
  *  \brief Test for interpreter::Optimizer
  *
  *  In PCC2, this test was implemented as a *.qs file and a perl script.
  *  The *.qs file was compiled into a *.qc file using c2asm, loaded by the 'teststmt' application, and verified by the perl script.
  *  That needs quite a lot of other code other than just the optimizer, and more infrastructure (perl).
  *  Doing this in C++ is a little more boilerplate code, but nicely integrates in the testsuite.
  *
  *  One key difference is that the 'teststmt' application always linearized after optimization while we don't.
  */

#include <iostream>
#include "interpreter/optimizer.hpp"

#include "t_interpreter.hpp"
#include "afl/sys/log.hpp"
#include "afl/io/nullfilesystem.hpp"
#include "interpreter/world.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/floatvalue.hpp"

using interpreter::Opcode;
using interpreter::BytecodeObject;

namespace {
    struct Stuff {
        afl::sys::Log log;
        afl::io::NullFileSystem fs;
        interpreter::World world;
        BytecodeObject bco;

        Stuff()
            : log(), fs(), world(log, fs), bco()
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
        const afl::data::NameMap& names = bco.getLocalNames();
        return (index < names.getNumNames()
                && names.getNameByIndex(index) == name);
    }

    bool isName(const BytecodeObject& bco, uint16_t index, String_t name)
    {
        const afl::data::NameMap& names = bco.getNames();
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
}

/*
 *  StoreDrop - merging store+drop -> pop
 */

/** Test storeloc a + drop 1 -> poploc a (drop removed). */
void
TestInterpreterOptimizer::testStoreDrop1()
{
    // ex test_opt.qs:in.storeDrop1:
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    TS_ASSERT(isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storeloc a, drop 2 -> poploc a, drop 1 (drop remains). */
void
TestInterpreterOptimizer::testStoreDrop2()
{
    // ex test_opt.qs:in.storeDrop2
    // storeloc a + drop 2 -> storeloc a + drop 1
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 2);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    TS_ASSERT(isLocalVariableName(s.bco, s.bco(0).arg, "A"));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
}

/** Test storeloc a, drop 0 (removes the drop, does not create invalid drop -1). */
void
TestInterpreterOptimizer::testStoreDrop3()
{
    // ex test_opt.qs:in.storeDrop3
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStore, Opcode::sLocal));
    TS_ASSERT(isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storeloc a + drop 0 + drop 1 -> poploc (drops are combined, then eliminated). */
void
TestInterpreterOptimizer::testStoreDrop4()
{
    // ex test_opt.qs:in.storeDrop4
    // storeloc + drop 0 + drop 1 -> poploc
    Stuff s;
    s.bco.addInstruction(Opcode::maStore, Opcode::sLocal, s.bco.addLocalVariable("A"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPop, Opcode::sLocal));
    TS_ASSERT(isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test storemem + drop -> popmem (maMemref instead of maStore). */
void
TestInterpreterOptimizer::testStoreDrop5()
{
    Stuff s;
    s.bco.addInstruction(Opcode::maMemref, Opcode::miIMStore, s.bco.addName("XY"));
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maMemref, Opcode::miIMPop));
    TS_ASSERT(isName(s.bco, s.bco(0).arg, "XY"));
}

/*
 *  MergeDrop - merging multiple drop statements into one
 */

/** Test merging multiple drop into one. */
void
TestInterpreterOptimizer::testMergeDrop1()
{
    // ex test_opt.qs:in.mergeDrop1
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 3));
}

/** Test merging multiple drop into one, even if some of them have count 0. */
void
TestInterpreterOptimizer::testMergeDrop2()
{
    // ex test_opt.qs:in.mergeDrop2
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 0);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 2);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 3));
}

/*
 *  NullOp - removing null operations (and preserving those that look like null ops but aren't)
 *
 *  Wrap the tests into guaranteed-unoptimizable instructions to avoid that the optimizer sees
 *  special cases at the end of the sub.
 */

/** Test removal of null operation "drop 0". */
void
TestInterpreterOptimizer::testNullOp1()
{
    // ex test_opt.qs:in.nullOp1
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackDrop,      0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test removal of null operation "swap 0". */
void
TestInterpreterOptimizer::testNullOp2()
{
    // ex test_opt.qs:in.nullOp2
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackSwap,      0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test preservation of non-null operation "dup 0". */
void
TestInterpreterOptimizer::testNullOp3()
{
    // ex test_opt.qs:in.nullOp3
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackDup,       0); // not a null op!
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackDup,       0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/** Test preservation of non-null operation "swap 1". */
void
TestInterpreterOptimizer::testNullOp4()
{
    // ex test_opt.qs:in.nullOp4
    Stuff s;
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
    s.bco.addInstruction(Opcode::maStack,   Opcode::miStackSwap,      1); // not a null op!
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maStack,   Opcode::miStackSwap,      1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maSpecial, Opcode::miSpecialSuspend, 0));
}

/*
 *  EraseUnusedLabels
 */

/** Test removal of unused labels. */
void
TestInterpreterOptimizer::testEraseUnusedLabels1()
{
    // ex test_opt.qs:in.eraseUnusedLabels1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jt #1, label #0, uinc, label #1, udec
    // -> remove label #0.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unDec,                  0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 4U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec,                  0));
}

/** Test removal of unused labels that enables further optimisation. */
void
TestInterpreterOptimizer::testEraseUnusedLabels2()
{
    // ex test_opt.qs:in.eraseUnusedLabels2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 4U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jIfTrue | Opcode::jSymbolic, l1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec,                  0));
}

/*
 *  InvertJumps - jump-across-jump
 */

/** Test removal of unconditional jump-across-jump.
    (Conditional jump-across-jump is testInvertJumps6). */
void
TestInterpreterOptimizer::testInvertJumps1()
{
    // ex test_opt.qs:in.InvertJumps1
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // j #0, j #1, label #0: disappears completely
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test popping-jump-across-popping-jump.
    Optimisation does not apply here. */
void
TestInterpreterOptimizer::testInvertJumps2()
{
    // ex test_opt.qs:in.InvertJumps2
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtp #0, jfep #1, label #0: two jumps with pop; optimisation does not apply here
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maUnary, interpreter::unInc,                  0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, inverse condition. */
void
TestInterpreterOptimizer::testInvertJumps3()
{
    // ex test_opt.qs:in.InvertJumps3 (fixed, #328)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtp #0, jfe #1, label #0: two jumps with opposite condition (regular inversion case)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, similar condition. */
void
TestInterpreterOptimizer::testInvertJumps4()
{
    // ex test_opt.qs:in.InvertJumps4
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtfp #0, jt #1, label #0: second jump never taken, group degenerates into 'drop'
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse /*| Opcode::jPopAlways*/, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    //TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc,  0));
}

/** Test conditional-jump-across-unconditional-jump.
    This is the regular jump-inversion case. */
void
TestInterpreterOptimizer::testInvertJumps5()
{
    // ex test_opt.qs:in.InvertJumps5
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtp #0, j #1, label #0: conditional followed by unconditional (common case)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, l1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,                  0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                   l1));
}

/** Test conditional-jump-across-conditional-jump, same condition. */
void
TestInterpreterOptimizer::testInvertJumps6()
{
    // ex test_opt.qs:in.InvertJumps6
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtf #0, jtf #1, label #0: disappears completely
    // (same thing with unconditional jumps is testInvertJumps1)
    // FIXME: wrong!!!!1 pop changes conditional.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                                     0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test conditional-jump-across-jdz. Optimisation does not apply here. */
void
TestInterpreterOptimizer::testInvertJumps7()
{
    // ex test_opt.qs:in.InvertJumps7
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jt #0, jdz #1, label #0: optimisation does not apply here
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue,  l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jDecZero, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                   0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                    l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue,  l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic | Opcode::jDecZero, l1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maUnary, interpreter::unInc,                   0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                    l1));
}

/** Test popping-conditional-jump-across-conditional-jump, inverse condition. */
void
TestInterpreterOptimizer::testInvertJumps8()
{
    // ex test_opt.qs:in.InvertJumps3 (fixed)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtp #0, jfe #1, label #0: two jumps with opposite condition. Optimisation does not apply due to pop.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
}

/** Test conditional-jump-across-conditional-jump, similar condition. */
void
TestInterpreterOptimizer::testInvertJumps9()
{
    // ex test_opt.qs:in.InvertJumps4
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtfp #0, jt #1, label #0: optimisation does not apply
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
}

/** Test conditional-jump-across-conditional-jump, same condition. */
void
TestInterpreterOptimizer::testInvertJumps10()
{
    // ex test_opt.qs:in.InvertJumps6 (fixed, #328)
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jtfp #0, jtf #1, label #0: optimisation does not apply due to pop.
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                                     0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                                      l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
}

/** Test jump-across-jump. */
void
TestInterpreterOptimizer::testInvertJumps11()
{
    Stuff s;
    BytecodeObject::Label_t l0 = s.bco.makeLabel();
    BytecodeObject::Label_t l1 = s.bco.makeLabel();
    TS_ASSERT_EQUALS(l0, 0U);
    TS_ASSERT_EQUALS(l1, 1U);

    // jp #0, jt #1, label #0: turns into drop
    // (This could also be achieved using a combination of dead-code-removal and jump threading.)
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways | Opcode::jPopAlways, l0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l0);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,                  0);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jSymbolic,                   l1);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
}

/*
 *  ThreadJumps - optimize jump-to-jump
 */

/** Test optimisation of jump-to-jump. */
void
TestInterpreterOptimizer::testThreadJumps1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 4U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l2));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic,                                       l2));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maUnary, interpreter::unDec, 0));
}

/** Test optimisation of jump-to-jump, infinite loop. */
void
TestInterpreterOptimizer::testThreadJumps2()
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
    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump, Opcode::jSymbolic));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump, Opcode::jSymbolic | Opcode::jAlways));
    TS_ASSERT_EQUALS(s.bco(0).arg, s.bco(1).arg);
}

/** Test optimisation of jump-to-jump, infinite loop. */
void
TestInterpreterOptimizer::testThreadJumps3()
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
    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump, Opcode::jSymbolic));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump, Opcode::jSymbolic | Opcode::jAlways));
    TS_ASSERT_EQUALS(s.bco(0).arg, s.bco(1).arg);
}

/** Test optimisation of jump-to-jump, jumping into the middle of an infinite loop. */
void
TestInterpreterOptimizer::testThreadJumps4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 4U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfTrue, l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jSymbolic));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, l0));
}

/** Test optimisation of jump-to-jump that degenerates into no jump. */
void
TestInterpreterOptimizer::testThreadJumps5()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test optimisation of conditional-jump-to-jump that degenerates into no jump. */
void
TestInterpreterOptimizer::testThreadJumps6()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test optimisation of popping-conditional-jump-to-jump that degenerates into no jump. */
void
TestInterpreterOptimizer::testThreadJumps7()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc,  0));
}

/** Test jump-to-conditional-jump (not optimized). */
void
TestInterpreterOptimizer::testThreadJumps8()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 7U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic,                    l1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l2));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maJump,  Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty, l1));
    TS_ASSERT(isInstruction(s.bco(6), Opcode::maJump,  Opcode::jSymbolic,                    l2));
}

/** Test catch-to-jump. */
void
TestInterpreterOptimizer::testThreadJumps9()
{
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
void
TestInterpreterOptimizer::testThreadJumps10()
{
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
void
TestInterpreterOptimizer::testRemoveUnused1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unInc, 0));
}

/** Test removal of unused code (jumped across) with a label in the middle. */
void
TestInterpreterOptimizer::testRemoveUnused2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 6U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maJump,  Opcode::jSymbolic,                    l1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maUnary, interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maJump,  Opcode::jSymbolic,                    l0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways,  l1));
}

/** Test removal of unused code using jump-away instructions other than jump. */
void
TestInterpreterOptimizer::testRemoveUnused3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 12U);
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maJump, Opcode::jSymbolic, l0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialTerminate, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump, Opcode::jSymbolic, l1));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialThrow, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maJump, Opcode::jSymbolic, l2));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialReturn, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maJump, Opcode::jSymbolic, l3));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary, interpreter::unDec, 0));
}

/*
 *  MergeNegation - merge two unary operations
 *
 *  These tests use 'sprint' as an un-optimizable instruction to separate individual cases.
 */

/** Test merging of negation instruction pairs, starting with unot. */
void
TestInterpreterOptimizer::testMergeNegation1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 23U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(14), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(17), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with ubool. */
void
TestInterpreterOptimizer::testMergeNegation2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 22U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(14), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with upos. */
void
TestInterpreterOptimizer::testMergeNegation3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 22U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uneg. */
void
TestInterpreterOptimizer::testMergeNegation4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 24U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(13), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uzap. */
void
TestInterpreterOptimizer::testMergeNegation5()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 23U);
    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(16), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(17), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(20), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uisempty. */
void
TestInterpreterOptimizer::testMergeNegation6()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 25U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(14), Opcode::maStack,   Opcode::miStackDrop,    1));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maPush,    Opcode::sBoolean,       0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(24), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with unot2. */
void
TestInterpreterOptimizer::testMergeNegation7()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 25U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(14), Opcode::maStack,   Opcode::miStackDrop,    1));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maPush,    Opcode::sBoolean,       0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(24), Opcode::maUnary,   interpreter::unDec,     0));
}

/** Test merging of negation instruction pairs, starting with uinc. */
void
TestInterpreterOptimizer::testMergeNegation8()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 24U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(22), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test merging of negation instruction pairs, starting with udec. */
void
TestInterpreterOptimizer::testMergeNegation9()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 24U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maUnary,   interpreter::unNot,     0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unBool,    0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maUnary,   interpreter::unZap,     0));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(14), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(15), Opcode::maUnary,   interpreter::unIsEmpty, 0));
    TS_ASSERT(isInstruction(s.bco(16), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(17), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(18), Opcode::maUnary,   interpreter::unNot2,    0));
    TS_ASSERT(isInstruction(s.bco(19), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(20), Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(21), Opcode::maSpecial, Opcode::miSpecialPrint, 0));

    TS_ASSERT(isInstruction(s.bco(22), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(23), Opcode::maUnary,   interpreter::unDec,     0));
}

/*
 *  UnaryCondition1 - fuse an unary operation and a conditional popping jump
 */

/** Test folding of uisempty + conditional jump. */
void
TestInterpreterOptimizer::testUnaryCondition1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 10U);

    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack,   Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jIfFalse | Opcode::jPopAlways, isf));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ist));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    // label "ise" got removed
    TS_ASSERT(isInstruction(s.bco(6), Opcode::maUnary,   interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(7), Opcode::maJump,    Opcode::jSymbolic, isf));
    TS_ASSERT(isInstruction(s.bco(8), Opcode::maUnary,   interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(9), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of unot + conditional jump. */
void
TestInterpreterOptimizer::testUnaryCondition2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 11U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, isf));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, ist));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maJump,    Opcode::jSymbolic, isf));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of uzap + conditional jump. */
void
TestInterpreterOptimizer::testUnaryCondition3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 9U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maStack,   Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    // isf got unreferenced, leaving us a uinc/udec combo merged to upos
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unPos, 0));
    TS_ASSERT(isInstruction(s.bco(8), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of unot2 + conditional jump. */
void
TestInterpreterOptimizer::testUnaryCondition4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 10U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maStack,   Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, isf));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, ist));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    // ise is unreferenced
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maJump,    Opcode::jSymbolic, isf));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maJump,    Opcode::jSymbolic, ist));
}

/** Test folding of ubool + conditional jump. */
void
TestInterpreterOptimizer::testUnaryCondition5()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 11U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfEmpty | Opcode::jPopAlways, ise));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfFalse | Opcode::jPopAlways, isf));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maJump,    Opcode::jSymbolic | Opcode::jIfTrue | Opcode::jPopAlways, ist));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump,    Opcode::jSymbolic, ise));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary,   interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maJump,    Opcode::jSymbolic, isf));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary,   interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maJump,    Opcode::jSymbolic, ist));
}

/*
 *  FoldUnary - fold unary operation following a push literal
 */

/** Test folding of push-literal + uzap. */
void
TestInterpreterOptimizer::testFoldUnary1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 7U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPush, Opcode::sBoolean, -1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maPush, Opcode::sInteger, 1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maPush, Opcode::sInteger, 2));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maPush, Opcode::sInteger, -1));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maPush, Opcode::sBoolean, -1));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maPush, Opcode::sBoolean, 1));
    TS_ASSERT(isInstruction(s.bco(6), Opcode::maPush, Opcode::sBoolean, -1));
}

/** Test folding of push-literal + uneg. */
void
TestInterpreterOptimizer::testFoldUnary2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 7U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPush, Opcode::sInteger, 0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maPush, Opcode::sInteger, -1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maPush, Opcode::sInteger, -2));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maPush, Opcode::sInteger, 1));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maPush, Opcode::sInteger, 0));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maPush, Opcode::sInteger, -1));
    TS_ASSERT(isInstruction(s.bco(6), Opcode::maPush, Opcode::sBoolean, -1));
}

/** Test folding of push-literal + different unary operations. */
void
TestInterpreterOptimizer::testFoldUnary3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 15U);

    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maPush, Opcode::sInteger, 10));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maPush, Opcode::sInteger, -10));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maPush, Opcode::sInteger, 10));

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maPush, Opcode::sBoolean, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maPush, Opcode::sBoolean, 0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maPush, Opcode::sBoolean, 1));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maPush, Opcode::sInteger, 10));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maPush, Opcode::sBoolean, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maPush, Opcode::sBoolean, 0));

    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maPush, Opcode::sBoolean, 1));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maPush, Opcode::sInteger, 10));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maPush, Opcode::sInteger, 10));

    TS_ASSERT(isInstruction(s.bco(12), Opcode::maPush, Opcode::sInteger, 11));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maPush, Opcode::sInteger, 9));
    TS_ASSERT(isInstruction(s.bco(14), Opcode::maPush, Opcode::sInteger, -11));
}

/** Test folding of push-literal + different unary operations with boundary cases. */
void
TestInterpreterOptimizer::testFoldUnary4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 6U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPush,  Opcode::sInteger, 32767));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maPush,  Opcode::sInteger, 32766));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maPush,  Opcode::sInteger, -32767));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maPush,  Opcode::sInteger, -32768));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maUnary, interpreter::unDec, 0));
}

/*
 *  FoldBinaryInt - pushint + binary operation -> unary operation
 */

/** Test folding of push-literal + badd. */
void
TestInterpreterOptimizer::testFoldBinaryInt1()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test folding of push-literal + bsub. */
void
TestInterpreterOptimizer::testFoldBinaryInt2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 5U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary,   interpreter::unDec,     0));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maUnary,   interpreter::unInc,     0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4), Opcode::maUnary,   interpreter::unPos,     0));
}

/** Test folding of push-literal + bmul/bdiv/bidiv. */
void
TestInterpreterOptimizer::testFoldBinaryInt3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 11U);
    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary,   interpreter::unNeg,     0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary,   interpreter::unPos,     0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maSpecial, Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary,   interpreter::unNeg,     0));
}

/** Test folding of push-literal + bpow. */
void
TestInterpreterOptimizer::testFoldBinaryInt4()
{
    // ex test_opt.qs:in.FoldBinaryInt4
    Stuff s;

    // pushint 1, bpow -> upos
    s.bco.addInstruction(Opcode::maPush,   Opcode::sInteger, 1);
    s.bco.addInstruction(Opcode::maBinary, interpreter::biPow, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maUnary, interpreter::unPos, 0));
}

/*
 *  FoldJump - Jump on constant condition
 */

/** Test folding of push-literal + conditional jump, with true condition. */
void
TestInterpreterOptimizer::testFoldJump1()
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

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 12U);

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, lend));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump,  Opcode::jSymbolic, lb));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary, interpreter::unDec, 0));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maJump,  Opcode::jSymbolic, lc));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary, interpreter::unNeg, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary, interpreter::unInc, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maJump,  Opcode::jSymbolic, lend));
}

/** Test folding of push-literal + conditional jump, with false condition. */
void
TestInterpreterOptimizer::testFoldJump2()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 12U);

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));

    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic, lb));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary, interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maJump,  Opcode::jSymbolic | Opcode::jAlways, lend));

    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maJump,  Opcode::jSymbolic, lc));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maUnary, interpreter::unNeg, 0));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary, interpreter::unInc, 0));

    TS_ASSERT(isInstruction(s.bco(11), Opcode::maJump,  Opcode::jSymbolic, lend));
}

/** Test folding of push-literal + conditional jump, with empty condition. */
void
TestInterpreterOptimizer::testFoldJump3()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 9U);

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));

    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maJump,  Opcode::jSymbolic, lb));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maUnary, interpreter::unDec, 0));

    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maJump,  Opcode::jSymbolic, lc));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maUnary, interpreter::unNeg, 0));
}

/** Test folding of push-literal + conditional jump, with a non-popping jump. */
void
TestInterpreterOptimizer::testFoldJump4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 12U);

    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maJump,  Opcode::jSymbolic, la));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maUnary, interpreter::unInc, 0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maPush,  Opcode::sBoolean, -1));

    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maJump,  Opcode::jSymbolic, lb));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maUnary, interpreter::unDec, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maPush,  Opcode::sBoolean, -1));

    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maJump,  Opcode::jSymbolic, lc));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maUnary, interpreter::unNeg, 0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maPush,  Opcode::sBoolean, -1));
}

/*
 *  PopPush - pop+push -> store if we're sure the value is preserved
 */

/** Test poploc+pushloc -> storeloc. */
void
TestInterpreterOptimizer::testPopPush1()
{
    // ex test_opt.qs:in.PopPush1
    Stuff s;

    // poploc X, pushloc X -> storeloc X
    uint16_t lv = s.bco.addLocalVariable("A");
    s.bco.addInstruction(Opcode::maPop,  Opcode::sLocal, lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sLocal, lv);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 1U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStore, Opcode::sLocal));
    TS_ASSERT(isLocalVariableName(s.bco, s.bco(0).arg, "A"));
}

/** Test popvar+pushvar; not optimized because it implies a type-cast. */
void
TestInterpreterOptimizer::testPopPush2()
{
    // ex test_opt.qs:in.PopPush2
    Stuff s;

    // pop/push using name is not optimized because it implies a type-cast
    uint16_t lv = s.bco.addName("A");
    s.bco.addInstruction(Opcode::maPop,  Opcode::sNamedVariable, lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, lv);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPop,  Opcode::sNamedVariable));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maPush, Opcode::sNamedVariable));
    TS_ASSERT(isName(s.bco, s.bco(0).arg, "A"));
    TS_ASSERT(isName(s.bco, s.bco(1).arg, "A"));
}

/** Test poploc+pushvar; not optimized because of different scope. */
void
TestInterpreterOptimizer::testPopPush3()
{
    // ex test_opt.qs:in.PopPush3
    Stuff s;

    // pop/push using different scope
    uint16_t lv = s.bco.addLocalVariable("A");
    uint16_t gv = s.bco.addName("A");
    TS_ASSERT_EQUALS(lv, gv);

    s.bco.addInstruction(Opcode::maPop,  Opcode::sLocal,       lv);
    s.bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, gv);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 2U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPop,  Opcode::sLocal,       lv));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maPush, Opcode::sNamedShared, gv));
}

/*
 *  CompareNC - drop the "NC" if we're sure it doesn't make a difference
 */

/** Test caseblind instructions that can be made case-preserving. */
void
TestInterpreterOptimizer::testCompareNC1()
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
    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 14U);
    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maBinary,      interpreter::biCompareEQ, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maBinary,      interpreter::biCompareNE, 0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maBinary,      interpreter::biFirstStr, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maBinary,      interpreter::biRestStr, 0));
    TS_ASSERT(isInstruction(s.bco(11), Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(12), Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(13), Opcode::maBinary,      interpreter::biFindStr, 0));
}

/** Test caseblind instructions that can NOT be made case-preserving. */
void
TestInterpreterOptimizer::testCompareNC2()
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
    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 11U);
    TS_ASSERT(isInstruction(s.bco(0),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(1),  Opcode::maBinary,      interpreter::biCompareGE_NC, 0));
    TS_ASSERT(isInstruction(s.bco(2),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(3),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(4),  Opcode::maBinary,      interpreter::biCompareGT_NC, 0));
    TS_ASSERT(isInstruction(s.bco(5),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(6),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(7),  Opcode::maBinary,      interpreter::biCompareLE_NC, 0));
    TS_ASSERT(isInstruction(s.bco(8),  Opcode::maSpecial,     Opcode::miSpecialPrint, 0));
    TS_ASSERT(isInstruction(s.bco(9),  Opcode::maFusedBinary, Opcode::sLiteral));
    TS_ASSERT(isInstruction(s.bco(10), Opcode::maBinary,      interpreter::biCompareLT_NC, 0));
}

/** Test caseblind instructions that can be made case-preserving with the given operands. */
void
TestInterpreterOptimizer::testCompareNC3()
{
    // ex test_opt.qs:in.CompareNC3
    Stuff s;
    afl::data::StringValue emptySV("");
    afl::data::StringValue dotSV(".");
    afl::data::StringValue bracketSV("[");
    afl::data::StringValue braceSV("}");
    afl::data::FloatValue oneFV(1.0);

    // Compare-inequal with different literals; all are accepted
    s.bco.addInstruction(Opcode::maPush,    Opcode::sInteger, 1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    s.bco.addInstruction(Opcode::maPush,    Opcode::sBoolean, 1);
    s.bco.addInstruction(Opcode::maBinary,  interpreter::biCompareNE_NC, 0);
    s.bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);

    // FIXME: PCC2 also tested the "pushlit 1" case here; BCO's public interface does not allow that.

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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 20U);
    for (size_t i = 0; i < 7; ++i) {
        TS_ASSERT(isInstruction(s.bco(1 + 3*i), Opcode::maBinary, interpreter::biCompareNE, 0));
    }
}

/** Test caseblind instructions that can NOT be made case-preserving with the given operands. */
void
TestInterpreterOptimizer::testCompareNC4()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 11U);
    for (size_t i = 0; i < 3; ++i) {
        TS_ASSERT(isInstruction(s.bco(1 + 3*i), Opcode::maBinary, interpreter::biCompareNE_NC, 0));
    }
}

/*
 *  Optimisation failures
 */

/** Test failure to optimize due to absolute jump. */
void
TestInterpreterOptimizer::testFailAbsolute1()
{
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maJump,  Opcode::jIfEmpty, 0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  Opcode::jIfEmpty,    0));
}

/** Test failure to optimize due to absolute label.
    (An absolute label is a no-op.)*/
void
TestInterpreterOptimizer::testFailAbsolute2()
{
    Stuff s;
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    s.bco.addInstruction(Opcode::maJump,  0,                   0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maStack, Opcode::miStackDrop, 1));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maJump,  0,                   0));
}

/** Test failure to optimize FoldUnary (un-optimizable unary operation). */
void
TestInterpreterOptimizer::testFailFoldUnary()
{
    Stuff s;

    // pushint 1, uatomstr -> not optimized, needs runtime state
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unAtomStr, 0);

    // pushint 1, uinc -> optimized, for comparison
    s.bco.addInstruction(Opcode::maPush,  Opcode::sInteger,       1);
    s.bco.addInstruction(Opcode::maUnary, interpreter::unInc,     0);

    optimize(s.world, s.bco, 2);

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 3U);
    TS_ASSERT(isInstruction(s.bco(0), Opcode::maPush,  Opcode::sInteger, 1));
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maUnary, interpreter::unAtomStr, 0));
    TS_ASSERT(isInstruction(s.bco(2), Opcode::maPush,  Opcode::sInteger, 2));
}

/** Test failure to optimize FoldBinary (un-optimizable operand). */
void
TestInterpreterOptimizer::testFailFoldBinary()
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

    TS_ASSERT_EQUALS(s.bco.getNumInstructions(), 8U);
    TS_ASSERT(isInstruction(s.bco(1), Opcode::maBinary, interpreter::biAdd, 0));
    TS_ASSERT(isInstruction(s.bco(3), Opcode::maBinary, interpreter::biSub, 0));
    TS_ASSERT(isInstruction(s.bco(5), Opcode::maBinary, interpreter::biMult, 0));
    TS_ASSERT(isInstruction(s.bco(7), Opcode::maBinary, interpreter::biPow, 0));
}
