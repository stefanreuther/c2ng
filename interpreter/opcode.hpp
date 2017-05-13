/**
  *  \file interpreter/opcode.hpp
  *  \brief Class interpreter::Opcode
  */
#ifndef C2NG_INTERPRETER_OPCODE_HPP
#define C2NG_INTERPRETER_OPCODE_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"
#include "interpreter/unaryoperation.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/ternaryoperation.hpp"

/* Avoid gcc/Linux namespace pollution */
#undef major
#undef minor

namespace interpreter {

    /** Opcode for compiled CCScript.
        Each instruction is represented by one object of this type, totalling to 32 bit per instruction. */
    struct Opcode {
        uint8_t  major;                ///< Major opcode.
        uint8_t  minor;                ///< Minor opcode/parameter.
        uint16_t arg;                  ///< Parameter.

        /** Major opcodes. */
        enum Major {
            /* Real opcodes. Part of external representation and must not change. */
            maPush,                    ///< Push.              minor=Scope,          arg=table index/value.
            maBinary,                  ///< Binary operation.  minor=IntBinaryOperation.
            maUnary,                   ///< Unary operation.   minor=IntUnaryOperation.
            maTernary,                 ///< Ternary operation. minor=IntTernaryOperation.
            maJump,                    ///< Jump.              minor=jump flags,     arg=target address/name.
            maIndirect,                ///< Indirect call.     minor=Indirect,       arg=number of args.
            maStack,                   ///< Stack operations.  minor=Stack,          arg=parameter.
            maPop,                     ///< Pop.               minor=Scope,          arg=table index.
            maStore,                   ///< Store.             minor=Scope,          arg=name table index.
            maMemref,                  ///< Member references. minor=Indirect,       arg=member name table index.
            maDim,                     ///< Make variable.     minor=Scope,          arg=name.
            maSpecial,                 ///< Specials.          minor=Special.

            /* Fused opcodes. Only used internally, never externally. Can change as required.
               A fused opcode is a shortcut for an instruction sequence which replaces the
               "major" field of the first instruction.
               The "minor" and "arg" fields remain intact. */
            maFusedUnary,              ///< Fused unary. maPush + maUnary.
            maFusedBinary,             ///< Fused binary. maPush + maBinary.
            maFusedComparison,         ///< Fused comparison + jump. maBinary + maJump.
            maFusedComparison2,        ///< Fused push + comparison + jump. maPush + maBinary + maJump.
            maInplaceUnary             ///< In-place unary. Destructive push + unary.
        };

        /** Scope. Used as minor opcode for Push, Pop, Store, Dim. This defines the interpretation of arg. */
        enum Scope {
            sNamedVariable,            ///< Named variable. Parameter is index into BCO's name list. Used for Push/Pop/Store.
            sLocal,                    ///< Local variable. Parameter is index into local frame data. Used for Push/Pop/Store/Dim.
            sStatic,                   ///< Static variable. Parameter is index into static (topmost frame) data. Used for Push/Pop/Store/Dim.
            sShared,                   ///< Shared variable. Parameter is index into shared data. Used for Push/Pop/Store/Dim.
            sNamedShared,              ///< Named shared variable. Parameter is index into BCO's name list. Used for Push/Pop/Store.
            sLiteral,                  ///< Literal. Parameter is index into BCO's literal list. Used for Push.
            sInteger,                  ///< Integer. Parameter is literal. Used for Push.
            sBoolean                   ///< Boolean. Parameter is literal. Used for Push.
        };

        /** Jump flags. Used as minor opcode for Jump. */
        // FIXME: should be const uint8_t
        enum {
            // Regular jumps:
            jIfTrue    = 1,            ///< Jump if true.
            jIfFalse   = 2,            ///< Jump if false.
            jIfEmpty   = 4,            ///< Jump if empty.
            jAlways    = 7,            ///< Jump always (sum of the above).
            jPopAlways = 8,            ///< Pop after checking condition.

            // Special jumps:
            jOtherMask = 0x70,         ///< If any of these bits is set, this is not a regular jump.
            jCatch     = 16,           ///< Push EH frame, jump on throw.
            jDecZero   = 17,           ///< Decrement and jump if zero.
            jLabel     = 0,            ///< This is a label (jump never).

            // General flag:
            jSymbolic  = 128           ///< If set, address is not relocated yet.
        };

        /** Stack operations. Used as minor opcode for Stack. */
        enum Stack {
            miStackDup,                ///< Duplicate nn'th element.
            miStackDrop,               ///< Drop nn elements.
            miStackSwap                ///< Swap nn'th element and TOS.
        };

        /** Indirect/Member operations. Used as minor opcode for Indirect/Memref.
            For Indirect, arg is number of parameters on stack. For Memref, arg
            is name table index of member name. Stack contains args (for Indirect),
            then object to call/dereference, then value to assign (for store/pop). */
        enum Indirect {
            miIMCall   = 0,            ///< Call fun(args), no result.
            miIMLoad   = 1,            ///< Call fun(args), one result.
            miIMStore  = 2,            ///< Assign fun(args) := value, keep value.
            miIMPop    = 3,            ///< Assign fun(args) := value, drop value.
            miIMOpMask = 3,            ///< Bitmask for the above.

            miIMRefuseFunctions  = 4,  ///< Refuse "function"-type objects.
            miIMRefuseProcedures = 8   ///< Refuse "procedure"-type objects.
        };

        /** Specials. */
        enum Special {
            miSpecialUncatch,           ///< Cancel previous catch.
            miSpecialReturn,            ///< Terminate current frame.
            miSpecialWith,              ///< Turn TOS into context.
            miSpecialEndWith,           ///< Cancel miSpecialWith.
            miSpecialFirstIndex,        ///< Iterate over TOS.
            miSpecialNextIndex,         ///< Next iteration.
            miSpecialEndIndex,          ///< Cancel miSpecialFirstIndex prematurely.
            miSpecialEvalStatement,     ///< Evaluate n strings as statement.
            miSpecialEvalExpr,          ///< Evaluate one string as expression.
            miSpecialDefSub,            ///< Define subroutine.
            miSpecialDefShipProperty,   ///< Create ship propety.
            miSpecialDefPlanetProperty, ///< Create planet propety.
            miSpecialLoad,              ///< Load file. One argument (=file name) on stack.
            miSpecialPrint,             ///< Print to console. One argument on stack.
            miSpecialAddHook,           ///< Add handler to hook.
            miSpecialRunHook,           ///< Run hook.
            miSpecialThrow,             ///< Throw exception.
            miSpecialTerminate,         ///< Terminate process.
            miSpecialSuspend,           ///< Suspend process.
            miSpecialNewArray,          ///< Make blank array, given dimensions. arg is number of dimensions, which are on stack.
            miSpecialMakeList,          ///< Make 1D array from values. arg is number of values, which are on stack.
            miSpecialNewHash,           ///< Make blank hash.
            miSpecialInstance,          ///< Make new struct instance.
            miSpecialResizeArray,       ///< Resize array, given dimensions. arg is number of dimensions, which are on stack.
            miSpecialBind               ///< Bind arguments to make a closure. arg is number of arguments.
        };

        /** Get template for disassembling this opcode.
            The caller must fill in the placeholders from the opcode's \c arg field.
            - %n name table index
            - %l literal table index
            - %s subroutine table index
            - %u unsigned int
            - %d signed int
            - %L local variable name, given by address
            - %T static variable name, given by address
            - %G shared variable name, given by address
            \return template string */
        String_t getDisassemblyTemplate() const;

        /** Check for special (miSpecialXXX) instruction.
            \param sp Instruction to check for
            \return true on match */
        bool is(Special sp) const;

        /** Check for stack (miStackXXX) instruction.
            \param st Instruction to check for
            \return true on match */
        bool is(Stack st) const;

        /** Check major opcode.
            \param m Major opcode to check for
            \return true on match */
        bool is(Major m) const;

        /** Check for unary (unXXX) instruction.
            \param un Operation to check for
            \return true on match */
        bool is(UnaryOperation un) const;

        /** Check for binary (biXXX) instruction.
            \param bi Operation to check for
            \return true on match */
        bool is(BinaryOperation bi) const;

        /** Check for ternary (teXXX) instruction.
            \param te Operation to check for
            \return true on match */
        bool is(TernaryOperation te) const;

        /** Check for jump or catch.
            This accepts all instructions that have a label as a target, that is, all jumps and catch (but not labels).
            \return true on match */
        bool isJumpOrCatch() const;

        /** Check for regular jumps.
            This accepts all regular jumps, but not special jumps (jdz), labels or catch.
            \return true on match */
        bool isRegularJump() const;

        /** Check for label.
            \return true on match */
        bool isLabel() const;

        /** Get external "major" value.
            For fused instructions, returns the original opcode.
            \return external opcode */
        uint8_t getExternalMajor() const;
    };

}

// Check for special (miSpecialXXX) instruction.
inline bool
interpreter::Opcode::is(Special sp) const
{
    return major == maSpecial && minor == sp;
}

// Check for stack (miStackXXX) instruction.
inline bool
interpreter::Opcode::is(Stack st) const
{
    return major == maStack && minor == st;
}

// Check major opcode.
inline bool
interpreter::Opcode::is(Major m) const
{
    return major == m;
}

// Check for unary (unXXX) instruction.
inline bool
interpreter::Opcode::is(UnaryOperation un) const
{
    return major == maUnary && minor == un;
}

// Check for binary (biXXX) instruction.
inline bool
interpreter::Opcode::is(BinaryOperation bi) const
{
    return major == maBinary && minor == bi;
}

// Check for ternary (teXXX) instruction.
inline bool
interpreter::Opcode::is(TernaryOperation te) const
{
    return major == maTernary && minor == te;
}

// Check for jump or catch.
inline bool
interpreter::Opcode::isJumpOrCatch() const
{
    return major == maJump && (minor & ~jSymbolic) != 0;
}

// Check for regular jumps.
inline bool
interpreter::Opcode::isRegularJump() const
{
    return major == maJump && (minor & jOtherMask) == 0 && (minor & jAlways) != 0;
}

// Check for label.
inline bool
interpreter::Opcode::isLabel() const
{
    return major == maJump && (minor & ~jSymbolic) == 0;
}

#endif
