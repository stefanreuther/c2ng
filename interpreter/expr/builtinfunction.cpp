/**
  *  \file interpreter/expr/builtinfunction.cpp
  *  \brief Code Generation for Builtin Functions
  */

#include <cassert>
#include <climits>
#include "interpreter/expr/builtinfunction.hpp"
#include "afl/base/countof.hpp"
#include "afl/data/stringvalue.hpp"
#include "interpreter/binaryoperation.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/identifiernode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/expr/rvaluefunctioncallnode.hpp"
#include "interpreter/opcode.hpp"
#include "interpreter/unaryoperation.hpp"

using interpreter::BytecodeObject;
using interpreter::CompilationContext;
using interpreter::Error;
using interpreter::Opcode;
using interpreter::expr::BuiltinFunctionDescriptor;
using interpreter::expr::FunctionCallNode;
using interpreter::expr::IdentifierNode;
using interpreter::expr::LiteralNode;
using interpreter::expr::Node;
using interpreter::expr::RValueFunctionCallNode;

namespace {
    enum {
        FC_Generic,
        FC_Ship,
        FC_Planet
    };

    /** "If" function. This binary or ternary function does not
        evaluate all its arguments. */
    class IfFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** Function that "folds" any number of arguments into a single value using
        a binary operation. Used for "Min" and "Max" functions as well as bitwise
        operations.

        This has special cases for Min/Max (to handle the CaseBlind flag), and for
        BitOr/BitAnd/BitXor (to enforce type safety when used with just one arg),
        but could otherwise be used to fold anything. */
    class FoldFunctionCallNode : public RValueFunctionCallNode {
        uint8_t m_minor;
     public:
        FoldFunctionCallNode(uint8_t minor)
            : m_minor(minor)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** Regular builtin function. Generates code for unary or binary functions.
        Caller must make sure that it receives an appropriate number of parameters. */
    class BuiltinFunctionNode : public RValueFunctionCallNode {
        uint8_t m_minor;
     public:
        BuiltinFunctionNode(uint8_t minor)
            : m_minor(minor)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** Regular builtin binary function that can distinguish case.
        For case-blind operation, generates opcode minor+1. */
    class CaseFunctionNode : public RValueFunctionCallNode {
        uint8_t m_minor;
     public:
        CaseFunctionNode(uint8_t minor)
            : m_minor(minor)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "Find" function. Iterates over its first arg (an array),
        looking for an object where the second arg (an expression) yields
        true, returning third arg. */
    class FindFunctionCallNode : public RValueFunctionCallNode {
        uint8_t m_which;
     public:
        FindFunctionCallNode(uint8_t which)
            : m_which(which)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "Count" function. Iterates over its first arg (an array), and
        counts objects where the the second arg (an expression) yields
        true. */
    class CountFunctionCallNode : public RValueFunctionCallNode {
        uint8_t m_which;
     public:
        CountFunctionCallNode(uint8_t which)
            : m_which(which)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** One- or two-argument function. Note that this special-cases over
        the opcode to deal with a missing second argument. */
    class OneTwoFunctionCallNode : public RValueFunctionCallNode {
        uint8_t m_minor;
     public:
        OneTwoFunctionCallNode(uint8_t minor)
            : m_minor(minor)
            { }
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "Mid" function. This function takes two or three arguments, and
        is implemented as one or two binary operations. */
    class MidFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "StrCase" function. This function just changes the compilation
        environment for its argument. */
    class StrCaseFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "Key" function. The first argument of this function is a
        keymap, not an expression. */
    class KeyFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };

    /** "ByName" function. The first argument of this function is a
        keymap, not an expression. */
    class ByNameFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { throw Error("\"ByName\" not allowed here"); }
        void compileEffect(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/) const
            { throw Error("\"ByName\" not allowed here"); }
        void compileCondition(BytecodeObject& /*bco*/, const CompilationContext& /*cc*/, BytecodeObject::Label_t /*ift*/, BytecodeObject::Label_t /*iff*/) const
            { throw Error("\"ByName\" not allowed here"); }
        void compileName(BytecodeObject& bco, const CompilationContext& cc) const
            {
                m_args[0]->compileValue(bco, cc);
                bco.addInstruction(Opcode::maUnary, interpreter::unUCase, 0);
            }
    };

    /** "Eval" function. This unary function evaluates its string
        argument as an expression. */
    class EvalFunctionCallNode : public RValueFunctionCallNode {
     public:
        void compileValue(BytecodeObject& bco, const CompilationContext& cc) const;
        void compileEffect(BytecodeObject& bco, const CompilationContext& cc) const
            { defaultCompileEffect(bco, cc); }
        void compileCondition(BytecodeObject& bco, const CompilationContext& cc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff) const
            { defaultCompileCondition(bco, cc, ift, iff); }
    };


    /** Helper for FindFunctionCallNode/CountFunctionCallNode: compile "array" expression.
        This is given explicitly for the generic version, and a hardcoded global variable for the specific ones.
        \param bco   Output bytecode object
        \param cc    Compilation context
        \param which Which flavour of Find/Count we're compiling (FC_xxx)
        \param args  Arguments */
    void compileFCArray(BytecodeObject& bco, const CompilationContext& cc, uint8_t which, const std::vector<const Node*>& args)
    {
        switch (which) {
         case FC_Generic:
            args[0]->compileValue(bco, cc);
            break;
         case FC_Ship:
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("SHIP"));
            break;
         case FC_Planet:
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("PLANET"));
            break;
         default:
            throw Error::internalError("Not implemented");
        }
    }

    /** Helper for FindFunctionCallNode/CountFunctionCallNode: compile "condition" expression.
        This is the second arg for the generic version, the first for the specific ones.
        \param bco   Output bytecode object
        \param cc    Compilation context
        \param which Which flavour of Find/Count we're compiling (FC_xxx)
        \param args  Arguments */
    void compileFCCondition(BytecodeObject& bco, const CompilationContext& cc,
                            BytecodeObject::Label_t ift, BytecodeObject::Label_t iff,
                            uint8_t which, const std::vector<const Node*>& args)
    {
        switch (which) {
         case FC_Generic:
            args[1]->compileCondition(bco, cc, ift, iff);
            break;
         case FC_Ship:
         case FC_Planet:
            args[0]->compileCondition(bco, cc, ift, iff);
            break;
         default:
            throw Error::internalError("Not implemented");
        }
    }

    /** Helper for FindFunctionCallNode: compile "value" expression.
        This is given explicitly for the generic version, and a hardcoded global variable for
        the specific ones.
        \param bco   Output bytecode object
        \param cc    Compilation context
        \param which Which flavour of Find we're compiling (FC_xxx)
        \param args  Arguments */
    void compileFCValue(BytecodeObject& bco, const CompilationContext& cc, uint8_t which, const std::vector<const Node*>& args)
    {
        switch (which) {
         case FC_Generic:
            args[2]->compileValue(bco, cc);
            break;
         case FC_Ship:
         case FC_Planet:
            bco.addInstruction(Opcode::maPush, Opcode::sNamedVariable, bco.addName("ID"));
            break;
         default:
            throw Error::internalError("Not implemented");
        }
    }
}

/************************* IfFunctionCallNode *************************/

void
IfFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    BytecodeObject::Label_t ift = bco.makeLabel();
    BytecodeObject::Label_t iff = bco.makeLabel();
    BytecodeObject::Label_t end = bco.makeLabel();
    m_args[0]->compileCondition(bco, cc, ift, iff);
    bco.addLabel(ift);
    m_args[1]->compileValue(bco, cc);
    bco.addJump(Opcode::jAlways, end);
    bco.addLabel(iff);
    if (m_args.size() > 2) {
        m_args[2]->compileValue(bco, cc);
    } else {
        bco.addPushLiteral(0);
    }
    bco.addLabel(end);
}

void
FoldFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    if (m_args.size() == 0) {
        // Special case (does not appear)
        bco.addPushLiteral(0);
    } else {
        // Handle case sensitivity
        uint8_t m = m_minor;
        if ((m_minor == interpreter::biMin || m_minor == interpreter::biMax) && cc.hasFlag(CompilationContext::CaseBlind)) {
            ++m;
        }

        // First arg
        m_args[0]->compileValue(bco, cc);

        // Remaining args
        for (uint32_t i = 1; i < m_args.size(); ++i) {
            m_args[i]->compileValue(bco, cc);
            bco.addInstruction(Opcode::maBinary, m, 0);
        }

        // Type check args. This makes sure that "BitAnd('foo')" fails.
        if (m_args.size() == 1 && (m_minor == interpreter::biBitAnd || m_minor == interpreter::biBitOr || m_minor == interpreter::biBitXor)) {
            bco.addInstruction(Opcode::maPush,   Opcode::sInteger,     0);
            bco.addInstruction(Opcode::maBinary, interpreter::biBitOr, 0);
        }
    }
}

void
BuiltinFunctionNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    if (m_args.size() == 1) {
        m_args[0]->compileValue(bco, cc);
        bco.addInstruction(Opcode::maUnary, m_minor, 0);
    } else if (m_args.size() == 2) {
        m_args[0]->compileValue(bco, cc);
        m_args[1]->compileValue(bco, cc);
        bco.addInstruction(Opcode::maBinary, m_minor, 0);
    } else if (m_args.size() == 0) {
        bco.addInstruction(Opcode::maSpecial, m_minor, 0);
    } else {
        assert(0);
    }
}

void
CaseFunctionNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    assert(m_args.size() == 2);
    if (m_minor == interpreter::biFirstStr || m_minor == interpreter::biRestStr) {
        /* Whereas "First" and "Rest" have the search string first, "Find" has it second.
           Until 1.99.19, "First" and "Rest" had the arguments swapped by accident, which
           is reflected in the bytecode. The easiest fix is swapping them back here. */
        m_args[1]->compileValue(bco, cc);
        m_args[0]->compileValue(bco, cc);
    } else {
        m_args[0]->compileValue(bco, cc);
        m_args[1]->compileValue(bco, cc);
    }
    bco.addInstruction(Opcode::maBinary,
                       cc.hasFlag(CompilationContext::CaseBlind) ? uint8_t(m_minor+1) : m_minor,
                       0);
}

void
FindFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex ccexpr.pas:find_func (sort-of)
    //     <array>
    //     firstindex
    //     jfep 2F
    // 1H: <expr>
    //     jtp 3F
    //     nextindex
    //     jt 1B
    // 2H: pushe
    //     j 4F
    // 3H: <result>
    //     endindex
    // 4H:
    BytecodeObject::Label_t loop     = bco.makeLabel();
    BytecodeObject::Label_t ift      = bco.makeLabel();
    BytecodeObject::Label_t iff      = bco.makeLabel();
    BytecodeObject::Label_t notfound = bco.makeLabel();
    BytecodeObject::Label_t end      = bco.makeLabel();

    CompilationContext ncc = cc;
    ncc.withoutFlag(CompilationContext::LocalContext);

    compileFCArray(bco, ncc, m_which, m_args);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, notfound);
    bco.addLabel(loop);
    compileFCCondition(bco, ncc, ift, iff, m_which, m_args);
    bco.addLabel(iff);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
    bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, loop);
    bco.addLabel(notfound);
    bco.addPushLiteral(0);
    bco.addJump(Opcode::jAlways, end);
    bco.addLabel(ift);
    if (m_which == FC_Generic && dynamic_cast<const LiteralNode*>(m_args[2]) != 0) {
        // Find(whatever, const): generate the constant after the endindex command, so it can merge with a possible following operation
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
        m_args[2]->compileValue(bco, cc);
    } else {
        // Standard mode
        compileFCValue(bco, ncc, m_which, m_args);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
    }
    bco.addLabel(end);
}

void
CountFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex ccexpr.pas:count_func (sort-of)
    //     pushint 0
    //     <array>
    //     firstindex
    //     jfep 3F
    // 1H: <expr>
    //     jfep 2F
    //     pushint 1
    //     badd
    // 2H: nextindex
    //     jtp 1B
    // 3H:
    BytecodeObject::Label_t loop     = bco.makeLabel();
    BytecodeObject::Label_t end      = bco.makeLabel();
    BytecodeObject::Label_t ift      = bco.makeLabel();
    BytecodeObject::Label_t iff      = bco.makeLabel();

    CompilationContext ncc = cc;
    ncc.withoutFlag(CompilationContext::LocalContext);

    bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
    compileFCArray(bco, cc, m_which, m_args);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
    bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, end);
    bco.addLabel(loop);
    if (m_which != FC_Generic || m_args.size() > 1) {
        compileFCCondition(bco, ncc, ift, iff, m_which, m_args);
        bco.addLabel(ift);
    }
    bco.addInstruction(Opcode::maUnary, interpreter::unInc, 0);
    bco.addLabel(iff);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
    bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, loop);
    bco.addLabel(end);
}

void
OneTwoFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    m_args[0]->compileValue(bco, cc);
    if (m_args.size() < 2) {
        // Special case for unary Str():
        if (m_minor == interpreter::biStr) {
            bco.addInstruction(Opcode::maUnary, interpreter::unStr, 0);
            return;
        }

        // Others have a second argument
        if (m_minor == interpreter::biStrMult) {
            // String(...). Does it make more sense to generate 'pushint 32, uchr'?
            afl::data::StringValue sv(" ");
            bco.addPushLiteral(&sv);
        } else {
            // ATan, Dim
            bco.addInstruction(Opcode::maPush, Opcode::sInteger, 1);
        }
    } else {
        // Second argument
        m_args[1]->compileValue(bco, cc);
    }
    bco.addInstruction(Opcode::maBinary, m_minor, 0);
}

void
MidFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    m_args[0]->compileValue(bco, cc);
    m_args[1]->compileValue(bco, cc);
    bco.addInstruction(Opcode::maBinary, interpreter::biLCut, 0);
    if (m_args.size() > 2) {
        m_args[2]->compileValue(bco, cc);
        bco.addInstruction(Opcode::maBinary, interpreter::biRCut, 0);
    }
}

void
StrCaseFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    CompilationContext ncc = cc;
    ncc.withoutFlag(CompilationContext::CaseBlind);
    m_args[0]->compileValue(bco, ncc);
}

void
KeyFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    // ex ccexpr.pas:op_KEY_func
    // Push keymap
    if (const IdentifierNode* id = dynamic_cast<const IdentifierNode*>(m_args[0])) {
        // Keymap literal (classic)
        afl::data::StringValue keymap(id->getIdentifier());
        bco.addPushLiteral(&keymap);
    } else if (const ByNameFunctionCallNode* bcn = dynamic_cast<const ByNameFunctionCallNode*>(m_args[0])) {
        // ByName(xx) syntax
        bcn->compileName(bco, cc);
    } else {
        throw Error::typeError(Error::ExpectKeymap);
    }
    bco.addInstruction(Opcode::maUnary, interpreter::unKeyLookup, 0);

    // Evaluate key
    m_args[1]->compileValue(bco, cc);

    bco.addInstruction(Opcode::maBinary, interpreter::biKeyFind, 0);
}

void
EvalFunctionCallNode::compileValue(BytecodeObject& bco, const CompilationContext& cc) const
{
    if (m_args.size() == 1) {
        // Eval(x):
        //   <x>
        //   sevalx
        m_args[0]->compileValue(bco, cc);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
    } else {
        // Eval(x, obj):
        //   <obj>
        //   je null
        //   <x>
        //   swap 1F
        //   swith <obj>
        //   sevalx
        //   sendwith
        // 1:
        BytecodeObject::Label_t skip = bco.makeLabel();
        m_args[1]->compileValue(bco, cc);
        bco.addJump(Opcode::jIfEmpty, skip);

        CompilationContext ncc = cc;
        ncc.withoutFlag(CompilationContext::LocalContext);
        m_args[0]->compileValue(bco, cc);

        bco.addInstruction(Opcode::maStack, Opcode::miStackSwap, 1);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalExpr, 0);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        bco.addLabel(skip);
    }
}

/*************************** External Interface **************************/

namespace {
    FunctionCallNode* makeIf(const BuiltinFunctionDescriptor&)
    {
        return new IfFunctionCallNode();
    }

    FunctionCallNode* makeBuiltin(const BuiltinFunctionDescriptor& desc)
    {
        return new BuiltinFunctionNode(desc.generator_arg);
    }

    FunctionCallNode* makeCase(const BuiltinFunctionDescriptor& desc)
    {
        return new CaseFunctionNode(desc.generator_arg);
    }

    FunctionCallNode* makeFold(const BuiltinFunctionDescriptor& desc)
    {
        return new FoldFunctionCallNode(desc.generator_arg);
    }

    FunctionCallNode* makeFind(const BuiltinFunctionDescriptor& desc)
    {
        return new FindFunctionCallNode(desc.generator_arg);
    }

    FunctionCallNode* makeCount(const BuiltinFunctionDescriptor& desc)
    {
        return new CountFunctionCallNode(desc.generator_arg);
    }

    FunctionCallNode* makeOneTwo(const BuiltinFunctionDescriptor& desc)
    {
        return new OneTwoFunctionCallNode(desc.generator_arg);
    }

    FunctionCallNode* makeMid(const BuiltinFunctionDescriptor&)
    {
        return new MidFunctionCallNode();
    }

    FunctionCallNode* makeStrCase(const BuiltinFunctionDescriptor&)
    {
        return new StrCaseFunctionCallNode();
    }

    FunctionCallNode* makeKey(const BuiltinFunctionDescriptor&)
    {
        return new KeyFunctionCallNode();
    }

    FunctionCallNode* makeByName(const BuiltinFunctionDescriptor&)
    {
        return new ByNameFunctionCallNode();
    }

    FunctionCallNode* makeEval(const BuiltinFunctionDescriptor&)
    {
        return new EvalFunctionCallNode();
    }

    const BuiltinFunctionDescriptor builtin_functions[] = {
        /* @q Abs(x:Num):Num (Elementary Function)
           Returns the absolute value of its argument.
           If the argument is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3 */
        { "ABS",         1, 1,       makeBuiltin, interpreter::unAbs },

        /* @q Asc(s:Str):Int (Elementary Function)
           Returns the character code of the first character of the string given as parameter.
           If %s is not a string, it is converted into one before being examined.
           If %s is EMPTY or an empty string, returns EMPTY.

           In PCC2 since 1.99.12, this function returns the Unicode value of the character,
           which can be an (almost) arbitrary non-negative integer.
           In previous versions, it returns the code in the extended ASCII set used as the game character set,
           which is in the range 0..255.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "ASC",         1, 1,       makeBuiltin, interpreter::unAsc },

        /* @q ATan(x:Num, Optional y:Num):Num (Elementary Function)
           Returns the arc-tangent of %x resp. %x/%y.
           The angle is returned in degrees (not radians as many other programming languages),
           and is in the range [0,360).

           A heading angle is computed as
           <pre class="ccscript">
             angle := ATan(Waypoint.DX, Waypoint.DY)
           </pre>
           This form is better than
           <pre class="ccscript">
             angle := ATan(Waypoint.DX / Waypoint.DY)
           </pre>
           because it gets the quadrant right, and does not divide by zero if %Waypoint.DX is 0.

           If any parameter is EMPTY, or if both parameters are 0, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3 */
        { "ATAN",        1, 2,       makeOneTwo,  interpreter::biATan },

        /* @q Atom(s:Str):Int (Elementary Function)
           Creates an atom from a string.
           An atom is a number that can be mapped back to the string using AtomStr().
           Calling Atom() again with the same string will return the same value.
           However, the mapping is not necessarily identical in different sessions.

           The empty string "" always maps to the atom 0.
           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.12 */
        { "ATOM",        1, 1,       makeBuiltin, interpreter::unAtom },

        /* @q AtomStr(n:Int):Str (Elementary Function)
           Returns the string associated with an atom.
           This is the same string that was passed to Atom() when it returned %n.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.12 */
        { "ATOMSTR",     1, 1,       makeBuiltin, interpreter::unAtomStr },

        /* @q BitAnd(n:Int...):Int (Elementary Function)
           Returns the bitwise AND of all its parameters.
           All parameters must be integers; if one parameter is EMPTY, the result is EMPTY.

           @diff Whereas PCC2 allows any number of parameters, PCC 1.x has a limit of 6.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.1.17 */
        { "BITAND",      1, INT_MAX, makeFold,    interpreter::biBitAnd },

        /* @q BitNot(n:Int):Int (Elementary Function)
           Returns the bitwise negation of its parameter.
           If the parameter is EMPTY, the result is EMPTY.
           @since PCC2 1.99.8, PCC 1.1.17 */
        { "BITNOT",      1, 1,       makeBuiltin, interpreter::unBitNot },

        /* @q BitOr(n:Int...):Int (Elementary Function)
           Returns the bitwise OR of all its parameters.
           All parameters must be integers; if one parameter is EMPTY, the result is EMPTY.

           @diff Whereas PCC2 allows any number of parameters, PCC 1.x has a limit of 6.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.1.17 */
        { "BITOR",       1, INT_MAX, makeFold,    interpreter::biBitOr },

        /* @q BitXor(n:Int...):Int (Elementary Function)
           Returns the bitwise XOR of all its parameters.
           All parameters must be integers; if one parameter is EMPTY, the result is EMPTY.

           @diff Whereas PCC2 allows any number of parameters, PCC 1.x has a limit of 6.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.1.17 */
        { "BITXOR",      1, INT_MAX, makeFold,    interpreter::biBitXor },

        // BYNAME is a placeholder to
        //   (a) permit the implementation of the KEY() function. Unlike the keymap/hook commands
        //       that parse the identifier-or-byname syntax themselves (StatementCompiler::compileNameString),
        //       KEY() will receive a fully-parsed expression tree and dissect that.
        //       Instead of distinguishing between token sequences, it will look at the shape of the expression tree.
        //   (b) refuse BYNAME at places where it does not belong
        // BYNAME is documented in statementcompiler.cpp.
        { "BYNAME",      1, 1,       makeByName,  0 },

        { "CC$TRACE",    1, 1,       makeBuiltin, interpreter::unTrace },

        /* @q Chr(n:Int):Str (Elementary Function), Chr$(n:Int):Str (Elementary Function)
           Returns a character, given the character code.
           For example, <tt>Chr(65)</tt> returns "A", and <tt>Chr(8745)</tt> returns "&#8745;".

           In PCC2 since 1.99.12, this returns the string containing Unicode character %n.
           In versions before 1.99.12, and in PCC 1.x, this function returns a character from
           the extended ASCII set used as game character set.

           PCC2 supports the WGL4 character set for display and thus supports most European
           languages including Greek and Russian. You can place other characters in strings, but
           PCC2 will not be able to display them.

           PCC 1.x's character repertoire depends on the font used; the default is codepage 437,
           but fonts in codepage 866 (cyrillic) exist in the CCFonts package.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "CHR",         1, 1,       makeBuiltin, interpreter::unChr },
        { "CHR$",        1, 1,       makeBuiltin, interpreter::unChr },

        /* @q Cos(x:Num):Num (Elementary Function)
           Compute the cosine of an angle.
           The angle %x is specified in degrees (not radians as many other programming languages).
           The result is a value between -1 and +1.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3
           @see Sin, Tan */
        { "COS",         1, 1,       makeBuiltin, interpreter::unCos },

        /* @q Count(a:Array, Optional q:Expr):Int (Elementary Function)
           Count number of objects in an array.
           %a must be an array of objects (such as a builtin object array like %Ship or %Planet).
           The expression %q is evaluated for each object, as if within a %ForEach loop,
           and the object is counted if it returns true.
           If %q is not specified, all objects are counted.
           @since PCC2 2.40, PCC2 1.99.9 */
        { "COUNT",       1, 2,       makeCount,   FC_Generic },

        /* @q CountPlanets(q:Expr):Int (Elementary Function)
           Count number of planets satisfying a condition.
           The expression %q is evaluated for each planet, and the planet is counted if it returns true.

           This function is (almost) equivalent to <tt>Count(Planet, q)</tt>.
           @since PCC2 2.40, PCC2 1.99.9, PCC 1.0.11 */
        { "COUNTPLANETS",1, 1,       makeCount,   FC_Planet },

        /* @q CountShips(q:Expr):Int (Elementary Function)
           Count number of ships satisfying a condition.
           The expression %q is evaluated for each ship, and the ship is counted if it returns true.

           This function is (almost) equivalent to <tt>Count(Ship, q)</tt>.
           @since PCC2 2.40, PCC2 1.99.9, PCC 1.0.11 */
        { "COUNTSHIPS",  1, 1,       makeCount,   FC_Ship },

        /* @q Dim(a:Array, Optional d:Int):Int (Elementary Function)
           Get size of an array.
           Returns the number of elements in the %d'th dimension of array %a.
           %d starts from 1, that is, for a two-dimensional array, you can pass 1 or 2 here.

           The return value is the number of elements in the array's dimension.
           The highest possible index into the array is one less than the value returned.

           For example,
           <pre class="ccscript">
             Dim a(10)
             Print Dim(a)                  % returns 10
             Print Dim(a,1)                % also returns 10
             For i:=0 To Dim(a)-1 Do ...   % iterates
           </pre>

           If any parameter is EMPTY, returns EMPTY.

           Since 1.99.22, this function also works for builtin arrays such as {Ship()}.
           Note that builtin arrays often don't have a zero element (i.e. there is no
           <tt>Ship(0)</tt>). For iteration through ships, you would therefore use
           | For i:=1 To Dim(Ships) Do ...
           Better, however, is to use {ForEach}.

           @since PCC2 2.40, PCC2 1.99.12
           @see IsArray (Elementary Function), Dim (Elementary Command) */
        { "DIM",         1, 2,       makeOneTwo,  interpreter::biArrayDim },

        /* @q Eval(s:Str, Optional c:Obj):Any (Elementary Function)
           Evaluate an expression given as string.
           For example, <tt>Eval("2+2")</tt> returns 4.

           If the second parameter is specified, the expression is evaluated in that context.

           If any parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.9
           @change The two-argument form is supported since PCC2 2.40.6.
           @see Eval (Elementary Command) */
        { "EVAL",        1, 2,       makeEval,    0 },

        /* @q Exp(n:Num):Num (Elementary Function)
           Exponential function.
           Computes e^n, where e is Euler's number.
           This is the inverse to the %Log function.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8 */
        { "EXP",         1, 1,       makeBuiltin, interpreter::unExp },

        /* @q Find(a:Array, q:Expr, v:Expr):Any (Elementary Function)
           Find element in an array.
           %a must be an array of objects (such as a builtin object array like %Ship or %Planet).
           The expression %q is evaluated for each object, as if within a %ForEach loop.
           If it returns true, the function returns %v evaluated in that object's context.
           If no object matches, the return value is EMPTY.
           @since PCC2 2.40, PCC2 1.99.9
           @see FindShip, FindPlanet */
        { "FIND",        3, 3,       makeFind,    FC_Generic },

        /* @q FindPlanet(q:Expr):Int (Elementary Function)
           Find planet.
           The expression %q is evaluated for each planet, as if within a %ForEach loop.
           If it returns true, the function returns that planet's Id.
           If no planet matches, the return value is EMPTY.

           This function is (almost) equivalent to <tt>Find(Planet, q, Id)</tt>.
           @since PCC2 2.40, PCC2 1.99.9, PCC 1.0.11
           @see Find */
        { "FINDPLANET",  1, 1,       makeFind,    FC_Planet },

        /* @q FindShip(q:Expr):Int (Elementary Function)
           Find ship.
           The expression %q is evaluated for each ship, as if within a %ForEach loop.
           If it returns true, the function returns that ship's Id.
           If no ship matches, the return value is EMPTY.

           This function is (almost) equivalent to <tt>Find(Ship, q, Id)</tt>.
           @since PCC2 2.40, PCC2 1.99.9, PCC 1.0.11
           @see Find */
        { "FINDSHIP",    1, 1,       makeFind,    FC_Ship },

        /* @q First(delim:Str, list:Str):Str (Elementary Function)
           Split string, return first part.

           Assuming that %list is a string containing multiple fields, separated by %delim,
           this function returns the first field. For example,
           <pre class="ccscript">
             First(",", "cln,-57,Clone a ship")
           </pre>
           returns "cln". If the string does not contain the delimiter, it is returned as-is:
           <pre class="ccscript">
             First(",", "huh?")
           </pre>
           returns "huh?".

           Note that, by default, substring search is case-insensitive.
           Use %StrCase to search case-sensitive.

           If any parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.17 */
        { "FIRST",       2, 2,       makeCase,    interpreter::biFirstStr },

        /* @q If(cond:Bool, yes:Expr, Optional no:Expr):Any (Elementary Function)
           Conditional evaluation.
           If the condition %cond evaluates to true, evaluates %yes and returns its value.
           Otherwise, if the condition is false or EMPTY, evaluates %no and returns its value
           (and if %no is not specified, just returns EMPTY).

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "IF",          2, 3,       makeIf,      0 },

        /* @q InStr(haystack:Str, needle:Str):Str (Elementary Function)
           Find substring.
           Locates the first occurrence of %needle in %haystack.
           It returns the position of that string as an integer, where 1 means the first position.
           If there is no match, returns 0.

           Note that, by default, substring search is case-insensitive.
           Use %StrCase to search case-sensitive.

           Examples:
           <pre class="ccscript">
             InStr("frob","o") = 3
             InStr("frob","x") = 0
           </pre>

           If any parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99.2 */
        { "INSTR",       2, 2,       makeCase,    interpreter::biFindStr },

        /* @q Int(n:Num):Int (Elementary Function)
           Convert to integer.
           If the parameter is a floating-point (fractional) number,
           truncates its fractional digits and converts it into an integer.
           If the parameter already is an integer, it is returned as is.

           Examples:
           <pre class="ccscript">
             Int(2.5) = 2
             Int(-2.1) = -2
           </pre>

           If the parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3
           @see Round */
        { "INT",         1, 1,       makeBuiltin, interpreter::unTrunc },

        /* @q IsArray(a:Any):Int (Elementary Function)
           Check for array.
           If the parameter refers to an array, returns the number of dimensions.
           If the parameter is another non-EMPTY value, returns 0.
           If the parameter is EMPTY, this function returns EMPTY.

           Since every array has at least one dimension,
           this function can be used as if it returns a truth value if required:
           <pre class="ccscript">
             If IsArray(a) Then Print "This is an array!"
           </pre>

           Since 1.99.22, this function also works for builtin arrays such as {Ship()}.

           @see Dim (Elementary Function)
           @since PCC2 2.40, PCC2 1.99.12 */
        { "ISARRAY",     1, 1,       makeBuiltin, interpreter::unIsArray },

        /* @q IsEmpty(a:Any):Bool (Elementary Function)
           Check for EMPTY.
           If the parameter is EMPTY, returns %True.
           Otherwise, returns %False.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3 */
        { "ISEMPTY",     1, 1,       makeBuiltin, interpreter::unIsEmpty },

        /* @q IsNum(a:Any):Bool (Elementary Function)
           Check for number.
           If the parameter is a number, returns True.
           Otherwise, returns False.

           @diff PCC 1.x returns False for Booleans.
           PCC2 returns True, since a Boolean can be used wherever a number is required.

           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.14 */
        { "ISNUM",       1, 1,       makeBuiltin, interpreter::unIsNum },

        /* @q IsString(a:Any):Bool (Elementary Function)
           Check for number.
           If the parameter is a string, returns True.
           Otherwise, returns False.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.14 */
        { "ISSTRING",    1, 1,       makeBuiltin, interpreter::unIsString },

        /* @q Key(k:Keymap, key:Str):Int (Elementary Function)
           Look up key in keymap.
           The keymap is specified as the keymap name, the key is a string, as in
           <pre class="ccscript">
           Key(Global, "Alt-C")
           </pre>

           If the key is bound in the keymap, returns its numeric command code.
           This usually is an atom that can be converted back into a command using %AtomStr.

           If the key is not bound in the keymap directly, its parent keymaps will be consulted.
           If the key cannot be found in those as well, the return value is EMPTY.

           @diff It is an error in PCC2 if the keymap does not exist.
           PCC 1.x just returns EMPTY in this case.

           @since PCC2 2.40, PCC2 1.99.9, PCC 1.1.10
           @see Bind */
        { "KEY",         2, 2,       makeKey,     0 },

        /* @q LCase(s:Str):Str (Elementary Function)
           Convert string to lower case.
           Returns the string with all ASCII characters converted to lower-case.

           If the parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40.8, PCC2 2.0.8
           @see UCase */
        { "LCASE",       1, 1,       makeBuiltin, interpreter::unLCase },

        /* @q Left(s:Str, n:Int):Str (Elementary Function)
           Get initial (left) part of a string.
           Returns the first %n characters of string %s.

           If any parameter is EMPTY, returns EMPTY.
           If %n is negative, returns an empty string.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99.2 */
        { "LEFT",        2, 2,       makeBuiltin, interpreter::biRCut },

        /* @q Len(s:Str):Int (Elementary Function)
           Get length of string.
           Returns the number of characters within the string.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "LEN",         1, 1,       makeBuiltin, interpreter::unLength },

        /* @q Log(n:Num):Num (Elementary Function)
           Natural logarithm.
           Computes the logarithm to base e, where e is Euler's number.
           The parameter must be a strictly positive number.
           This is the inverse to the %Exp function.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8 */
        { "LOG",         1, 1,       makeBuiltin, interpreter::unLog },

        /* @q LTrim(s:Str):Str (Elementary Function)
           Trim leading (left) whitespace.
           Returns the string %s with all leading space and tab characters removed.

           If the parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99
           @see Trim, RTrim */
        { "LTRIM",       1, 1,       makeBuiltin, interpreter::unLTrim },

        /* @q Max(a:Any...):Any (Elementary Function)
           Maximum.
           Compares all arguments, which must all be numbers, or all strings, and returns the maximum.
           If any argument is EMPTY, returns EMPTY.

           Note that, by default, string comparison is case-insensitive.
           Use %StrCase to compare case-sensitive.

           @diff Whereas PCC2 allows any number of parameters, PCC 1.x has a limit of 6.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.7
           @see StrCase, Min */
        { "MAX",         1, INT_MAX, makeFold,    interpreter::biMax },

        /* @q Mid(s:Str, pos:Int, Optional count:Int):Str (Elementary Function)
           Substring extraction.
           Returns %count characters from string %s starting at position %pos.
           If %count is not specified, returns all characters from that position.

           If any parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99.2
           @see RMid, Left, Right */
        { "MID",         2, 3,       makeMid,     0 },

        /* @q Min(a:Any...):Any (Elementary Function)
           Minimum.
           Compares all arguments, which must all be numbers, or all strings, and returns the minimum.
           If any argument is EMPTY, returns EMPTY.

           Note that, by default, string comparison is case-insensitive.
           Use %StrCase to compare case-sensitive.

           @diff Whereas PCC2 allows any number of parameters, PCC 1.x has a limit of 6.
           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.7
           @see StrCase, Max */
        { "MIN",         1, INT_MAX, makeFold,    interpreter::biMin },

        /* @q NewHash():Hash (Elementary Function)
           Create hash.
           Allocates a new hash and returns it.

           Normally, hashes are created using {Dim|Dim ... As Hash},
           but this function remains available as a shortcut.

           @since PCC2 2.40, PCC2 1.99.15 */
        { "NEWHASH",     0, 0,       makeBuiltin, Opcode::miSpecialNewHash },

        /* @q Rest(delim:Str, list:Str):Str (Elementary Function)
           Split string, return remainder.

           Assuming that %list is a string containing multiple fields, separated by %delim,
           this function returns everything but the first field. For example,
           <pre class="ccscript">
             Rest(",", "cln,-57,Clone a ship")
           </pre>
           returns "-57,Clone a ship".
           If the string does not contain the delimiter, this function returns EMPTY.
           <pre class="ccscript">
             Rest(",", "huh?")
           </pre>

           Note that, by default, substring search is case-insensitive.
           Use %StrCase to search case-sensitive.

           If any parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.17
           @see First */
        { "REST",        2, 2,       makeCase,    interpreter::biRestStr },

        /* @q Right(s:Str, n:Int):Str (Elementary Function)
           Get ending (right) part of a string.
           Returns the last %n characters of string %s.

           If any parameter is EMPTY, returns EMPTY.
           If %n is negative, returns an empty string.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99.2 */
        { "RIGHT",       2, 2,       makeBuiltin, interpreter::biEndCut },

        /* @q Round(n:Num):Int (Elementary Function)
           Round to integer.
           If the parameter is a floating-point (fractional) number,
           it is rounded using the usual arithmetic rules: .5 or higher rounds up towards infinity,
           below rounds down towards 0.
           If the parameter already is an integer, it is returned as is.

           Examples:
           <pre class="ccscript">
             Round(2.5) = 3
             Round(-2.5) = -3
           </pre>

           If the parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3
           @see Int */
        { "ROUND",       1, 1,       makeBuiltin, interpreter::unRound },

        /* @q RTrim(s:Str):Str (Elementary Function)
           Trim trailing (right) whitespace.
           Returns the string %s with all trailing space and tab characters removed.

           If the parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99
           @see Trim, LTrim */
        { "RTRIM",       1, 1,       makeBuiltin, interpreter::unRTrim },

        /* @q Sin(x:Num):Num (Elementary Function)
           Compute the sine of an angle.
           The angle %x is specified in degrees (not radians as many other programming languages).
           The result is a value between -1 and +1.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3
           @see Cos, Tan */
        { "SIN",         1, 1,       makeBuiltin, interpreter::unSin },

        /* @q Sqr(x:Num):Num (Elementary Function), Sqrt(x:Num):Num (Elementary Function)
           Square root.
           Returns the square root of its argument,
           i.e. a number that, when multiplied by itself, returns the argument again.
           Square roots are defined for non-negative values only.

           If the parameter is EMPTY, returns EMPTY.

           This function can be used to compute distances using the Pythagorean theorem:
           <pre class="ccscript">
             dist := Sqrt(xDisplacement^2 + yDisplacement^2)
           </pre>
           Note that PCC also offers a %Distance function.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3 */
        { "SQR",         1, 1,       makeBuiltin, interpreter::unSqrt },
        { "SQRT",        1, 1,       makeBuiltin, interpreter::unSqrt },

        /* @q Str(x:Any, Optional precision:Int):Str (Elementary Function)
           Convert to string.
           Returns a string containing a human-readable representation of %x.
           If the %precision argument is specified, it defines the number of fractional decimal places
           to use for numbers. If it is not specified, the same conversion as for the %Print command
           or the "&" operator is used.

           If any parameter is EMPTY, this function returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3 */
        { "STR",         1, 2,       makeOneTwo,  interpreter::biStr },

        /* @q StrCase(x:Expr):Any (Elementary Function)
           Case-sensitive evaluation.
           By default, string comparisons and substring searches are case-insensitive.
           The %StrCase function causes the expression %x to be evaluated in case-sensitive mode.
           For example,
           <pre class="ccscript">
             "a" = "A"           % True
             StrCase("a" = "A")  % False
           </pre>

           Note that case-sensitivity only applies to operations that happen directly in the expression %x.
           If %x calls a user-defined function, that function's body operates case-insensitive again.

           @since PCC2 2.40, PCC2 1.99.8, PCC 1.0.4 */
        { "STRCASE",     1, 1,       makeStrCase, 0 },

        /* @q String(n:Int, Optional s:Str):Str (Elementary Function), String$(n:Int, Optional s:Str):Str (Elementary Function)
           Replicate string.
           Returns a string that contains %n copies of %s.
           If %s is not specified, returns a string containing %n spaces.

           If any parameter is EMPTY, this function returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "STRING",      1, 2,       makeOneTwo,  interpreter::biStrMult },
        { "STRING$",     1, 2,       makeOneTwo,  interpreter::biStrMult },

        /* @q Tan(x:Num):Num (Elementary Function)
           Compute the tangent of an angle.
           The angle %x is specified in degrees (not radians as many other programming languages).
           The result is a value between -1 and +1.

           The tangent of 90&#176; or 270&#176; cannot be computed and produces an error.

           If the parameter is EMPTY, returns EMPTY.
           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.3
           @see Sin, Cos */
        { "TAN",         1, 1,       makeBuiltin, interpreter::unTan },

        /* @q Trim(s:Str):Str (Elementary Function)
           Trim whitespace.
           Returns the string %s with all leading and trailing space and tab characters removed.

           If the parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99
           @see LTrim, RTrim */
        { "TRIM",        1, 1,       makeBuiltin, interpreter::unLRTrim },

        /* @q UCase(s:Str):Str (Elementary Function)
           Convert string to upper case.
           Returns the string with all ASCII characters converted to lower-case.

           If the parameter is EMPTY, returns EMPTY.

           @since PCC2 2.40.8, PCC2 2.0.8
           @see LCase */
        { "UCASE",       1, 1,       makeBuiltin, interpreter::unUCase },

        /* @q Val(s:Str):Num (Elementary Function)
           Convert string to number.
           Attempts to interpret the string as a number, and returns that.
           If the string does not look like a number, returns EMPTY (leading and trailing whitespace is OK, though).

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.99.6 */
        { "VAL",         1, 1,       makeBuiltin, interpreter::unVal },

        /* @q Z(x:Any):Any (Elementary Function), Zap(x:Any):Any (Elementary Function)
           Force false expression to EMPTY.
           If the parameter is an empty string, False, or zero, returns EMPTY.
           Otherwise, the parameter is returned as-is.
           The idea is to make zero/empty values disappear in messages, e.g.
           <pre class="ccscript">
             Z(Money) # ' mc'
           </pre>
           will return a string like "10 mc" if there is some money, but disappear if there's none.

           @since PCC2 2.40, PCC2 1.99.8, PCC 0.98.5 */
        { "Z",           1, 1,       makeBuiltin, interpreter::unZap },
        { "ZAP",         1, 1,       makeBuiltin, interpreter::unZap },
    };
}

const BuiltinFunctionDescriptor*
interpreter::expr::lookupBuiltinFunction(const String_t& name)
{
    /* Quick & Dirty. */
    for (size_t i = 0; i < countof(builtin_functions); ++i) {
        if (name == builtin_functions[i].name) {
            return &builtin_functions[i];
        }
    }
    return 0;
}
