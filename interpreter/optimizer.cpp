/**
  *  \file interpreter/optimizer.cpp
  *  \brief Interpreter: Optimizer
  */

#include <cassert>
#include <memory>
#include "interpreter/optimizer.hpp"
#include "afl/data/booleanvalue.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/integervalue.hpp"
#include "afl/data/scalarvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/data/value.hpp"
#include "afl/string/char.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/fusion.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/unaryexecution.hpp"
#include "interpreter/unaryoperation.hpp"

using interpreter::Opcode;
using interpreter::BytecodeObject;

namespace {
    /** State of optimizer. */
    class OptimizerState {
     public:
        typedef BytecodeObject::PC_t PC_t;

        OptimizerState(interpreter::World& world, BytecodeObject& bco, int level);

        bool iterate();

     private:
        struct LabelInfo {
            PC_t address;         ///< Address of label.
            uint32_t useCount;    ///< Reference count.
            LabelInfo()
                : address(0), useCount(0)
                { }
        };

        interpreter::World& m_world;
        BytecodeObject& m_bco;
        std::vector<LabelInfo> m_labelInfo;
        bool m_hadBogusJumps;
        int m_optimisationLevel;

        void initLabelInfo();
        void clearInstruction(PC_t pc);

        bool doStoreDrop(PC_t pc);
        bool doStoreDropMember(PC_t pc);
        bool doMergeDrop(PC_t pc);
        bool doNullOp(PC_t pc);
        bool doEraseUnusedLabels(PC_t pc);
        bool doInvertJumps(PC_t pc);
        bool doThreadJumps(PC_t pc);
        bool doMergeJumps(PC_t pc);
        bool doRemoveUnused(PC_t pc);
        bool doMergeNegation(PC_t pc);
        bool doUnaryCondition(PC_t pc);
        bool doFoldUnaryInt(PC_t pc);
        bool doFoldBinaryInt(PC_t pc);
        bool doFoldBinaryTypeCheck(PC_t pc);
        bool doFoldJump(PC_t pc);
        bool doPopPush(PC_t pc);
        bool doCompareNC(PC_t pc);
        bool doIntCompare(PC_t pc);
        bool doTailMerge(PC_t pc);
        bool doDeadStore(PC_t pc);

        bool removeDeadStores(PC_t pc);
    };
}

/*************************** OptimizerState ***************************/

/** Constructor. */
OptimizerState::OptimizerState(interpreter::World& world, BytecodeObject& bco, int level)
    : m_world(world), m_bco(bco), m_hadBogusJumps(false), m_optimisationLevel(level)
{
    initLabelInfo();
}

/** Perform one optimisation iteration.
    \retval true there was a change to the code
    \retval false no possible optimisation found, code unchanged */
bool
OptimizerState::iterate()
{
    /* When there are absolute labels, we cannot optimize */
    if (m_hadBogusJumps) {
        return false;
    }

    struct Table {
        bool (OptimizerState::*func)(PC_t);           /**< Function to call. */
        Opcode::Major major;                             /**< Required major opcode. */
        PC_t                      observed_insns;        /**< Number of observed instructions after pc. */
        const char*               name;
    };
    static const Table table[] = {
        { &OptimizerState::doStoreDrop,         Opcode::maStore,    1, "StoreDrop" },
        { &OptimizerState::doStoreDropMember,   Opcode::maMemref,   1, "StoreDropIM" },
        { &OptimizerState::doStoreDropMember,   Opcode::maIndirect, 1, "StoreDropIM" },
        { &OptimizerState::doMergeDrop,         Opcode::maStack,    1, "MergeDrop" },
        { &OptimizerState::doNullOp,            Opcode::maStack,    0, "NullOp" },
        { &OptimizerState::doEraseUnusedLabels, Opcode::maJump,     0, "EraseUnusedLabels" },
        { &OptimizerState::doInvertJumps,       Opcode::maJump,     2, "InvertJumps" },
        { &OptimizerState::doThreadJumps,       Opcode::maJump,     0, "ThreadJumps" },
        { &OptimizerState::doMergeJumps,        Opcode::maJump,     1, "MergeJumps" },
        { &OptimizerState::doRemoveUnused,      Opcode::maJump,     1, "RemoveUnused" },
        { &OptimizerState::doRemoveUnused,      Opcode::maSpecial,  1, "RemoveUnused" },
        { &OptimizerState::doMergeNegation,     Opcode::maUnary,    1, "MergeNegation" },
        { &OptimizerState::doUnaryCondition,    Opcode::maUnary,    1, "UnaryCondition" },
        { &OptimizerState::doFoldUnaryInt,      Opcode::maPush,     1, "FoldUnaryInt" },
        { &OptimizerState::doFoldBinaryInt,     Opcode::maPush,     1, "FoldBinaryInt" },
        { &OptimizerState::doFoldBinaryTypeCheck, Opcode::maBinary, 1, "FoldBinaryTypeCheck" },
        { &OptimizerState::doFoldJump,          Opcode::maPush,     1, "FoldJump" },
        { &OptimizerState::doPopPush,           Opcode::maPop,      1, "PopPush" },
        { &OptimizerState::doCompareNC,         Opcode::maPush,     1, "CompareNC" },
        { &OptimizerState::doIntCompare,        Opcode::maBinary,   3, "IntCompare" },
        { &OptimizerState::doTailMerge,         Opcode::maJump,     0, "TailMerge" },
        { &OptimizerState::doDeadStore,         Opcode::maSpecial,  0, "DeadStore" },
    };

    bool did = false;
    // printf("iterate():\n");
    for (PC_t i = 0; i < m_bco.getNumInstructions(); ++i) {
        for (uint32_t ti = 0; ti < sizeof(table)/sizeof(table[0]); ++ti) {
            if (m_bco(i).major == table[ti].major && m_bco.getNumInstructions() - i > table[ti].observed_insns) {
                if ((this->*table[ti].func)(i)) {
                    // printf("  %s applied at %d\n", table[ti].name, int(i));
                    did = true;
                }
            }
        }
    }

    // Remove dead stores starting at end of function
    if (removeDeadStores(m_bco.getNumInstructions())) {
        did = true;
    }

    return did;
}

/** Initialize label information. Finds all label's addresses and
    reference count. This also checks for absplute jumps. When
    absolute jumps are present, optimisation will not be possible. */
void
OptimizerState::initLabelInfo()
{
    m_labelInfo.resize(m_bco.getNumLabels());
    for (BytecodeObject::PC_t i = 0; i < m_bco.getNumInstructions(); ++i) {
        if (m_bco(i).major == Opcode::maJump) {
            if ((m_bco(i).minor & Opcode::jSymbolic) != 0) {
                uint16_t index = m_bco(i).arg;
                if (index < m_labelInfo.size()) {
                    // Correct symbolic address and correct metadata
                    if (m_bco(i).isLabel()) {
                        m_labelInfo[index].address = i;
                    } else {
                        ++m_labelInfo[index].useCount;
                    }
                } else {
                    // Symbolic address range not correctly reported by producer of BCO; refuse to optimize
                    m_hadBogusJumps = true;
                }
            } else {
                // Absolute address
                m_hadBogusJumps = true;
            }
        }
    }
}


/** Make instruction at specified pc blank. We use absolute labels as
    null operations. Those will be removed by BytecodeObject::compact(). */
void
OptimizerState::clearInstruction(PC_t pc)
{
    if (m_bco(pc).isJumpOrCatch()) {
        /* Jump or catch, so decrement label's reference count */
        --m_labelInfo[m_bco(pc).arg].useCount;
    }
    m_bco(pc).major = Opcode::maJump;
    m_bco(pc).minor = Opcode::jLabel;
    m_bco(pc).arg   = 0;
}

/************************ Individual Optimisations ***********************/

/** StoreDrop optimisation. Combine STOREx + DROP into POPx.
    Implemented by decreasing the DROP's counter; if it reaches zero,
    it is removed by doNullOp.

    This pattern appears in assignments. */
bool
OptimizerState::doStoreDrop(PC_t pc)
{
    if (m_bco(pc+1).is(Opcode::miStackDrop) && m_bco(pc+1).arg > 0) {
        m_bco(pc).major = Opcode::maPop;
        --m_bco(pc+1).arg;
        return true;
    } else {
        return false;
    }
}

/** StoreDrop optimisation for member references. Combine STOREx + DROP into POPx.
    The same optimisation can be done for LOADx + DROP -> CALLx.
    Implemented by decreasing the DROP's counter; if it reaches zero,
    it is removed by doNullOp.

    This pattern appears in assignments. */
bool
OptimizerState::doStoreDropMember(PC_t pc)
{
    int minor = (m_bco(pc).minor & Opcode::miIMOpMask);
    if ((minor == Opcode::miIMStore || minor == Opcode::miIMLoad)
        && m_bco(pc+1).is(Opcode::miStackDrop)
        && m_bco(pc+1).arg > 0)
    {
        if (minor == Opcode::miIMStore) {
            m_bco(pc).minor = uint8_t(m_bco(pc).minor - Opcode::miIMStore + Opcode::miIMPop);
        } else {
            m_bco(pc).minor = uint8_t(m_bco(pc).minor - Opcode::miIMLoad + Opcode::miIMCall);
        }
        --m_bco(pc+1).arg;
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
OptimizerState::doMergeDrop(PC_t pc)
{
    if (m_bco(pc).is(Opcode::miStackDrop) && m_bco(pc+1).is(Opcode::miStackDrop)) {
        // Avoid overflow!
        uint32_t total = uint32_t(m_bco(pc+1).arg) + m_bco(pc).arg;
        if (total < 0xFFFF) {
            clearInstruction(pc);
            m_bco(pc+1).arg = uint16_t(total);
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

/** Remove null operations. Those are DROP 0 and SWAP 0.
    DROP 0 appears after doStoreDrop(). */
bool
OptimizerState::doNullOp(PC_t pc)
{
    if ((m_bco(pc).is(Opcode::miStackDrop) || m_bco(pc).is(Opcode::miStackSwap)) && m_bco(pc).arg == 0) {
        clearInstruction(pc);
        return true;
    } else {
        return false;
    }
}

/** Erase unused labels. Those appear frequently. */
bool
OptimizerState::doEraseUnusedLabels(PC_t pc)
{
    /* Remove unreferenced labels */
    if (m_bco(pc).isLabel() && (m_bco(pc).minor & Opcode::jSymbolic) != 0 && m_labelInfo[m_bco(pc).arg].useCount == 0) {
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
OptimizerState::doInvertJumps(PC_t pc)
{
    if (m_bco(pc).isRegularJump()
        && m_labelInfo[m_bco(pc).arg].address == pc+2
        && m_bco(pc+1).isRegularJump()
        && (m_bco(pc+1).minor & Opcode::jPopAlways) == 0
        && ((m_bco(pc).minor & Opcode::jPopAlways) == 0
            || (m_bco(pc+1).minor & Opcode::jAlways) == Opcode::jAlways))
    {
        uint8_t nextMinor = uint8_t((m_bco(pc+1).minor & ~(m_bco(pc).minor & Opcode::jAlways)) | (m_bco(pc).minor & Opcode::jPopAlways));
        if ((nextMinor & Opcode::jAlways) == 0) {
            // Second jump is never taken. Eliminate both.
            clearInstruction(pc);
            clearInstruction(pc+1);
            if (nextMinor & Opcode::jPopAlways) {
                m_bco(pc+1).major = Opcode::maStack;
                m_bco(pc+1).minor = Opcode::miStackDrop;
                m_bco(pc+1).arg = 1;
            }
        } else {
            // Second jump is taken sometimes. Eliminate first jump.
            clearInstruction(pc);
            m_bco(pc+1).minor = nextMinor;
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
OptimizerState::doThreadJumps(PC_t pc)
{
    /* Is it actually a jump? */
    if (!m_bco(pc).isRegularJump()) {
        return false;
    }

    bool had_backward_jump = false;
    uint16_t target_label = m_bco(pc).arg;
    while (1) {
        PC_t target_address = m_labelInfo[target_label].address;
        assert(m_bco(target_address).isLabel());
        if (target_address+1 >= m_bco.getNumInstructions()) {
            /* Jump to label at end of routine */
            break;
        } else if (m_bco(target_address+1).isLabel() && (m_bco(target_address+1).minor & Opcode::jSymbolic) != 0) {
            /* Jump to a label, which is followed by a label. Jump to the later label,
               to make the former unreferenced and removable. */
            target_label = m_bco(target_address+1).arg;
            assert(m_labelInfo[target_label].address == target_address+1);
        } else if (m_bco(target_address+1).isRegularJump()
                   && (m_bco(target_address+1).minor == (Opcode::jAlways | Opcode::jSymbolic)
                       || ((m_bco(pc).minor & Opcode::jPopAlways) == 0
                           && (m_bco(target_address+1).minor & Opcode::jPopAlways) == 0
                           && (m_bco(pc).minor & ~m_bco(target_address+1).minor) == 0)))
        {
            /* Jump to an unconditional jump (frequent in if-within-loops),
               or to a conditional jump with the same condition (in a:=b xor c). */
            uint16_t arg = m_bco(target_address+1).arg;
            if (m_labelInfo[arg].address <= target_address) {
                /* At most one backward jump per iteration to avoid infinite loops */
                if (had_backward_jump) {
                    break;
                }
                had_backward_jump = true;
            }
            target_label = arg;
        } else {
            /* Not a jump or label */
            break;
        }
    }

    /* Did we change anything? */
    if (m_labelInfo[target_label].address == pc+1) {
        /* It is a jump to the next instruction. Delete it. */
        if (m_bco(pc).minor & Opcode::jPopAlways) {
            clearInstruction(pc);
            m_bco(pc).major = Opcode::maStack;
            m_bco(pc).minor = Opcode::miStackDrop;
            m_bco(pc).arg = 1;
        } else {
            clearInstruction(pc);
        }
        return true;
    } else if (target_label != m_bco(pc).arg) {
        /* We followed a jump chain. */
        --m_labelInfo[m_bco(pc).arg].useCount;
        m_bco(pc).arg = target_label;
        ++m_labelInfo[m_bco(pc).arg].useCount;
        return true;
    } else {
        return false;
    }
}

/** Merge conditional jump followed by unconditional jump to same target.
    This appears if a condition is evaluated and ignored, e.g. if someone does "func1() or func2()" as a statement. */
bool
OptimizerState::doMergeJumps(PC_t pc)
{
    if (m_bco(pc).isRegularJump()
        && m_bco(pc+1).isRegularJump()
        && (m_bco(pc+1).minor & Opcode::jAlways) == Opcode::jAlways
        && m_bco(pc).arg == m_bco(pc+1).arg
        && (m_bco(pc).minor & Opcode::jSymbolic) == (m_bco(pc+1).minor & Opcode::jSymbolic))
    {
        // If first jump is 'jXXp', turn it into 'drop 1'; otherwise, turn into 'drop 0' aka NOP.
        uint16_t arg = ((m_bco(pc).minor & Opcode::jPopAlways) != 0) ? 1 : 0;
        m_bco(pc).major = Opcode::maStack;
        m_bco(pc).minor = Opcode::miStackDrop;
        m_bco(pc).arg   = arg;
        return true;
    } else {
        return false;
    }
}

/** Remove unused code. Removes code following an unconditional jump,
    a UTHROW, or SRETURN instruction. Such code is never executed.

    This appears in code such as "If a Then Return Else ..." */
bool
OptimizerState::doRemoveUnused(PC_t pc)
{
    if ((m_bco(pc).isRegularJump() && (m_bco(pc).minor & Opcode::jAlways) == Opcode::jAlways)
        || (m_bco(pc).is(Opcode::miSpecialThrow))
        || (m_bco(pc).is(Opcode::miSpecialTerminate))
        || (m_bco(pc).is(Opcode::miSpecialReturn)))
    {
        // Jump, throw or return
        PC_t i = pc+1;
        while (i < m_bco.getNumInstructions() && !m_bco(i).isLabel()) {
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
OptimizerState::doMergeNegation(PC_t pc)
{
    /* Merge pairs of unary logic into single instructions */
    enum { None = -1, RepFalse = -2, ZapBool = -3 };
    int result = None;

    if (!m_bco(pc+1).is(Opcode::maUnary)) {
        return false;
    }

    switch (m_bco(pc).minor) {
     case interpreter::unNot:
        switch (m_bco(pc+1).minor) {
         case interpreter::unNot:     result = interpreter::unBool;    break;
         case interpreter::unBool:    result = interpreter::unNot;     break;
         case interpreter::unIsEmpty: result = interpreter::unIsEmpty; break;
        }
        break;
     case interpreter::unBool:
        switch (m_bco(pc+1).minor) {
         case interpreter::unNot:     result = interpreter::unNot;     break;
         case interpreter::unBool:    result = interpreter::unBool;    break;
         case interpreter::unNot2:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = interpreter::unIsEmpty; break;
         case interpreter::unZap:     result = ZapBool;   break;
        }
        break;
     case interpreter::unNot2:
        switch (m_bco(pc+1).minor) {
         case interpreter::unBool:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = RepFalse;  break;
        }
        break;
     case interpreter::unIsEmpty:
        switch (m_bco(pc+1).minor) {
         case interpreter::unBool:    result = interpreter::unIsEmpty; break;
         case interpreter::unIsEmpty: result = RepFalse;  break;
        }
        break;
     case interpreter::unZap:
        switch (m_bco(pc+1).minor) {
         case interpreter::unNot2:    result = interpreter::unNot2;    break;
         case interpreter::unIsEmpty: result = interpreter::unNot2;    break;
         case interpreter::unZap:     result = interpreter::unZap;     break;
        }
        break;
     case interpreter::unNeg:
        switch (m_bco(pc+1).minor) {
         case interpreter::unNeg:     result = interpreter::unPos;     break;
         case interpreter::unPos:     result = interpreter::unNeg;     break;
        }
        break;
     case interpreter::unPos:
        switch (m_bco(pc+1).minor) {
         case interpreter::unNeg:     result = interpreter::unNeg;     break;
         case interpreter::unPos:     result = interpreter::unPos;     break;
         case interpreter::unInc:     result = interpreter::unInc;     break;
         case interpreter::unDec:     result = interpreter::unDec;     break;
        }
        break;
     case interpreter::unInc:
        switch (m_bco(pc+1).minor) {
         case interpreter::unDec:     result = interpreter::unPos;     break;
         case interpreter::unPos:     result = interpreter::unInc;     break;
        }
        break;
     case interpreter::unDec:
        switch (m_bco(pc+1).minor) {
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
        m_bco(pc).major   = Opcode::maStack;
        m_bco(pc).minor   = Opcode::miStackDrop;
        m_bco(pc).arg     = 1;
        m_bco(pc+1).major = Opcode::maPush;
        m_bco(pc+1).minor = Opcode::sBoolean;
        m_bco(pc+1).arg   = 0;
        return true;
    } else if (result == ZapBool) {
        /* Replace by 'uzap; ubool' (potentially fewer temporaries) */
        m_bco(pc).minor = interpreter::unZap;
        m_bco(pc+1).minor = interpreter::unBool;
        return true;
    } else {
        /* Replace by 'u<whatever>' */
        clearInstruction(pc);
        m_bco(pc+1).minor = uint8_t(result);
        return true;
    }
}

/** UnaryCondition optimisation. If a logic instruction is followed by
    a conditional jump, modify the jump's condition to evaluate the
    logic, e.g. merge UNOT+JTP intp JFP.

    This appears often in conditions such as "If IsEmpty(x)". */
bool
OptimizerState::doUnaryCondition(PC_t pc)
{
    /* u<logic> / j<cc>p -> j<cc'>p */
    if (m_bco(pc+1).isRegularJump() && (m_bco(pc+1).minor & Opcode::jPopAlways) != 0) {
        uint8_t oldCond = m_bco(pc+1).minor;
        uint8_t newCond = 0;
        if (m_bco(pc).is(interpreter::unIsEmpty)) {
            /* uisempty: t->e, f->tf, e->never */
            if (oldCond & Opcode::jIfTrue) {
                newCond |= Opcode::jIfEmpty;
            }
            if (oldCond & Opcode::jIfFalse) {
                newCond |= Opcode::jIfTrue | Opcode::jIfFalse;
            }
        } else if (m_bco(pc).is(interpreter::unNot)) {
            /* unot: t->f, f->t, e->e */
            if (oldCond & Opcode::jIfTrue) {
                newCond |= Opcode::jIfFalse;
            }
            if (oldCond & Opcode::jIfFalse) {
                newCond |= Opcode::jIfTrue;
            }
            if (oldCond & Opcode::jIfEmpty) {
                newCond |= Opcode::jIfEmpty;
            }
        } else if (m_bco(pc).is(interpreter::unZap)) {
            /* uzap: t->t, f->never, e->fe */
            if (oldCond & Opcode::jIfTrue) {
                newCond |= Opcode::jIfTrue;
            }
            if (oldCond & Opcode::jIfEmpty) {
                newCond |= Opcode::jIfEmpty | Opcode::jIfFalse;
            }
        } else if (m_bco(pc).is(interpreter::unNot2)) {
            /* unot2: t->fe, f->t, e->never */
            if (oldCond & Opcode::jIfTrue) {
                newCond |= Opcode::jIfFalse | Opcode::jIfEmpty;
            }
            if (oldCond & Opcode::jIfFalse) {
                newCond |= Opcode::jIfTrue;
            }
        } else if (m_bco(pc).is(interpreter::unBool)) {
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
            m_bco(pc+1).major = Opcode::maStack;
            m_bco(pc+1).minor = Opcode::miStackDrop;
            m_bco(pc+1).arg   = 1;
        } else {
            /* Update jump */
            m_bco(pc+1).minor = newCond | Opcode::jPopAlways | Opcode::jSymbolic;
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
OptimizerState::doFoldUnaryInt(PC_t pc)
{
    if ((m_bco(pc).minor != Opcode::sInteger && m_bco(pc).minor != Opcode::sBoolean) || !m_bco(pc+1).is(Opcode::maUnary)) {
        return false;
    }

    switch (m_bco(pc+1).minor) {
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
            std::auto_ptr<afl::data::Value> result;
            if (m_bco(pc).minor == Opcode::sInteger) {
                afl::data::IntegerValue iv(+int16_t(m_bco(pc).arg));
                result.reset(interpreter::executeUnaryOperation(m_world, m_bco(pc+1).minor, &iv));
            } else if (int16_t(m_bco(pc).arg) < 0) {
                result.reset(executeUnaryOperation(m_world, m_bco(pc+1).minor, 0));
            } else {
                afl::data::BooleanValue iv(+int16_t(m_bco(pc).arg));
                result.reset(executeUnaryOperation(m_world, m_bco(pc+1).minor, &iv));
            }

            /* Can we encode the result? */
            bool did;
            if (result.get() == 0) {
                m_bco(pc).minor = Opcode::sBoolean;
                m_bco(pc).arg   = uint16_t(-1U);
                did = true;
            } else if (afl::data::ScalarValue* iv = dynamic_cast<afl::data::ScalarValue*>(result.get())) {
                if (dynamic_cast<afl::data::BooleanValue*>(iv) != 0) {
                    m_bco(pc).minor = Opcode::sBoolean;
                    m_bco(pc).arg   = (iv->getValue() != 0);
                    did = true;
                } else if (iv->getValue() >= -32767 && iv->getValue() <= +32767) {
                    m_bco(pc).minor = Opcode::sInteger;
                    m_bco(pc).arg   = static_cast<uint16_t>(iv->getValue());
                    did = true;
                } else {
                    did = false;
                }
            } else {
                did = false;
            }
            if (did) {
                clearInstruction(pc+1);
            }
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
OptimizerState::doFoldBinaryInt(PC_t pc)
{
    if (m_bco(pc).minor != Opcode::sInteger || !m_bco(pc+1).is(Opcode::maBinary)) {
        return false;
    }

    int16_t value = m_bco(pc).arg;
    switch (m_bco(pc+1).minor) {
     case interpreter::biAdd:
        /* "+ 1" => uinc, "+ -1" => udec, "+0" => upos */
        if (value == 0 || value == 1 || value == -1) {
            m_bco(pc+1).major = Opcode::maUnary;
            m_bco(pc+1).minor = (value == 0 ? interpreter::unPos : value == 1 ? interpreter::unInc : interpreter::unDec);
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     case interpreter::biSub:
        /* "- +1" => uinc, "- -1" => uinc, "+0" => upos */
        if (value == 0 || value == 1 || value == -1) {
            m_bco(pc+1).major = Opcode::maUnary;
            m_bco(pc+1).minor = (value == 0 ? interpreter::unPos : value == 1 ? interpreter::unDec : interpreter::unInc);
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
            m_bco(pc+1).major = Opcode::maUnary;
            m_bco(pc+1).minor = (value == 1 ? interpreter::unPos : interpreter::unNeg);
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     case interpreter::biPow:
        /* "^ +1" => upos */
        if (value == 1) {
            m_bco(pc+1).major = Opcode::maUnary;
            m_bco(pc+1).minor = interpreter::unPos;
            clearInstruction(pc);
            return true;
        } else {
            return false;
        }
     default:
        return false;
    }
}

/** Remove type checks following a binary operation. */
bool
OptimizerState::doFoldBinaryTypeCheck(PC_t pc)
{
    if (m_bco(pc+1).major != Opcode::maUnary) {
        return false;
    }

    bool match;
    const uint8_t nextMinor = m_bco(pc+1).minor;
    switch (m_bco(pc).minor) {
     case interpreter::biAnd:
     case interpreter::biOr:
     case interpreter::biXor:
     case interpreter::biCompareEQ:
     case interpreter::biCompareEQ_NC:
     case interpreter::biCompareNE:
     case interpreter::biCompareNE_NC:
     case interpreter::biCompareLE:
     case interpreter::biCompareLE_NC:
     case interpreter::biCompareLT:
     case interpreter::biCompareLT_NC:
     case interpreter::biCompareGE:
     case interpreter::biCompareGE_NC:
     case interpreter::biCompareGT:
     case interpreter::biCompareGT_NC:
        // Result is bool; remove ubool
        // Appears in "x := (a=b or c)", "if not(a=b or c)".
        match = (nextMinor == interpreter::unBool);
        break;

     case interpreter::biSub:
     case interpreter::biMult:
     case interpreter::biDivide:
     case interpreter::biIntegerDivide:
     case interpreter::biRemainder:
     case interpreter::biPow:
     case interpreter::biFindStr:
     case interpreter::biFindStr_NC:
     case interpreter::biBitAnd:
     case interpreter::biBitOr:
     case interpreter::biBitXor:
     case interpreter::biATan:
     case interpreter::biArrayDim:
        // Result is numeric; remove upos
        // Appears in "for i:=a*2 to b".
        match = (nextMinor == interpreter::unPos);
        break;

     default:
        // In theory, we could also optimize combinations such as bconcat/ustr.
        // However, since we do not implicitly generate ustr, don't bother for now;
        // if user asked for them, they will get them.
        match = false;
    }

    if (match) {
        clearInstruction(pc+1);
        return true;
    } else {
        return false;
    }
}

/** Fold conditional jump on constant. If a literal is immediately
    followed by a conditional jump, or by a jump to a conditional
    jump, evaluate that condition and generate an unconditional jump
    instead.

    This appears in 'Do While True' or 'FindShip(1)'. */
bool
OptimizerState::doFoldJump(PC_t pc)
{
    if (m_bco(pc).minor != Opcode::sInteger && m_bco(pc).minor != Opcode::sBoolean) {
        return false;
    }
    if (!m_bco(pc+1).isRegularJump()) {
        return false;
    }

    /* Figure out condition */
    int cond;
    if (m_bco(pc).minor == Opcode::sBoolean && int16_t(m_bco(pc).arg) < 0) {
        cond = Opcode::jIfEmpty;
    } else if (m_bco(pc).arg == 0) {
        cond = Opcode::jIfFalse;
    } else {
        cond = Opcode::jIfTrue;
    }

    /* Check jump */
    if ((m_bco(pc+1).minor & Opcode::jAlways) == Opcode::jAlways) {
        /* Unconditional jump. Check whether it jumps at a conditional jump. */
        if (m_bco(pc+1).minor & Opcode::jPopAlways) {
            /* Pathological case: push + jp, turn into j */
            m_bco(pc+1).minor &= uint8_t(~Opcode::jPopAlways);
            clearInstruction(pc);
            return true;
        }

        /* Follow this jump */
        uint16_t target_label = m_bco(pc+1).arg;
        PC_t target_address = m_labelInfo[target_label].address;
        if (target_address+1 >= m_bco.getNumInstructions() || !m_bco(target_address+1).isRegularJump()) {
            /* Jump to end of routine or not to a jump; keep it */
            return false;
        }

        if (m_bco(target_address+1).minor & cond) {
            /* Jump to conditional jump that will be taken; follow it */
            if (m_bco(target_address+1).minor & Opcode::jPopAlways) {
                clearInstruction(pc);
            }
            --m_labelInfo[m_bco(pc+1).arg].useCount;
            m_bco(pc+1).arg = m_bco(target_address+1).arg;
            ++m_labelInfo[m_bco(pc+1).arg].useCount;
            return true;
        } else {
            /* Conditional jump is not taken; we cannot optimize this */
            return false;
        }
    } else {
        /* Conditional jump */
        if (m_bco(pc+1).minor & Opcode::jPopAlways) {
            /* This jump will pop, drop the push */
            clearInstruction(pc);
        }

        if (m_bco(pc+1).minor & cond) {
            /* This jump will be taken. Make it unconditional. */
            m_bco(pc+1).minor |= Opcode::jAlways;
            m_bco(pc+1).minor &= uint8_t(~Opcode::jPopAlways);
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
OptimizerState::doPopPush(PC_t pc)
{
    /* Next instruction must be push with same address, but may not be
       a named variable: assigning those implies a type-cast. */
    if (m_bco(pc+1).major == Opcode::maPush
        && m_bco(pc).minor == m_bco(pc+1).minor
        && m_bco(pc).arg == m_bco(pc+1).arg
        && m_bco(pc).minor != Opcode::sNamedVariable)
    {
        /* do it */
        m_bco(pc+1).major = Opcode::maStore;
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
OptimizerState::doCompareNC(PC_t pc)
{
    const uint8_t pm = m_bco(pc).minor;
    const uint8_t cm = m_bco(pc+1).minor;

    /* First must be push literal */
    if (pm != Opcode::sInteger && pm != Opcode::sBoolean && pm != Opcode::sLiteral)
        return false;

    /* Second must be case-blind compare */
    if (m_bco(pc+1).major != Opcode::maBinary) {
        return false;
    }
    if (cm != interpreter::biCompareEQ_NC && cm != interpreter::biCompareNE_NC && cm != interpreter::biFirstStr_NC && cm != interpreter::biRestStr_NC && cm != interpreter::biFindStr_NC) {
        return false;
    }

    /* If first is pushlit, it must be an acceptable literal */
    if (pm == Opcode::sLiteral) {
        afl::data::Value* iv = m_bco.getLiteral(m_bco(pc).arg);
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
    --m_bco(pc+1).minor;
    return true;
}

/** Convert binary operation that produces an integer, followed by comparison against 0, to direct jump-on-zero. */
bool
OptimizerState::doIntCompare(PC_t pc)
{
    // First operation
    switch (m_bco(pc).minor) {
     case interpreter::biAnd:
     case interpreter::biOr:
     case interpreter::biXor:
     case interpreter::biFindStr_NC:
     case interpreter::biFindStr:
     case interpreter::biBitAnd:
     case interpreter::biBitOr:
     case interpreter::biBitXor:
        break;
     default:
        return false;
    }

    // Push zero
    const uint8_t pushOp = m_bco(pc+1).major;
    const uint8_t pushType = m_bco(pc+1).minor;
    if (pushOp != Opcode::maPush || (pushType != Opcode::sInteger && pushType != Opcode::sBoolean) || m_bco(pc+1).arg != 0) {
        return false;
    }

    // Comparison
    const uint8_t cmpOp = m_bco(pc+2).major;
    const uint8_t cmpType = m_bco(pc+2).minor;
    if (cmpOp != Opcode::maBinary) {
        return false;
    }

    if (cmpType == interpreter::biCompareEQ_NC || cmpType == interpreter::biCompareEQ) {
        // op / pushint 0 / bcmpeq --> op / unot
        m_bco(pc+1).major = Opcode::maUnary;
        m_bco(pc+1).minor = interpreter::unNot;
        m_bco(pc+1).arg = 0;
        clearInstruction(pc+2);
        return true;
    } else if (cmpType == interpreter::biCompareNE_NC || cmpType == interpreter::biCompareNE) {
        // op / pushint 0 / bcmpne --> op / ubool
        m_bco(pc+1).major = Opcode::maUnary;
        m_bco(pc+1).minor = interpreter::unBool;
        m_bco(pc+1).arg = 0;
        clearInstruction(pc+2);
        return true;
    } else {
        return false;
    }
}

bool
OptimizerState::doTailMerge(PC_t pc)
{
    // Only at -O2, because this reduces quality of line number information
    if (m_optimisationLevel < 2) {
        return false;
    }

    // Only unconditional, non-popping jumps
    if (m_bco(pc).minor != Opcode::jAlways + Opcode::jSymbolic) {
        return false;
    }

    // Only forward jumps
    PC_t target = m_labelInfo[m_bco(pc).arg].address;
    PC_t source = pc;
    if (target <= source) {
        return false;
    }

    // Determine common instructions
    while (source > 0 && m_bco(source-1).major != Opcode::maJump && m_bco(source-1) == m_bco(target-1)) {
        clearInstruction(source);
        --source;
        --target;
    }

    // Did something?
    if (source == pc) {
        return false;
    }

    // Add label.
    BytecodeObject::Label_t label;
    if (m_bco(target-1).isLabel()) {
        label = m_bco(target-1).arg;
    } else {
        label = m_bco.makeLabel();
        m_bco.insertLabel(label, target);
    }
    m_bco(source).major = Opcode::maJump;
    m_bco(source).minor = Opcode::jAlways + Opcode::jSymbolic;
    m_bco(source).arg = label;
    initLabelInfo();
    return true;
}

bool
OptimizerState::doDeadStore(PC_t pc)
{
    // Must be 'sreturn'
    if (m_bco(pc).minor != Opcode::miSpecialReturn) {
        return false;
    }

    // Strike out preceding null operations until we find a non-null operation
    return removeDeadStores(pc);
}

bool
OptimizerState::removeDeadStores(PC_t pc)
{
    // Only at -O2, because this has questionable use and hurts debuggability
    // (Also, several unit tests for other optimisations rely on removeDeadStores only happening at -O2.)
    if (m_optimisationLevel < 2) {
        return false;
    }

    bool did = false;
    while (pc > 0) {
        --pc;
        if (m_bco(pc).is(Opcode::maPush) && (m_bco(pc).minor == Opcode::sLiteral || m_bco(pc).minor == Opcode::sInteger || m_bco(pc).minor == Opcode::sBoolean)) {
            // skip over this one
        } else if (m_bco(pc).is(Opcode::maStore) && m_bco(pc).minor == Opcode::sLocal) {
            // store into local -> dead
            clearInstruction(pc);
            did = true;
        } else if (m_bco(pc).is(Opcode::maPop) && m_bco(pc).minor == Opcode::sLocal) {
            // pop into local -> dead
            m_bco(pc).major = Opcode::maStack;
            m_bco(pc).minor = Opcode::miStackDrop;
            m_bco(pc).arg = 1;
            did = true;
        } else if (m_bco(pc).is(Opcode::maUnary) || m_bco(pc).is(Opcode::maBinary)) {
            // skip over simple operations
        } else if (m_bco(pc).isLabel()) {
            // skip over labels
        } else {
            break;
        }
    }
    return did;
}


void
interpreter::optimize(World& world, BytecodeObject& bco, int level)
{
    unfuseInstructions(bco);
    while (OptimizerState(world, bco, level).iterate()) {
        bco.compact();
    }
    fuseInstructions(bco);
}
