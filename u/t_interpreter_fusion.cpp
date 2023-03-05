/**
  *  \file u/t_interpreter_fusion.cpp
  *  \brief Test for interpreter::Fusion
  */

#include "interpreter/fusion.hpp"

#include "t_interpreter.hpp"
#include "interpreter/bytecodeobject.hpp"

using interpreter::Opcode;
using interpreter::BytecodeObject;

namespace {
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

/** Test fusion push+binary. */
void
TestInterpreterFusion::testFusedBinary()
{
    // pushloc + binary -> fused
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        bco.addInstruction(Opcode::maBinary, interpreter::biAdd, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 2U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedBinary, Opcode::sLocal, 3));
        TS_ASSERT(isInstruction(bco(1), Opcode::maBinary,      interpreter::biAdd, 0));
    }

    // pushvar + binary -> not fused
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, 3);
        bco.addInstruction(Opcode::maBinary, interpreter::biAdd, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 2U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maPush,        Opcode::sNamedVariable, 3));
        TS_ASSERT(isInstruction(bco(1), Opcode::maBinary,      interpreter::biAdd, 0));
    }
}

/** Test fusion push+unary. */
void
TestInterpreterFusion::testFusedUnary()
{
    // pushgvar + unary -> fused
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sNamedShared, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unStr, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 2U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sNamedShared, 7));
        TS_ASSERT(isInstruction(bco(1), Opcode::maUnary,      interpreter::unStr, 0));
    }

    // pushint + unary -> not fused
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sInteger, 9);
        bco.addInstruction(Opcode::maUnary, interpreter::unAtomStr, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 2U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maPush,  Opcode::sInteger, 9));
        TS_ASSERT(isInstruction(bco(1), Opcode::maUnary, interpreter::unAtomStr, 0));
    }
}

/** Test fusion push+unary to in-place operation. */
void
TestInterpreterFusion::testInplaceUnary()
{
    // pushloc + uinc + poploc -> in-place
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 3U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maInplaceUnary, Opcode::sLocal, 7));
        TS_ASSERT(isInstruction(bco(1), Opcode::maUnary, interpreter::unInc, 0));
        TS_ASSERT(isInstruction(bco(2), Opcode::maPop, Opcode::sLocal, 7));
    }

    // pushloc + uinc + pushloc -> fused, not in-place [value re-used]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 3U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
        TS_ASSERT(isInstruction(bco(1), Opcode::maUnary, interpreter::unInc, 0));
        TS_ASSERT(isInstruction(bco(2), Opcode::maPush, Opcode::sLocal, 7));
    }

    // pushloc + uinc -> fused, not in-place [value not provably overwritten]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 2U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
        TS_ASSERT(isInstruction(bco(1), Opcode::maUnary, interpreter::unInc, 0));
    }

    // pushloc + uinc + other + poploc -> in-place [overwritten after other operations]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maUnary, interpreter::unStr, 0);
        bco.addInstruction(Opcode::maBinary, interpreter::biMult, 0);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 5U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maInplaceUnary, Opcode::sLocal, 7));
    }

    // catch + pushloc + uinc + other + poploc -> fused, not in-place [overwritten after other operations, but not exception-safe]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maJump,  Opcode::jCatch, 1);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maUnary, interpreter::unStr, 0);
        bco.addInstruction(Opcode::maBinary, interpreter::biMult, 0);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 6U);
        TS_ASSERT(isInstruction(bco(1), Opcode::maFusedUnary, Opcode::sLocal, 7));
    }

    // catch + pushloc + uinc + poploc -> in-place [immediately overwritten, no exception risk]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maJump,  Opcode::jCatch, 1);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 4U);
        TS_ASSERT(isInstruction(bco(1), Opcode::maInplaceUnary, Opcode::sLocal, 7));
    }

    // pushloc + uinc + j + poploc -> in-place [overwritten in all branches]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue, 5);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maJump,  Opcode::jAlways, 6);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 6U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maInplaceUnary, Opcode::sLocal, 7));
    }

    // pushloc + uinc + j + poploc -> fused, not in-place [not overwritten in all branches]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue, 4);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maJump,  Opcode::jAlways, 6);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 6U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
    }

    // pushloc + uinc + j + poploc -> fused, not in-place [not overwritten in all branches]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue, 4);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maJump,  Opcode::jAlways, 6);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 6U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
    }

    // pushloc + uinc + j + poploc -> fused, not in-place [infinite loop not provable]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maJump,  Opcode::jIfTrue, 2);
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 4U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
    }

    // pushloc + uinc + pushvar + poploc -> fused, not in-place [pushvar not provably disjoint from pushloc]
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush,  Opcode::sLocal, 7);
        bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
        bco.addInstruction(Opcode::maPush,  Opcode::sNamedVariable, 3);
        bco.addInstruction(Opcode::maPop,   Opcode::sLocal, 7);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 4U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedUnary, Opcode::sLocal, 7));
    }
}

/** Test fusion with comparison. */
void
TestInterpreterFusion::testFusedComparison()
{
    // bcmp + jcondp -> fusedcomparison
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maBinary, interpreter::biCompareEQ, 0);
        bco.addInstruction(Opcode::maJump, Opcode::jIfEmpty | Opcode::jPopAlways, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 42);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 3U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedComparison, interpreter::biCompareEQ, 0));
    }

    // bcmp + jcont -> not fused
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maBinary, interpreter::biCompareEQ, 0);
        bco.addInstruction(Opcode::maJump, Opcode::jIfTrue, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 42);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 3U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maBinary, interpreter::biCompareEQ, 0));
    }

    // pushloc + bcmp + jcond -> fusedcomparison2
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 9);
        bco.addInstruction(Opcode::maBinary, interpreter::biCompareEQ, 0);
        bco.addInstruction(Opcode::maJump, Opcode::jIfEmpty | Opcode::jPopAlways, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 42);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 4U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maFusedComparison2, Opcode::sLocal, 9));
        TS_ASSERT(isInstruction(bco(1), Opcode::maFusedComparison, interpreter::biCompareEQ, 0));
    }
}

/** Test miscellaneous. boundary cases. */
void
TestInterpreterFusion::testMisc()
{
    // Empty
    {
        BytecodeObject bco;
        fuseInstructions(bco);
        TS_ASSERT_EQUALS(bco.getNumInstructions(), 0U);
    }

    // One
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        fuseInstructions(bco);
        TS_ASSERT_EQUALS(bco.getNumInstructions(), 1U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maPush, Opcode::sLocal, 3));
    }

    // Fusion at place other than first
    {
        BytecodeObject bco;
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        bco.addInstruction(Opcode::maPush, Opcode::sLocal, 3);
        bco.addInstruction(Opcode::maBinary, interpreter::biAdd, 0);

        fuseInstructions(bco);

        TS_ASSERT_EQUALS(bco.getNumInstructions(), 5U);
        TS_ASSERT(isInstruction(bco(0), Opcode::maPush, Opcode::sLocal, 3));
        TS_ASSERT(isInstruction(bco(1), Opcode::maPush, Opcode::sLocal, 3));
        TS_ASSERT(isInstruction(bco(2), Opcode::maPush, Opcode::sLocal, 3));
        TS_ASSERT(isInstruction(bco(3), Opcode::maFusedBinary, Opcode::sLocal, 3));
        TS_ASSERT(isInstruction(bco(4), Opcode::maBinary,      interpreter::biAdd, 0));
    }
}
