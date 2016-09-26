/**
  *  \file interpreter/fusion.cpp
  *  \brief Interpreter: Instruction Fusion
  *
  *  To avoid unnecessary creation of temporary values, we can fuse
  *  instructions together. The idea is to detect patterns, and mark
  *  fusable instructions with a special major opcode. The interpreter
  *  then executes these major opcodes in one cycle, saving
  *  temporaries where possible.
  *
  *  Instruction fusion is an internal optimisation. Fused
  *  instructions are not saved to the VM file; see
  *  interpreter::Opcode::getExternalMajor().
  *
  *  No changes are done to the instruction stream other than the
  *  major opcode. Thus, all information can still be found at the
  *  usual places. In particular, if a jump exists into the middle of
  *  a fused instruction, it will simply proceed by executing the
  *  original unfused instruction.
  *
  *  FIXME: file taken almost verbatim; needs to be beefed up a little
  */

#include "interpreter/fusion.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/binaryoperation.hpp"

namespace {
    const uint32_t STORE_DEPTH = 10;

    /** Local Variable Tracer.
        Used to find local variables that are guarenteed to be overwritten.
        This is an object to allow re-using some state between different invocations. */
    class IntLocalTracer {
     public:
        IntLocalTracer(const interpreter::BytecodeObject& bco);

        bool isOverwrittenLocal(interpreter::BytecodeObject::PC_t pc, uint16_t address, uint32_t depth);
        bool hasExceptionHandling();

     private:
        const interpreter::BytecodeObject& bco;
        enum { DontKnow, No, Yes } ehState;
    };
}

/** Constructor.
    \param bco BCO to analyze */
IntLocalTracer::IntLocalTracer(const interpreter::BytecodeObject& bco)
    : bco(bco),
      ehState(DontKnow)
{ }

/** Check for overwritten local variable.
    If we know a local variable is overwritten, we can clobber it during execution.
    This function must return true if it can prove that the variable is overwritten,
    false if it is re-used or it cannot be proven.
    To limit the amount of work to do, we trace only as many instructions as given
    by the %depth parameter. If we cannot prove overwriting within so-many instructions,
    we assume the variable is still needed and therefore return false.

    \param pc Start at this program counter
    \param address Look for this local variable
    \param depth Check this many instructions

    \retval true The variable is guaranteed to be overwritten and losing its value is ok
    \retval false The variable's value might still be required, or we are unsure */
bool
IntLocalTracer::isOverwrittenLocal(interpreter::BytecodeObject::PC_t pc,
                                   uint16_t address,
                                   uint32_t depth)
{
    /* Check depth */
    while (depth > 0 && pc < bco.getNumInstructions()) {
        const interpreter::Opcode& op = bco(pc++);
        --depth;       
        switch (op.major) {
         case interpreter::Opcode::maPush:
         case interpreter::Opcode::maFusedUnary:
         case interpreter::Opcode::maFusedBinary:
         case interpreter::Opcode::maFusedComparison2:
         case interpreter::Opcode::maInplaceUnary:
            /* Accept only pushloc for different locals, or literals */
            if ((op.minor == interpreter::Opcode::sLocal && op.arg != address)
                || op.minor == interpreter::Opcode::sLiteral
                || op.minor == interpreter::Opcode::sInteger
                || op.minor == interpreter::Opcode::sBoolean)
            {
                /* accept */
            } else {
                /* reject */
                return false;
            }
            break;

         case interpreter::Opcode::maBinary:
         case interpreter::Opcode::maFusedComparison:
         case interpreter::Opcode::maUnary:
         case interpreter::Opcode::maTernary:
            /* These can throw. Do not accept if it smells like exception handling,
               because the exception handler might want to examine this local variable. */
            if (hasExceptionHandling()) {
                return false;
            }
            break;

         case interpreter::Opcode::maJump:
            /* Trace regular jumps */
            if (op.isLabel()) {
                /* labels: skip */
            } else if (op.isRegularJump()) {
                /* regular jump: continue tracing at jump target */
                if ((op.minor & interpreter::Opcode::jAlways) != interpreter::Opcode::jAlways) {
                    /* conditional jump: trace following instruction first */
                    if (!isOverwrittenLocal(pc, address, depth)) {
                        return false;
                    }
                }
                pc = bco.getJumpTarget(op.minor, op.arg);
            } else {
                /* jdz, catch: too complex/rare to reason about */
                return false;
            }
            break;
            
         case interpreter::Opcode::maIndirect:
         case interpreter::Opcode::maMemref:
            /* These can do anything. Do not accept. */
            return false;

         case interpreter::Opcode::maStack:
            /* Always safe, does not modify locals */
            break;

         case interpreter::Opcode::maPop:
         case interpreter::Opcode::maStore:
            /* These are what we want. If we found the store to our address, report so.
               Otherwise, continue tracing. (We're looking for guaranteed matches;
               unlike for push, finding a possible match is not sufficient.) */
            if (op.minor == interpreter::Opcode::sLocal && op.arg == address) {
                return true;
            }
            break;

         case interpreter::Opcode::maDim:
            /* Always safe, does not modify existing locals */
            break;

         case interpreter::Opcode::maSpecial:
            /* These can do anything */
            return false;
        }
    }

    /* When we're here, we cannot prove that this local is overwritten */
    return false;
}

/** Check whether exception handling is in use.
    \retval true This BCO uses exception handling
    \retval false This BCO does not use exception handling */
bool
IntLocalTracer::hasExceptionHandling()
{
    if (ehState == DontKnow) {
        ehState = No;
        for (interpreter::BytecodeObject::PC_t i = 0, n = bco.getNumInstructions(); i < n; ++i) {
            const interpreter::Opcode& op = bco(i);
            if (op.major == interpreter::Opcode::maJump && (op.minor & ~interpreter::Opcode::jSymbolic) == interpreter::Opcode::jCatch) {
                ehState = Yes;
                break;
            }
        }
    }
    return (ehState == Yes);
}

/**************************** Other Functions ****************************/

/** Check for comparison instruction. */
static bool
isComparison(const interpreter::Opcode& op)
{
    return op.is(interpreter::Opcode::maBinary)
        && (op.minor >= interpreter::biCompareEQ && op.minor <= interpreter::biCompareGT_NC);
}

/** Check for regular conditional jump. */
static bool
isConditionalJump(const interpreter::Opcode& op)
{
    return op.isRegularJump()
        && (op.minor & interpreter::Opcode::jPopAlways) != 0;
}

/** Check for direct storage class, i.e. storage classes that directly
    refer to a data segment that can provide/take values with defined
    ownership semantics. */
static bool
isDirectStorageClass(const interpreter::Opcode& op)
{
    switch (op.minor) {
     case interpreter::Opcode::sLocal:
     case interpreter::Opcode::sStatic:
     case interpreter::Opcode::sShared:
     case interpreter::Opcode::sNamedShared:
     case interpreter::Opcode::sLiteral:
        return true;

     default:
        return false;
    }
}

/** Perform instruction fusion.
    Processes the whole BCO and fuses instructions where possible.
    \param bco Byte code object, updated in-place */
void
interpreter::fuseInstructions(BytecodeObject& bco)
{
    /* Pathological case */
    const BytecodeObject::PC_t n = bco.getNumInstructions();
    if (n == 0) {
        return;
    }

    IntLocalTracer tracer(bco);

    /* Iterate backwards. We'll be combining instruction pairs. */
    for (interpreter::BytecodeObject::PC_t i = n-1; i > 0; --i) {
        const interpreter::Opcode& me = bco(i);
        interpreter::Opcode& prev = bco(i-1);
        switch (me.major) {
         case interpreter::Opcode::maBinary:
            /* push + binary -> fusedbinary */
            if (prev.major == interpreter::Opcode::maPush && isDirectStorageClass(prev)) {
                prev.major = interpreter::Opcode::maFusedBinary;
            }
            break;

         case interpreter::Opcode::maUnary:
            /* push + unary -> fusedunary/inplaceunary */
            if (prev.major == interpreter::Opcode::maPush && isDirectStorageClass(prev)) {
                if (prev.minor == interpreter::Opcode::sLocal
                    && (me.minor == unInc || me.minor == unDec)
                    && tracer.isOverwrittenLocal(i+1, prev.arg, STORE_DEPTH))
                {
                    prev.major = interpreter::Opcode::maInplaceUnary;
                } else {
                    prev.major = interpreter::Opcode::maFusedUnary;
                }
            }
            break;

         case interpreter::Opcode::maJump:
            /* bcmpXX + jXXp -> fusedcomparison */
            if (isConditionalJump(me) && isComparison(prev)) {
                prev.major = interpreter::Opcode::maFusedComparison;
            }
            break;

         case interpreter::Opcode::maFusedComparison:
            /* push + fusedcomparison -> fusedcomparison2 */
            if (prev.major == interpreter::Opcode::maPush && isDirectStorageClass(prev)) {
                prev.major = interpreter::Opcode::maFusedComparison2;
            }
            break;
        }
    }
}

/** Undo instruction fusion.
    Restores the original instructions in the whole BCO.
    \param bco Byte code object, updated in-place */
void
interpreter::unfuseInstructions(BytecodeObject& bco)
{
    for (BytecodeObject::PC_t i = 0, n = bco.getNumInstructions(); i < n; ++i) {
        bco(i).major = bco(i).getExternalMajor();
    }
}
