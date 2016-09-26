/**
  *  \file interpreter/optimizer.cpp
  *  \brief Interpreter: Optimizer
  *
  *  FIXME: file taken almost verbatim; needs to be beefed up a little
  */

#include <cassert>
#include "interpreter/optimizer.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/unaryoperation.hpp"
#include "afl/data/value.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/string/char.hpp"
#include "interpreter/fusion.hpp"
#include "interpreter/unaryexecution.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/booleanvalue.hpp"

namespace {
    /** Information about a label. */
    struct IntLabelInfo {
        uint32_t address;       ///< Address of label.
        uint32_t use_count;     ///< Reference count.
        IntLabelInfo()
            : address(0), use_count(0)
            { }
    };

    /** State of optimizer. */
    class IntOptimizerState {
        interpreter::World& world;
        interpreter::BytecodeObject& bco;
        std::vector<IntLabelInfo> label_info;
        bool had_absolute;
        typedef interpreter::BytecodeObject::PC_t PC_t;

        void initLabelInfo();

        bool doStoreDrop(PC_t pc);
        bool doStoreDropMember(PC_t pc);
        bool doMergeDrop(PC_t pc);
        bool doNullOp(PC_t pc);
        bool doEraseUnusedLabels(PC_t pc);
        bool doInvertJumps(PC_t pc);
        bool doThreadJumps(PC_t pc);
        bool doRemoveUnused(PC_t pc);
        bool doMergeNegation(PC_t pc);
        bool doUnaryCondition(PC_t pc);
        bool doFoldUnaryInt(PC_t pc);
        bool doFoldBinaryInt(PC_t pc);
        bool doFoldJump(PC_t pc);
        bool doPopPush(PC_t pc);
        bool doCompareNC(PC_t pc);

        void clearInstruction(PC_t pc);

     public:
        IntOptimizerState(interpreter::World& world, interpreter::BytecodeObject& bco);
        bool iterate();
    };
}

/*************************** IntOptimizerState ***************************/

/** Constructor. */
IntOptimizerState::IntOptimizerState(interpreter::World& world, interpreter::BytecodeObject& bco)
    : world(world), bco(bco), had_absolute(false)
{
    initLabelInfo();
}

/** Initialize label information. Finds all label's addresses and
    reference count. This also checks for absplute jumps. When
    absolute jumps are present, optimisation will not be possible. */
void
IntOptimizerState::initLabelInfo()
{
    label_info.resize(bco.getNumLabels());
    for (interpreter::BytecodeObject::PC_t i = 0; i < bco.getNumInstructions(); ++i) {
        if (bco(i).major == interpreter::Opcode::maJump) {
            if ((bco(i).minor & interpreter::Opcode::jSymbolic) != 0) {
                assert(bco(i).arg < label_info.size());
                if (bco(i).isLabel()) {
                    label_info[bco(i).arg].address = i;
                } else {
                    ++label_info[bco(i).arg].use_count;
                }
            } else {
                had_absolute = true;
            }
        }
    }
}

/** Perform one optimisation iteration.
    \retval true there was a change to the code
    \retval false no possible optimisation found, code unchanged */
bool
IntOptimizerState::iterate()
{
    /* When there are absolute labels, we cannot optimize */
    if (had_absolute)
        return false;

    struct Table {
        bool (IntOptimizerState::*func)(PC_t);           /**< Function to call. */
        interpreter::Opcode::Major major;                 /**< Required major opcode. */
        PC_t                      observed_insns;        /**< Number of observed instructions after pc. */
        const char*               name;
    };
    static const Table table[] = {
        { &IntOptimizerState::doStoreDrop,         interpreter::Opcode::maStore,    1, "StoreDrop" },
        { &IntOptimizerState::doStoreDropMember,   interpreter::Opcode::maMemref,   1, "StoreDropIM" },
        { &IntOptimizerState::doStoreDropMember,   interpreter::Opcode::maIndirect, 1, "StoreDropIM" },
        { &IntOptimizerState::doMergeDrop,         interpreter::Opcode::maStack,    1, "MergeDrop" },
        { &IntOptimizerState::doNullOp,            interpreter::Opcode::maStack,    0, "NullOp" },
        { &IntOptimizerState::doEraseUnusedLabels, interpreter::Opcode::maJump,     0, "EraseUnusedLabels" },
        { &IntOptimizerState::doInvertJumps,       interpreter::Opcode::maJump,     2, "InvertJumps" },
        { &IntOptimizerState::doThreadJumps,       interpreter::Opcode::maJump,     0, "ThreadJumps" },
        { &IntOptimizerState::doRemoveUnused,      interpreter::Opcode::maJump,     1, "RemoveUnused" },
        { &IntOptimizerState::doRemoveUnused,      interpreter::Opcode::maSpecial,  1, "RemoveUnused" },
        { &IntOptimizerState::doMergeNegation,     interpreter::Opcode::maUnary,    1, "MergeNegation" },
        { &IntOptimizerState::doUnaryCondition,    interpreter::Opcode::maUnary,    1, "UnaryCondition" },
        { &IntOptimizerState::doFoldUnaryInt,      interpreter::Opcode::maPush,     1, "FoldUnaryInt" },
        { &IntOptimizerState::doFoldBinaryInt,     interpreter::Opcode::maPush,     1, "FoldBinaryInt" },
        { &IntOptimizerState::doFoldJump,          interpreter::Opcode::maPush,     1, "FoldJump" },
        { &IntOptimizerState::doPopPush,           interpreter::Opcode::maPop,      1, "PopPush" },
        { &IntOptimizerState::doCompareNC,         interpreter::Opcode::maPush,     1, "CompareNC" },
    };

    bool did = false;
    // printf("iterate():\n");
    for (PC_t i = 0; i < bco.getNumInstructions(); ++i) {
        for (uint32_t ti = 0; ti < sizeof(table)/sizeof(table[0]); ++ti) {
            if (bco(i).major == table[ti].major && bco.getNumInstructions() - i > table[ti].observed_insns) {
                if ((this->*table[ti].func)(i)) {
                    // printf("  %s applied at %d\n", table[ti].name, int(i));
                    did = true;
                }
            }
        }
    }

    return did;
}

/** Make instruction at specified pc blank. We use absolute labels as
    null operations. Those will be removed by interpreter::BytecodeObject::compact(). */
void
IntOptimizerState::clearInstruction(PC_t pc)
{
    if (bco(pc).isJumpOrCatch()) {
        /* Jump or catch, so decrement label's reference count */
        --label_info[bco(pc).arg].use_count;
    }
    bco(pc).major = interpreter::Opcode::maJump;
    bco(pc).minor = interpreter::Opcode::jLabel;
    bco(pc).arg   = 0;
}

/************************ Individual Optimisations ***********************/

/** StoreDrop optimisation. Combine STOREx + DROP into POPx. 
    Implemented by decreasing the DROP's counter; if it reaches zero,
    it is removed by doNullOp.

    This pattern appears in assignments. */
bool
IntOptimizerState::doStoreDrop(PC_t pc)
{
    if (bco(pc+1).is(interpreter::Opcode::miStackDrop) && bco(pc+1).arg > 0) {
        bco(pc).major = interpreter::Opcode::maPop;
        --bco(pc+1).arg;
        return true;
    } else {
        return false;
    }
}

/** StoreDrop optimisation for member references. Combine STOREx + DROP into POPx. 
    Implemented by decreasing the DROP's counter; if it reaches zero,
    it is removed by doNullOp.

    This pattern appears in assignments. */
bool
IntOptimizerState::doStoreDropMember(PC_t pc)
{
    if ((bco(pc).minor & interpreter::Opcode::miIMOpMask) == interpreter::Opcode::miIMStore
        && bco(pc+1).is(interpreter::Opcode::miStackDrop)
        && bco(pc+1).arg > 0)
    {
        bco(pc).minor -= interpreter::Opcode::miIMStore;
        bco(pc).minor += interpreter::Opcode::miIMPop;
        --bco(pc+1).arg;
        return true;
    } else {
        return false;
    }
}

/** MergeDrop optimisation.
    Combine two DROP into one.

    This pattern appears in "For ... Select Case ... Break", which
    generates one DROP for the selector, and one for the loop
    bound. */
bool
IntOptimizerState::doMergeDrop(PC_t pc)
{
    if (bco(pc).is(interpreter::Opcode::miStackDrop) && bco(pc+1).is(interpreter::Opcode::miStackDrop)) {
        bco(pc+1).arg += bco(pc).arg;
        clearInstruction(pc);
        return true;
    } else {
        return false;
    }
}

/** Remove null operations. Those are DROP 0 and SWAP 0.
    DROP 0 appears after doStoreDrop(). */
bool
IntOptimizerState::doNullOp(PC_t pc)
{
    if ((bco(pc).is(interpreter::Opcode::miStackDrop) || bco(pc).is(interpreter::Opcode::miStackSwap)) && bco(pc).arg == 0) {
        clearInstruction(pc);
        return true;
    } else {
        return false;
    }
}

/** Erase unused labels. Those appear frequently. */
bool
IntOptimizerState::doEraseUnusedLabels(PC_t pc)
{
    /* Remove unreferenced labels */
    if (bco(pc).isLabel() && (bco(pc).minor & interpreter::Opcode::jSymbolic) != 0 && label_info[bco(pc).arg].use_count == 0) {
        clearInstruction(pc);
        return true;
    } else {
        return false;
    }
}

/** Invert jumps. Converts a conditional jump across another jump into
    a single conditional jump.

    This pattern appears frequently in conditions. */
bool
IntOptimizerState::doInvertJumps(PC_t pc)
{
    if (bco(pc).isRegularJump()
        && label_info[bco(pc).arg].address == pc+2
        && bco(pc+1).isRegularJump()
        && (bco(pc+1).minor & interpreter::Opcode::jPopAlways) == 0)
    {
        int nextMinor = (bco(pc+1).minor & ~(bco(pc).minor & interpreter::Opcode::jAlways)) | (bco(pc).minor & interpreter::Opcode::jPopAlways);
        if ((nextMinor & interpreter::Opcode::jAlways) == 0) {
            // Second jump is never taken. Eliminate both.
            clearInstruction(pc);
            clearInstruction(pc+1);
            if (nextMinor & interpreter::Opcode::jPopAlways) {                
                bco(pc+1).major = interpreter::Opcode::maStack;
                bco(pc+1).minor = interpreter::Opcode::miStackDrop;
                bco(pc+1).arg = 1;
            }
        } else {
            // Second jump is taken sometimes. Eliminate first jump.
            clearInstruction(pc);
            bco(pc+1).minor = nextMinor;
        }
        return true;
    } else {
        return false;
    }
}

/** Thread jumps. When a jump targets a label or another jump, adjust
    its target to minimize work at runtime.
    - skip over labels, so that if several jumps target these labels, all jump to the last label,
      and the others are unreferenced and can be removed
    - follow unconditional jumps and directly jump to their targets
    - follow at most one backward jump, to avoid chasing infinite loops

    This pattern appears frequently. */
bool
IntOptimizerState::doThreadJumps(PC_t pc)
{
    /* Is it actually a jump? */
    if (!bco(pc).isRegularJump())
        return false;

    bool had_backward_jump = false;
    uint16_t target_label = bco(pc).arg;
    while (1) {
        PC_t target_address = label_info[target_label].address;
        assert(bco(target_address).isLabel());
        if (target_address+1 >= bco.getNumInstructions()) {
            /* Jump to label at end of routine */
            break;
        } else if (bco(target_address+1).isLabel() && (bco(target_address+1).minor & interpreter::Opcode::jSymbolic) != 0) {
            /* Jump to a label, which is followed by a label. Jump to the later label,
               to make the former unreferenced and removable. */
            target_label = bco(target_address+1).arg;
            assert(label_info[target_label].address == target_address+1);
        } else if (bco(target_address+1).isRegularJump()
                   && (bco(target_address+1).minor == (interpreter::Opcode::jAlways | interpreter::Opcode::jSymbolic)
                       || ((bco(pc).minor & interpreter::Opcode::jPopAlways) == 0
                           && (bco(target_address+1).minor & interpreter::Opcode::jPopAlways) == 0
                           && (bco(pc).minor & ~bco(target_address+1).minor) == 0)))
        {
            /* Jump to an unconditional jump (frequent in if-within-loops),
               or to a conditional jump with the same condition (in a:=b xor c). */
            PC_t arg = bco(target_address+1).arg;
            if (label_info[arg].address <= target_address) {
                /* At most one backward jump per iteration to avoid infinite loops */
                if (had_backward_jump)
                    break;
                had_backward_jump = true;
            }
            target_label = arg;
        } else {
            /* Not a jump or label */
            break;
        }
    }

    /* Did we change anything? */
    if (label_info[target_label].address == pc+1) {
        /* It is a jump to the next instruction. Delete it. */
        if (bco(pc).minor & interpreter::Opcode::jPopAlways) {
            clearInstruction(pc);
            bco(pc).major = interpreter::Opcode::maStack;
            bco(pc).minor = interpreter::Opcode::miStackDrop;
            bco(pc).arg = 1;
        } else {
            clearInstruction(pc);
        }
        return true;
    } else if (target_label != bco(pc).arg) {
        /* We followed a jump chain. */
        --label_info[bco(pc).arg].use_count;
        bco(pc).arg = target_label;
        ++label_info[bco(pc).arg].use_count;
        return true;
    } else {
        return false;
    }
}

/** Remove unused code. Removes code following an unconditional jump,
    a UTHROW, or SRETURN instruction. Such code is never executed.

    This appears in code such as "If a Then Return Else ..." */
bool
IntOptimizerState::doRemoveUnused(PC_t pc)
{
    if ((bco(pc).isRegularJump() && (bco(pc).minor & interpreter::Opcode::jAlways) == interpreter::Opcode::jAlways)
        || (bco(pc).is(interpreter::Opcode::miSpecialThrow))
        || (bco(pc).is(interpreter::Opcode::miSpecialTerminate))
        || (bco(pc).is(interpreter::Opcode::miSpecialReturn)))
    {
        // Jump, throw or return
        PC_t i = pc+1;
        while (i < bco.getNumInstructions() && !bco(i).isLabel()) {
            clearInstruction(i);
            ++i;
        }
        return i > pc+1;
    } else {
        return false;
    }
}

/** MergeNegation optimisation. Merges two unary logic/sign instructions
    into one if possible.

    This appears in "If IsEmpty(Zap(x))" (UZAP + UISEMPTY => UNOT2),
    or in "For i:=-1 To +1" (UNEG from boundary + UPOS used as type-check),
    and in explicitly written code ("Zap(Zap(x))"). */
bool
IntOptimizerState::doMergeNegation(PC_t pc)
{
    /* Merge pairs of unary logic into single instructions */
    enum { None = -1, RepFalse = -2, ZapBool = -3 };
    int result = None;

    if (!bco(pc+1).is(interpreter::Opcode::maUnary))
        return false;

    switch (bco(pc).minor) {
     case interpreter::unNot:
        switch (bco(pc+1).minor) {
         case interpreter::unNot:     result = interpreter::unBool;    break;
         case interpreter::unBool:    result = interpreter::unNot;     break;
         case interpreter::unIsEmpty: result = interpreter::unIsEmpty; break;
        }
        break;
     case interpreter::unBool:
        switch (bco(pc+1).minor) {
         case interpreter::unNot:     result = interpreter::unNot;     break;
         case interpreter::unBool:    result = interpreter::unBool;    break;
         case interpreter::unNot2:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = interpreter::unIsEmpty; break;
         case interpreter::unZap:     result = ZapBool;   break;
        }
        break;
     case interpreter::unNot2:
        switch (bco(pc+1).minor) {
         case interpreter::unBool:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = RepFalse;  break;
        }
        break;
     case interpreter::unIsEmpty:
        switch (bco(pc+1).minor) {
         case interpreter::unBool:    result = interpreter::unIsEmpty; break;
         case interpreter::unIsEmpty: result = RepFalse;  break;
        }
        break;
     case interpreter::unZap:
        switch (bco(pc+1).minor) {
         case interpreter::unNot2:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = interpreter::unNot2;    break;
         case interpreter::unZap:     result = interpreter::unZap;     break;
        }
        break;
     case interpreter::unNeg:
        switch (bco(pc+1).minor) {
         case interpreter::unNeg:     result = interpreter::unPos;     break;
         case interpreter::unPos:     result = interpreter::unNeg;     break;
        }
        break;
     case interpreter::unPos:
        switch (bco(pc+1).minor) {
         case interpreter::unNeg:     result = interpreter::unNeg;     break;
         case interpreter::unPos:     result = interpreter::unPos;     break;
         case interpreter::unInc:     result = interpreter::unInc;     break;
         case interpreter::unDec:     result = interpreter::unDec;     break;
        }
        break;
     case interpreter::unInc:
        switch (bco(pc+1).minor) {
         case interpreter::unDec:     result = interpreter::unPos;     break;
         case interpreter::unPos:     result = interpreter::unInc;     break;
        }
        break;
     case interpreter::unDec:
        switch (bco(pc+1).minor) {
         case interpreter::unInc:     result = interpreter::unPos;     break;
         case interpreter::unPos:     result = interpreter::unDec;     break;
        }
        break;
    }

    if (result == None) {
        /* Cannot reduce this one */
        return false;
    } else if (result == RepFalse) {
        /* Replace by 'drop 1; pushbool 0' */
        bco(pc).major   = interpreter::Opcode::maStack;
        bco(pc).minor   = interpreter::Opcode::miStackDrop;
        bco(pc).arg     = 1;
        bco(pc+1).major = interpreter::Opcode::maPush;
        bco(pc+1).minor = interpreter::Opcode::sBoolean;
        bco(pc+1).arg   = 0;
        return true;
    } else if (result == ZapBool) {
        /* Replace by 'uzap; ubool' (potentially fewer temporaries) */
        bco(pc).minor = interpreter::unZap;
        bco(pc+1).minor = interpreter::unBool;
        return true;
    } else {
        /* Replace by 'u<whatever>' */
        clearInstruction(pc);
        bco(pc+1).minor = result;
        return true;
    }
}

/** UnaryCondition optimisation. If a logic instruction is followed by
    a conditional jump, modify the jump's condition to evaluate the
    logic, e.g. merge UNOT+JTP intp JFP.

    This appears often in conditions such as "If IsEmpty(x)". */
bool
IntOptimizerState::doUnaryCondition(PC_t pc)
{
    /* u<logic> / j<cc>p -> j<cc'>p */
    if (bco(pc+1).isRegularJump() && (bco(pc+1).minor & interpreter::Opcode::jPopAlways) != 0) {
        int oldCond = bco(pc+1).minor;
        int newCond = 0;
        if (bco(pc).is(interpreter::unIsEmpty)) {
            /* uisempty: t->e, f->tf, e->never */
            if (oldCond & interpreter::Opcode::jIfTrue)
                newCond |= interpreter::Opcode::jIfEmpty;
            if (oldCond & interpreter::Opcode::jIfFalse)
                newCond |= interpreter::Opcode::jIfTrue | interpreter::Opcode::jIfFalse;
        } else if (bco(pc).is(interpreter::unNot)) {
            /* unot: t->f, f->t, e->e */
            if (oldCond & interpreter::Opcode::jIfTrue)
                newCond |= interpreter::Opcode::jIfFalse;
            if (oldCond & interpreter::Opcode::jIfFalse)
                newCond |= interpreter::Opcode::jIfTrue;
            if (oldCond & interpreter::Opcode::jIfEmpty)
                newCond |= interpreter::Opcode::jIfEmpty;
        } else if (bco(pc).is(interpreter::unZap)) {
            /* uzap: t->t, f->never, e->fe */
            if (oldCond & interpreter::Opcode::jIfTrue)
                newCond |= interpreter::Opcode::jIfTrue;
            if (oldCond & interpreter::Opcode::jIfEmpty)
                newCond |= interpreter::Opcode::jIfEmpty | interpreter::Opcode::jIfFalse;
        } else if (bco(pc).is(interpreter::unNot2)) {
            /* unot2: t->fe, f->t, e->never */
            if (oldCond & interpreter::Opcode::jIfTrue)
                newCond |= interpreter::Opcode::jIfFalse | interpreter::Opcode::jIfEmpty;
            if (oldCond & interpreter::Opcode::jIfFalse)
                newCond |= interpreter::Opcode::jIfTrue;
        } else if (bco(pc).is(interpreter::unBool)) {
            /* ubool / jccp --> jccp */
            newCond = oldCond;
        } else {
            return false;
        }

        /* Update it */
        clearInstruction(pc);
        if (newCond == 0) {
            /* This will become a "jump never" */
            clearInstruction(pc+1);
            bco(pc+1).major = interpreter::Opcode::maStack;
            bco(pc+1).minor = interpreter::Opcode::miStackDrop;
            bco(pc+1).arg   = 1;
        } else {
            /* Update jump */
            bco(pc+1).minor = newCond | interpreter::Opcode::jPopAlways | interpreter::Opcode::jSymbolic;
        }
        return true;
    } else {
        return false;
    }
}

/** Constant-fold unary operations on integer constants. The most
    frequent occurrences are negative literals (encoded as "pushint,
    uneg"), "For i:=1 to 10" type checks ("pushint 1, upos"), and
    "Z(0)" ("pushint 0, uzap"). The others are included for
    completeness. */
bool
IntOptimizerState::doFoldUnaryInt(PC_t pc)
{
    if ((bco(pc).minor != interpreter::Opcode::sInteger && bco(pc).minor != interpreter::Opcode::sBoolean) || !bco(pc+1).is(interpreter::Opcode::maUnary))
        return false;

    switch (bco(pc+1).minor) {
     case interpreter::unZap:
     case interpreter::unNeg:
     case interpreter::unPos:
     case interpreter::unNot:
     case interpreter::unNot2:
     case interpreter::unBool:
     case interpreter::unAbs:
     case interpreter::unIsEmpty:
     case interpreter::unIsString:
     case interpreter::unIsNum:
     case interpreter::unTrunc:
     case interpreter::unRound:
     case interpreter::unInc:
     case interpreter::unDec:
     case interpreter::unBitNot:
        try {
            afl::data::Value* result;
            if (bco(pc).minor == interpreter::Opcode::sInteger) {
                afl::data::IntegerValue iv(+int16_t(bco(pc).arg));
                result = interpreter::executeUnaryOperation(world, bco(pc+1).minor, &iv);
            } else if (int16_t(bco(pc).arg) < 0) {
                result = executeUnaryOperation(world, bco(pc+1).minor, 0);
            } else {
                afl::data::BooleanValue iv(+int16_t(bco(pc).arg));
                result = executeUnaryOperation(world, bco(pc+1).minor, &iv);
            }

            /* Can we encode the result? */
            bool did;
            if (result == 0) {
                bco(pc).minor = interpreter::Opcode::sBoolean;
                bco(pc).arg   = uint16_t(-1U);
                did = true;
            } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(result)) {
                if (dynamic_cast<afl::data::BooleanValue*>(iv) != 0) {
                    bco(pc).minor = interpreter::Opcode::sBoolean;
                    bco(pc).arg   = (iv->getValue() != 0);
                    did = true;
                } else if (iv->getValue() >= -32767 && iv->getValue() <= +32767) {
                    bco(pc).minor = interpreter::Opcode::sInteger;
                    bco(pc).arg   = iv->getValue();
                    did = true;
                } else {
                    did = false;
                }
            } else {
                did = false;
            }
            delete result;
            if (did)
                clearInstruction(pc+1);
            return did;
        }
        catch (std::exception& e) {
            // Execution failed; keep instruction unchanged.
            // This does not normally happen as the operations we're trying are failsafe.
            return false;
        }
     default:
        return false;
    }
}

/** Convert binary operations using a single constant integer
    parameter into unary operations.

    The "+1"/"-1" case appears frequently in user code, the others are
    included for completeness. */
bool
IntOptimizerState::doFoldBinaryInt(PC_t pc)
{
    if (bco(pc).minor != interpreter::Opcode::sInteger || !bco(pc+1).is(interpreter::Opcode::maBinary))
        return false;

    int16_t value = bco(pc).arg;
    switch (bco(pc+1).minor) {
     case interpreter::biAdd:
        /* "+ 1" => uinc, "+ -1" => udec, "+0" => upos */
        if (value == 0 || value == 1 || value == -1) {
            bco(pc+1).major = interpreter::Opcode::maUnary;
            bco(pc+1).minor = (value == 0 ? interpreter::unPos : value == 1 ? interpreter::unInc : interpreter::unDec);
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     case interpreter::biSub:
        /* "- +1" => uinc, "- -1" => uinc, "+0" => upos */
        if (value == 0 || value == 1 || value == -1) {
            bco(pc+1).major = interpreter::Opcode::maUnary;
            bco(pc+1).minor = (value == 0 ? interpreter::unPos : value == 1 ? interpreter::unDec : interpreter::unInc);
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     case interpreter::biMult:
     case interpreter::biDivide:
     case interpreter::biIntegerDivide:
        /* "* +1" => upos, "* -1" => uneg */
        if (value == 1 || value == -1) {
            bco(pc+1).major = interpreter::Opcode::maUnary;
            bco(pc+1).minor = (value == 1 ? interpreter::unPos : interpreter::unNeg);
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     case interpreter::biPow:
        /* "^ +1" => upos */
        if (value == 1) {
            bco(pc+1).major = interpreter::Opcode::maUnary;
            bco(pc+1).minor = interpreter::unPos;
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     default:
        return false;
    }
}

/** Fold conditional jump on constant. If a literal is immediately
    followed by a conditional jump, or by a jump to a conditional
    jump, evaluate that condition and generate an unconditional jump
    instead.

    This appears in 'Do While True' or 'FindShip(1)'. */
bool
IntOptimizerState::doFoldJump(PC_t pc)
{
    if (bco(pc).minor != interpreter::Opcode::sInteger && bco(pc).minor != interpreter::Opcode::sBoolean)
        return false;
    if (!bco(pc+1).isRegularJump())
        return false;

    /* Figure out condition */
    int cond;
    if (bco(pc).minor == interpreter::Opcode::sBoolean && int16_t(bco(pc).arg) < 0)
        cond = interpreter::Opcode::jIfEmpty;
    else if (bco(pc).arg == 0)
        cond = interpreter::Opcode::jIfFalse;
    else
        cond = interpreter::Opcode::jIfTrue;

    /* Check jump */
    if ((bco(pc+1).minor & interpreter::Opcode::jAlways) == interpreter::Opcode::jAlways) {
        /* Unconditional jump. Check whether it jumps at a conditional jump. */
        if (bco(pc+1).minor & interpreter::Opcode::jPopAlways) {
            /* Pathological case: push + jp, turn into j */
            bco(pc+1).minor &= ~interpreter::Opcode::jPopAlways;
            clearInstruction(pc);
            return true;
        }

        /* Follow this jump */
        uint16_t target_label = bco(pc+1).arg;
        PC_t target_address = label_info[target_label].address;
        if (target_address+1 >= bco.getNumInstructions() || !bco(target_address+1).isRegularJump()) {
            /* Jump to end of routine or not to a jump; keep it */
            return false;
        }

        if (bco(target_address+1).minor & cond) {
            /* Jump to conditional jump that will be taken; follow it */
            if (bco(target_address+1).minor & interpreter::Opcode::jPopAlways) {
                clearInstruction(pc);
            }
            --label_info[bco(pc+1).arg].use_count;
            bco(pc+1).arg = bco(target_address+1).arg;
            ++label_info[bco(pc+1).arg].use_count;
            return true;
        } else {
            /* Conditional jump is not taken; we cannot optimize this */
            return false;
        }
    } else {
        /* Conditional jump */
        if (bco(pc+1).minor & interpreter::Opcode::jPopAlways) {
            /* This jump will pop, drop the push */
            clearInstruction(pc);
        }

        if (bco(pc+1).minor & cond) {
            /* This jump will be taken. Make it unconditional. */
            bco(pc+1).minor |= interpreter::Opcode::jAlways;
            bco(pc+1).minor &= ~interpreter::Opcode::jPopAlways;
        } else {
            /* This jump will not be taken. Drop it. */
            clearInstruction(pc+1);
        }
    }

    return true;
}

/** Fold pop followed by push into store. This appears when a variable is
    set and then immediately used, as in 'arg := expr; foo(arg)'. */
bool
IntOptimizerState::doPopPush(PC_t pc)
{
    /* Next instruction must be push with same address, but may not be
       a named variable: assigning those implies a type-cast. */
    if (bco(pc+1).major == interpreter::Opcode::maPush
        && bco(pc).minor == bco(pc+1).minor
        && bco(pc).arg == bco(pc+1).arg
        && bco(pc).minor != interpreter::Opcode::sNamedVariable)
    {
        /* do it */
        bco(pc+1).major = interpreter::Opcode::maStore;
        clearInstruction(pc);
        return true;
    } else {
        /* not applicable */
        return false;
    }
}

/** Convert push followed by case-blind comparison into regular
    comparisons if it can be proven that case-blindness is not needed. 
    This removes the need for upcasing in the comparison. The same
    applies for First/Rest/FindStr. Note that only equality
    comparisons can use this optimisation; ordering comparisons depend
    on upcasing (i.e. "a" < "[" in case-blind mode, "a" > "[" in
    case-sensitive mode). */
bool
IntOptimizerState::doCompareNC(PC_t pc)
{
    const uint8_t pm = bco(pc).minor;
    const uint8_t cm = bco(pc+1).minor;

    /* First must be push literal */
    if (pm != interpreter::Opcode::sInteger && pm != interpreter::Opcode::sBoolean && pm != interpreter::Opcode::sLiteral)
        return false;

    /* Second must be case-blind compare */
    if (bco(pc+1).major != interpreter::Opcode::maBinary)
        return false;
    if (cm != interpreter::biCompareEQ_NC && cm != interpreter::biCompareNE_NC && cm != interpreter::biFirstStr_NC && cm != interpreter::biRestStr_NC && cm != interpreter::biFindStr_NC)
        return false;

    /* If first is pushlit, it must be an acceptable literal */
    if (pm == interpreter::Opcode::sLiteral) {
        afl::data::Value* iv = bco.getLiteral(bco(pc).arg);
        if (dynamic_cast<afl::data::ScalarValue*>(iv) != 0 || dynamic_cast<afl::data::FloatValue*>(iv) != 0) {
            /* accept */ ;
        } else if (afl::data::StringValue* sv = dynamic_cast<afl::data::StringValue*>(iv)) {
            /* accept if valid */
            String_t v = sv->getValue();
            for (String_t::size_type i = 0; i < v.size(); ++i)
                if (afl::string::charIsAlphanumeric(v[i]))
                    return false;
        } else {
            /* reject */
            return false;
        }
    }

    /* OK */
    --bco(pc+1).minor;
    return true;
}

/** Optimize the given bytecode object. It must not have been relocated yet.
    \param bco [in/out] Bytecode object
    \param level [in] Optimisation level to apply */
void
interpreter::optimize(World& world, BytecodeObject& bco, int /*level*/)
{
    unfuseInstructions(bco);
    while (IntOptimizerState(world, bco).iterate()) {
        bco.compact();
    }
    fuseInstructions(bco);
}
