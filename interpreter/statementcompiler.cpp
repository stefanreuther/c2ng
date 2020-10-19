/**
  *  \file interpreter/statementcompiler.cpp
  *  \brief Class interpreter::StatementCompiler
  */

#include <cstring>
#include "interpreter/statementcompiler.hpp"
#include "afl/data/floatvalue.hpp"
#include "afl/data/stringvalue.hpp"
#include "afl/string/format.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"
#include "interpreter/commandsource.hpp"
#include "interpreter/context.hpp"
#include "interpreter/contextprovider.hpp"
#include "interpreter/defaultstatementcompilationcontext.hpp"
#include "interpreter/error.hpp"
#include "interpreter/expr/builtinfunction.hpp"
#include "interpreter/expr/casenode.hpp"
#include "interpreter/expr/literalnode.hpp"
#include "interpreter/expr/parser.hpp"
#include "interpreter/expr/simplenode.hpp"
#include "interpreter/keywords.hpp"
#include "interpreter/optimizer.hpp"
#include "interpreter/selectionexpression.hpp"
#include "interpreter/specialcommand.hpp"
#include "interpreter/statementcompilationcontext.hpp"
#include "interpreter/structuretype.hpp"
#include "interpreter/subroutinevalue.hpp"
#include "interpreter/values.hpp"
#include "interpreter/world.hpp"
#include "util/charsetfactory.hpp"

namespace {
    using interpreter::CompilationContext;
    using interpreter::Error;
    using interpreter::StatementCompilationContext;
    using interpreter::Tokenizer;

    enum TypeKeyword {
        tkNone,
        tkAny,
        tkInteger,
        tkFloat,
        tkString,
        tkHash
    };

    TypeKeyword identifyType(const String_t& s)
    {
        if (s == "ANY") {
            return tkAny;
        } else if (s == "INTEGER" || s == "LONG") {
            return tkInteger;
        } else if (s == "FLOAT" || s == "SINGLE" || s == "DOUBLE") {
            return tkFloat;
        } else if (s == "STRING") {
            return tkString;
        } else if (s == "HASH") {
            return tkHash;
        } else {
            return tkNone;
        }
    }

    /** Strip prefix from an identifier.
        \param s   [in] Identifier given by user
        \param pfx [in] Prefix
        \return adjusted string */
    String_t stripPrefix(const String_t& s, const char* pfx)
    {
        size_t pfxlen = std::strlen(pfx);
        if (s.compare(0, pfxlen, pfx, pfxlen) == 0) {
            if (s.size() == pfxlen) {
                // Cannot happen, "SHIP." tokenizes as "SHIP" + ".", not an identifier
                throw Error("Invalid identifier");
            }
            return String_t(s, pfxlen);
        }
        return s;
    }

    /** Ensure we are allowed to execute multiline commands. */
    void validateMultiline(const StatementCompilationContext& cc)
    {
        if (cc.hasFlag(CompilationContext::RefuseBlocks)) {
            throw Error::invalidMultiline();
        }
    }

    /** Check for a next element separated by comma. */
    bool parseNext(Tokenizer& tok)
    {
        if (tok.checkAdvance(Tokenizer::tComma)) {
            return true;
        } else if (tok.getCurrentToken() == Tokenizer::tEnd) {
            return false;
        } else {
            throw Error::expectSymbol(",");
        }
    }

    /** Parse argument to an "Option" command.
        It can be either absent (equivalent to 1), or a number or boolean literal.
        \param tok Tokenizer to read from
        \param min,max Range to accept
        \return value */
    int parseOptionArgument(interpreter::Tokenizer& tok, int min, int max)
    {
        if (tok.checkAdvance(tok.tLParen)) {
            /* Parentheses given. Read sign. */
            bool negate = false;
            if (tok.checkAdvance(tok.tPlus)) {
                // ok
            } else if (tok.checkAdvance(tok.tMinus)) {
                negate = true;
            } else {
                // nothing
            }

            /* Read number. */
            if (tok.getCurrentToken() != tok.tInteger && tok.getCurrentToken() != tok.tBoolean) {
                throw Error("Expecting integer");
            }
            int val = tok.getCurrentInteger();
            if (negate) {
                val = -val;
            }
            if (val < min || val > max) {
                throw Error::rangeError();
            }
            tok.readNextToken();
            if (!tok.checkAdvance(tok.tRParen)) {
                throw Error::expectSymbol(")");
            }
            return val;
        } else {
            /* Default always is 1 */
            return 1;
        }
    }
}


const int interpreter::StatementCompiler::MIN_OPTIMISATION_LEVEL;
const int interpreter::StatementCompiler::MAX_OPTIMISATION_LEVEL;
const int interpreter::StatementCompiler::DEFAULT_OPTIMISATION_LEVEL;


// Constructor.
interpreter::StatementCompiler::StatementCompiler(CommandSource& cs)
    : m_commandSource(cs),
      m_allowLocalTypes(false),
      m_allowLocalSubs(false),
      m_optimisationLevel(DEFAULT_OPTIMISATION_LEVEL)
{
    // ex IntStatementCompiler::IntStatementCompiler
    cs.readNextLine();
}

// Compile a statement.
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compile(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compile

    /* End of file reached? */
    if (m_commandSource.isEOF()) {
        if (scc.hasFlag(CompilationContext::WantTerminators)) {
            /* If we want a terminator, we do not want end of file */
            throw Error("Unexpected end of script");
        } else {
            return EndOfInput;
        }
    }

    /* Remember current compilation position for debugging */
    bco.addLineNumber(m_commandSource.getLineNumber());

    /* Do we have a nice statement? */
    Tokenizer& tok = m_commandSource.tokenizer();
    if (tok.getCurrentToken() == tok.tEnd) {
        /* Blank line */
        return CompiledStatement;
    } else if (tok.getCurrentToken() == tok.tIdentifier) {
        /* Identifier */
        switch (lookupKeyword(tok.getCurrentString())) {
         case kwAbort:
            return compileAbort(bco, scc);

         case kwBind:
            return compileBind(bco, scc);

         case kwBreak:
            /* @q Break (Elementary Command)
               Exit a loop.
               This command is valid within a {Do}, {For}, or {ForEach} loop.
               It cancels the current iteration and all iterations that would follow,
               and continues execution immediately after the %Loop or %Next keyword.
               @since PCC2 1.99.9, PCC 1.0.6 */
            tok.readNextToken();
            parseEndOfLine();
            scc.compileBreak(bco);
            return CompiledStatement;

         case kwCall:
            return compileCall(bco, scc);

         case kwContinue:
            /* @q Continue (Elementary Command)
               Continue a loop.
               This command is valid within a {Do}, {For}, or {ForEach} loop.
               It cancels the current iteration and proceeds with the next one, if any.
               @since PCC2 1.99.9, PCC 1.0.6 */
            tok.readNextToken();
            parseEndOfLine();
            scc.compileContinue(bco);
            return CompiledStatement;

         case kwCreateKeymap:
            return compileCreateKeymap(bco, scc);

         case kwCreatePlanetProperty:
            return compileCreateProperty(bco, scc, Opcode::miSpecialDefPlanetProperty, "PLANET.");

         case kwCreateShipProperty:
            return compileCreateProperty(bco, scc, Opcode::miSpecialDefShipProperty, "SHIP.");

         case kwDim:
            tok.readNextToken();
            return compileDim(bco, scc);

         case kwDo:
            return compileDo(bco, scc);

         case kwEnd:
            /* @q End (Elementary Command)
               Terminate this script.
               This command normally makes no sense in regular code such as keybindings,
               but it may be useful in scripts intended to run stand-alone.
               To exit from a subroutine, use {Return}.
               @since PCC2 1.99.9, PCC 1.0.6 */
            tok.readNextToken();
            parseEndOfLine();
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialTerminate, 0);
            return CompiledStatement;

         case kwCase:
            /* @q Case (Elementary Command), EndSelect (Elementary Command)
               @noproto
               This keyword is part of the {Select} command, see there. */
         case kwElse:
            /* @q Else (Elementary Command)
               @noproto
               This keyword is part of the {If}, and {Try} statements, see there. */
         case kwEndIf:
            /* @q EndIf (Elementary Command)
               @noproto
               This keyword is part of the {If} statement, see there. */
         case kwEndOn:
            /* @q EndOn (Elementary Command)
               @noproto
               This keyword is part of the {On} statement, see there.
               @since PCC2 2.40.8 */
         case kwEndSelect:
         case kwEndSub:
            /* @q EndSub (Elementary Command)
               @noproto
               This keyword is part of the {Sub} command, see there. */
         case kwEndFunction:
            /* @q EndFunction (Elementary Command)
               @noproto
               This keyword is part of the {Function} command, see there. */
         case kwEndTry:
            /* @q EndTry (Elementary Command)
               @noproto
               This keyword is part of the {Try} command, see there. */
         case kwEndWith:
            /* @q EndWith (Elementary Command)
               @noproto
               This keyword is part of the {With} command, see there. */
         case kwLoop:
            /* @q Loop (Elementary Command)
               @noproto
               This keyword is part of the {Do} loop, see there. */
         case kwNext:
            /* @q Next (Elementary Command)
               @noproto
               This keyword is part of the {For} and {ForEach} loops, see there. */
            if (scc.hasFlag(CompilationContext::WantTerminators)) {
                return Terminator;
            } else {
                throw Error::misplacedKeyword(tok.getCurrentString().c_str());
            }

         case kwEndStruct:
            /* @q EndStruct (Elementary Command)
               @noproto
               This keyword is part of the {Struct} command, see there. */
            throw Error::misplacedKeyword("EndStruct");
         case kwEval:
            return compileEval(bco, scc);
         case kwFor:
            return compileFor(bco, scc);
         case kwForEach:
            return compileForEach(bco, scc);
         case kwFunction:
            return compileSub(bco, scc, false, Opcode::sShared);
         case kwIf:
            return compileIf(bco, scc);
         case kwLoad:
            return compileLoad(bco, scc, true);
         case kwLocal:
            return compileScope(bco, scc, Opcode::sLocal);
         case kwOn:
            return compileOn(bco, scc);
         case kwOption:
            return compileOption(bco, scc);
         case kwPrint:
            return compilePrint(bco, scc);
         case kwReDim:
            return compileReDim(bco, scc);
         case kwRestart:
            /* @q Restart (Elementary Command)
               @noproto
               This is not an actual script command.
               It can only be used in auto tasks.
               It causes the auto task to start again from the beginning.
               @since PCC2 1.99.16, PCC 1.0.19 */
            throw Error::misplacedKeyword("Restart");
         case kwReturn:
            tok.readNextToken();
            return compileReturn(bco, scc);
         case kwRunHook:
            return compileRunHook(bco, scc);
         case kwSelect:
            return compileSelect(bco, scc);
         case kwSelectionExec:
            return compileSelectionExec(bco, scc);
         case kwShared:
            return compileScope(bco, scc, Opcode::sShared);
         case kwStatic:
            return compileScope(bco, scc, Opcode::sStatic);
         case kwStop:
            /* @q Stop (Elementary Command)
               Suspend the process.
               The process will automatically be woken up periodically
               (normally, whenever you open your turn).
               This can be used to implement things like `Wait one turn':
               | Local t = Turn
               | Do While t = Turn
               |   Stop
               | Loop
               (this is precisely the definition of {WaitOneTurn}).

               Suspended processes will be saved to disk.
               However, there are restrictions upon the suspended process:
               - not all variables can be saved and restored safely.
                 In particular, user-interface related things normally cannot be saved,
                 thus processes that are in the middle of a user-interface action should not suspend.
               - PCC 1.x has various limits on the size of a suspended process;
                 if the process uses deep recursion or very large code sequences,
                 it will fail to save.

               As a general guideline, functions should not suspend, directly or indirectly.

               When a script wakes up again, all sorts of things may have been changed (for example, a turn has passed).
               Local and static variables will be saved with the process (because they belong to it exclusively),
               shared variables will not be saved.

               When the script executes in a context that no longer exists, it will not be restored.
               PCC will not wake up scripts when you temporarily switched back to an earlier turn.

               @since PCC2 1.99.10, PCC 1.0.7 */
            tok.readNextToken();
            parseEndOfLine();
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialSuspend, 0);
            return CompiledStatement;
         case kwStruct:
            return compileStruct(bco, scc, Opcode::sShared);
         case kwSub:
            return compileSub(bco, scc, true, Opcode::sShared);
         case kwTry:
            return compileTry(bco, scc);
         case kwTryLoad:
            return compileLoad(bco, scc, false);
         case kwUntil:
            throw Error::misplacedKeyword("Until");
         case kwUseKeymap:
            return compileUseKeymap(bco, scc);
         case kwWhile:
            throw Error::misplacedKeyword("While");
         case kwWith:
            return compileWith(bco, scc);
         case kwNone:
            if (SpecialCommand* sp = scc.world().lookupSpecialCommand(tok.getCurrentString())) {
                /* Special command */
                sp->compileCommand(tok, bco, scc);
                parseEndOfLine();
                return CompiledStatement;
            } else {
                /* Ambiguous command (expression or subroutine call) */
                return compileAmbiguousStatement(bco, scc);
            }
        }
        throw Error("Not implemented");
    } else {
        /* Anything else, must be expression */
        return compileExpressionStatement(bco, scc);
    }
}

// Compile statement list.
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileList(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileList
    while (1) {
        switch (Result r = compile(bco, scc)) {
         case EndOfInput:
         case Terminator:
            return r;
         case CompiledExpression:
            /* This means there is a value on top of the stack which we do not want */
            bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            m_commandSource.readNextLine();
            break;
         default:
            m_commandSource.readNextLine();
            break;
        }
    }
}

// Set optimisation level.
void
interpreter::StatementCompiler::setOptimisationLevel(int level)
{
    m_optimisationLevel = level;
}

// Finish a BCO.
void
interpreter::StatementCompiler::finishBCO(BytecodeObject& bco, const StatementCompilationContext& scc) const
{
    // ex IntStatementCompiler::finishBCO
    if (m_optimisationLevel > 0) {
        optimize(scc.world(), bco, m_optimisationLevel);
    }
    if (m_optimisationLevel >= 0) {
        bco.relocate();
    }
}

/** Compile ambiguous statement. If a statement starts with an unknown keyword, it might
    be an expression or a subroutine call. Both have distinct syntax, and even some
    identical productions: "SetSpeed +3" could be a subroutine call using argument "+3",
    or an expression performing an addition and discarding the result.

    If we have an execution context, we're lucky and can just look up what we find; it's
    an error if we find nothing.

    The complicated case is if we don't have an execution context. We try to determine
    the type of a statement by looking at the second token. It can be either a possible
    first token in an expression, or a possible second one. If it is a possible first,
    but not a possible second, we have a subroutine call. If it is a possible second, but
    not a possible first, we have an expression.

    Possible seconds are all binary operators, parentheses ("foo(x)") and dot.
    Possible firsts are all unary operators, identifiers, literals, parentheses.

    Cases that still cannot be resolved are compiled into a runtime switch by trying to
    compile them both as an expression and a statement, and branching depending on the
    first word. Many cases will be resolved implicitly by failing compilation of one of
    the branches (e.g. "SetWaypoint +x,+y" will fail because "," cannot appear in
    expressions, "Ship(id).Speed$ := 9" will fail because "(id).Speed$" is not a valid
    expression.

    This used to be compiled into a "Eval" statement, but this was much less efficient
    and will break lexical scoping. */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileAmbiguousStatement(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileAmbiguousStatement
    const String_t name = m_commandSource.tokenizer().getCurrentString();
    if (ContextProvider* cp = scc.getContextProvider()) {
        /* We have an execution context, so we can actually look up the value to see what it is.
           Note that it is an error to have a ContextProvider and be in multi-line mode;
           see StatementCompilationContext::withContextProvider. */
        bool is_proc;
        Context::PropertyIndex_t index;
        if (Context* con = cp->lookup(name, index)) {
            std::auto_ptr<afl::data::Value> v(con->get(index));
            if (CallableValue* cv = dynamic_cast<CallableValue*>(v.get())) {
                // Callable builtin
                is_proc = cv->isProcedureCall();
            } else {
                // Variable
                is_proc = false;
            }
        } else {
            // Unknown; expression may be valid if this is a builtin fundamental function
            if (interpreter::expr::lookupBuiltinFunction(name) == 0) {
                throw Error::unknownIdentifier(name);
            }
            is_proc = false;
        }

        if (is_proc) {
            /* It's a procedure */
            return compileProcedureCall(bco, scc);
        } else {
            /* It's a function */
            return compileExpressionStatement(bco, scc);
        }
    } else {
        /* Find next token */
        Tokenizer::TokenType next = Tokenizer(m_commandSource.tokenizer()).readNextToken();
        switch (next) {
         case Tokenizer::tInteger:
         case Tokenizer::tFloat:
         case Tokenizer::tString:
         case Tokenizer::tBoolean:
         case Tokenizer::tIdentifier:
         case Tokenizer::tNOT:
            /* Possible firsts but not possible seconds: subroutine call */
            return compileProcedureCall(bco, scc);

         case Tokenizer::tAND:
         case Tokenizer::tOR:
         case Tokenizer::tXOR:
         case Tokenizer::tMOD:
         case Tokenizer::tNE:
         case Tokenizer::tGE:
         case Tokenizer::tLE:
         case Tokenizer::tAssign:
         case Tokenizer::tAmpersand:
         case Tokenizer::tMultiply:
         case Tokenizer::tSlash:
         case Tokenizer::tBackslash:
         case Tokenizer::tCaret:
         case Tokenizer::tEQ:
         case Tokenizer::tLT:
         case Tokenizer::tGT:
         case Tokenizer::tDot:
         case Tokenizer::tArrow:
         case Tokenizer::tSemicolon:
            /* Possible seconds but not possible firsts: expression */
            return compileExpressionStatement(bco, scc);

         case Tokenizer::tComma:
         case Tokenizer::tInvalid:
         case Tokenizer::tRParen:
         case Tokenizer::tColon:
            /* Impossible anywhere */
            throw Error("Syntax error");

         case Tokenizer::tEnd:
            /* Single word */
            return compileAmbiguousSingleWord(name, bco, scc);

         case Tokenizer::tHash:
         case Tokenizer::tPlus:
         case Tokenizer::tMinus:
         case Tokenizer::tLParen:
         default:
#if 1
            /* Possible firsts and seconds: compile as both */
            return compileAmbiguousRuntimeSwitch(name, next==Tokenizer::tLParen, bco, scc);
#else
            /* Possible firsts and seconds: defer until execution */
            IntStringValue sv(name + m_commandSource.tokenizer().getRemainingLine());
            bco.addPushLiteral(&sv);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, 1);
            return CompiledStatement;
#endif
        }
    }
}

/** Compile ambiguous single-word statement into a runtime switch. This compiles
    a statement as expression and procedure call, and decides at runtime which
    case to use. It is used for statements consisting of a single word. This is
    a subset of compileAmbiguousRuntimeSwitch, generating simpler code.
    \param name [in] Name in first position
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileAmbiguousSingleWord(const String_t& name, BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileAmbiguousSingleWord
    // FIXME: this evaluates the variable reference twice (runtime cost).
    // A version that evaluates it only once would be
    //      push
    //      dup 0
    //      uisproc
    //      jfep 1F
    //      procind 0
    //      j 2F
    //   1: drop
    //   2:
    // and would be preferrable for by-name lookups.
    BytecodeObject::Label_t lskip = bco.makeLabel();
    bco.addVariableReferenceInstruction(Opcode::maPush, name, scc);
    bco.addInstruction(Opcode::maUnary, unIsProcedure, 0);
    bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, lskip);
    bco.addVariableReferenceInstruction(Opcode::maPush, name, scc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall + Opcode::miIMRefuseFunctions, 0);
    bco.addLabel(lskip);
    return CompiledStatement;
}

/** Compile ambiguous statement into a runtime switch. This compiles a statement
    as expression and procedure call, and decides at runtime which case to use.
    \param name [in] Name in first position
    \param paren [in] true iff name is followed by an opening parenthesis
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileAmbiguousRuntimeSwitch(const String_t& name, bool paren, BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileAmbiguousRuntimeSwitch

    /* Compile both halves */
    BytecodeObject procBCO;
    BytecodeObject exprBCO;
    Error procError("");
    Error exprError("");
    procBCO.copyLocalVariablesFrom(bco);
    exprBCO.copyLocalVariablesFrom(bco);
    bool procValid = false;
    bool exprValid = false;
    Tokenizer tokSave = m_commandSource.tokenizer();
    try {
        compileProcedureCall(procBCO, scc);
        procValid = true;
    }
    catch (Error& e) {
        procError = e;
    }
    m_commandSource.tokenizer() = tokSave;
    try {
        compileExpressionStatement(exprBCO, DefaultStatementCompilationContext(scc).withFlag(CompilationContext::ExpressionsAreStatements));
        exprValid = true;
    }
    catch (Error& e) {
        exprError = e;
    }

    /* Check what happened */
    if (procValid) {
        if (exprValid) {
            /* Valid as both kinds */
            /*
               [catch   @fail]
                pushvar name
               [suncatch]
                uisproc
                jfep    @expr
                <proc>
                j @done
              [@fail
                drop 1]
              @expr
                <expr>
              @done

                The "catch" is required when the statement starts as "xx(",
                where "xx" is a possible builtin. Builtins do not appear in
                the symbol table, which causes the runtime switch to fail
                otherwise with an undefined identifier error even if the
                statement is actually correct.

                The "catch" is also only required if "xx" does not appear in
                the local or global frames. Even if it is not compiled into a
                "pushloc" due to an enclosing 'With', we'll know that it will
                at runtime find something.
            */
            // FIXME: this evaluates the variable reference twice (runtime cost).
            // To evaluate only once, we'd need to do rewrite the halves (replace push by dup).
            bool protect = paren
                && interpreter::expr::lookupBuiltinFunction(name) != 0
                && !bco.hasLocalVariable(name);
            BytecodeObject::Label_t lexpr = bco.makeLabel();
            BytecodeObject::Label_t ldone = bco.makeLabel();
            BytecodeObject::Label_t lfail = bco.makeLabel();;
            if (protect) {
                bco.addJump(Opcode::jCatch, lfail);
            }
            bco.addVariableReferenceInstruction(Opcode::maPush, name, scc);
            if (protect) {
                bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
            }
            bco.addInstruction(Opcode::maUnary, unIsProcedure, 0);
            bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, lexpr);
            bco.append(procBCO);
            bco.addJump(Opcode::jAlways, ldone);
            if (protect) {
                bco.addLabel(lfail);
                bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            }
            bco.addLabel(lexpr);
            bco.append(exprBCO);
            bco.addLabel(ldone);
        } else {
            /* Only valid as procedure */
            bco.append(procBCO);
        }
    } else {
        if (exprValid) {
            /* Only valid as expression */
            bco.append(exprBCO);
        } else {
            /* Both failed */
            throw procError;
        }
    }
    return CompiledStatement;
}

/** Compile "Abort" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileAbort(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileAbort
    /* @q Abort Optional what:Str (Elementary Command)
       Abort script with an error.
       The result is the same as if you had an error in your script, e.g. a division by zero
       or use of an undefined variable.

       If there is a surrounding {Try} block, execution will resume in its %Else part
       (or after its %EndTry if there is no %Else).
       Otherwise, the script will stop, with the error message printed on the console.

       @since PCC2 1.99.9, PCC 1.0.6 */

    /* Parse args */
    afl::container::PtrVector<interpreter::expr::Node> args;
    m_commandSource.tokenizer().readNextToken();
    parseArgumentList(args);
    checkArgumentCount(args.size(), 0, 1);

    /* Compile */
    if (args.size() == 0) {
        bco.addPushLiteral(0);
    } else {
        args[0]->compileValue(bco, scc);
    }
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
    return CompiledStatement;
}

/** Compile "Bind" statement.
    \param bco Bytecode output
    \param scc Statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileBind(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileBind
    /* @q Bind keymap:Keymap key:Str := action:Any... (Elementary Command)
       Assign keys.
       This command arranges that %action is invoked when the %key is pressed while %keymap is active.
       %keymap is an identifier or {ByName()} expression.
       The %key is a string specifying the key, the %action is either a string containing a command,
       or a numeric atom (see {Atom}, {Key}).

       For example,
       <pre class="ccscript">
         Bind PlanetScreen "a" := "AutoBuild"
       </pre>
       makes the <kbd>A</kbd> key on the planet screen run the {AutoBuild} command.
       New key definitions override old definitions.

       You can define multiple keys in the same keymap in one line,
       by simply writing multiple assignments separated by commas.

       Keystrokes consist of zero or more modifiers (<tt>Ctrl-</tt>, <tt>Alt-</tt>, <tt>Shift-</tt>, <tt>Meta-</tt>,
       possibly abbreviated to <tt>C-</tt>, <tt>A-</tt>, etc.), followed by a key name.
       A key name is either an ASCII character, or a special key name: <tt>F1</tt> to <tt>F15</tt>,
       <tt>Backspace</tt>/<tt>BS</tt>,
       <tt>Pause</tt>,
       <tt>Del</tt>,
       <tt>Down</tt>,
       <tt>End</tt>,
       <tt>ESC</tt>,
       <tt>Home</tt>,
       <tt>Ins</tt>,
       <tt>Left</tt>,
       <tt>Num5</tt>,
       <tt>PgDn</tt>,
       <tt>PgUp</tt>,
       <tt>Print</tt>,
       <tt>Ret</tt>/<tt>Enter</tt>,
       <tt>Right</tt>,
       <tt>Space</tt>/<tt>Spc</tt>,
       <tt>Tab</tt>,
       <tt>Up</tt>,
       or <tt>WheelUp</tt>/<tt>WheelDown</tt> for mouse wheel events.
       In addition, <tt>Quit</tt> means the "close-me" button on the window frame (<tt>[X]</tt>).
       The available combinations differ between PCC versions and operating systems.

       To undefine a key, bind it to the empty string.
       Unlike object properties, keymaps do not survive PCC exiting and re-loading.

       The commands you bind to keys can examine the {UI.Prefix} variable to find out the current prefix argument.

       Unlike PCC 1.x, PCC2 is case-sensitive.
       When you bind <tt>Shift-A</tt>, you must actually type an upper-case A to trigger this function
       (i.e. press <kbd>Shift-A</kbd>).
       PCC 1.x didn't distinguish between upper and lower case for (latin) alphabetic keys.
       Otherwise, PCC2 ignores the Shift modifier for printable keys.
       <kbd>Shift-4</kbd> generates a "$" sign, so you have to bind <tt>$</tt>, not <tt>Shift-4</tt>,
       if you want something to happen on <kbd>Shift-4</kbd>.
       When in doubt, use the <a href="pcc2:keymap">keymap debugger</a>.

       @since PCC2 1.99.9, PCC 1.0.12 */

    /* Bind keymap <expr> := <expr> [, <expr> := <expr>]*
           pushlit "keymap"
           ukeylookup
          [<expr>
           <expr>
           tkeyadd]*
           drop 1 */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    compileNameString(bco, scc, "keymap name");
    bco.addInstruction(Opcode::maUnary, unKeyLookup, 0);

    /* Parse assignments */
    while (1) {
        std::auto_ptr<interpreter::expr::Node> expr(interpreter::expr::Parser(m_commandSource.tokenizer()).parseNA());
        expr->compileValue(bco, scc);

        /* Here, we only accept ":=", because "=" is swallowed by parseNA anyway. */
        if (!tok.checkAdvance(tok.tAssign)) {
            throw Error::expectSymbol(":=");
        }

        expr.reset(interpreter::expr::Parser(m_commandSource.tokenizer()).parseNA());
        expr->compileValue(bco, scc);

        bco.addInstruction(Opcode::maTernary, teKeyAdd, 0);

        if (!parseNext(tok)) {
            break;
        }
    }

    /* Drop keymap */
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    return CompiledStatement;
}

/** Compile "Call" statement.
    \param bco Bytecode output
    \param scc Statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileCall(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    /* @q Call command args, ... (Elementary Command)
       Invoke a command.

       Normally, commands are invoked by listing their name and arguments, as in
       <pre class="ccscript">
         SetWaypoint 1000, 1020
       </pre>
       However, this only works if the command is a single word.
       Invoking a command on another object requires {With}.

       Using <tt>Call</tt>, you can invoke commands using an expression.
       This allows commands that are hard to express using {With}, for example
       <pre class="ccscript">
         % set towee's fcode to the same as ours
         Call Ship(Mission.Tow).SetFCode FCode
       </pre>
       In addition, it can be a tiny bit more efficient in some cases.

       <b>Caveat Emptor:</b> when interpreting the "command" expression,
       {Call} will consume the longest possible expression (greedy parsing).
       This means, <tt>Call Foo -1</tt> will be interpreted as the call of a subtraction expression,
       which is meaningless, instead of as a call of <tt>Foo</tt> with parameter <tt>-1</tt>.
       In this case, add an additional comma to indicate where the "command" expression ends:
       <pre class="ccscript">
         Call Foo, -1
       </pre>

       @since PCC2 2.0.2, PCC2 2.40.1 */

    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();

    /* Procedure */
    std::auto_ptr<interpreter::expr::Node> procedure(interpreter::expr::Parser(tok).parse());

    /* Skip comma */
    tok.checkAdvance(tok.tComma);

    /* Arguments */
    afl::container::PtrVector<interpreter::expr::Node> args;
    parseArgumentList(args);
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, scc);
    }

    /* Warning for code such as
         Call Foo +1
       which would be an ambiguous-but-eventually-correctly-executed runtime switch without 'Call',
       but is always a binary operator that fails execution with 'Call'. */
    if (interpreter::expr::SimpleNode* n = dynamic_cast<interpreter::expr::SimpleNode*>(procedure.get())) {
        if (n->is(Opcode::maBinary, biConcat) || n->is(Opcode::maBinary, biAdd) || n->is(Opcode::maBinary, biSub)) {
            Error e("Binary operator in first operand to 'Call' is most likely not what you want");
            m_commandSource.addTraceTo(e, afl::string::Translator::getSystemInstance());
            scc.world().logError(afl::sys::LogListener::Warn, e);
        }
    }

    /* Call */
    procedure->compileValue(bco, scc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall + Opcode::miIMRefuseFunctions, uint16_t(args.size()));

    return CompiledStatement;
}


/** Compile "CreateKeymap" statement.
    \param bco Bytecode output
    \param scc Statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileCreateKeymap(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileCreateKeymap
    /* @q CreateKeymap name(parent:Keymap...),... (Elementary Command)
       Create a keymap.
       A keymap contains a set of keystrokes and commands active in a particular context.
       The %name is an identifier or {ByName()} expression that names the new keymap; this keymap must not yet exist.
       If desired, one or more parent keymaps can be specified in parentheses;
       if the keymap should not have a parent keymap, the parentheses can be omitted.

       The keymap can later be filled with keys using the {Bind} command.

       A key is looked up first in the keymap itself.
       If it is not found there, it is searched for in the parents.

       Keymaps have a separate namespace from variables,
       i.e. a keymap %MyStuff and a variable %MyStuff are not related in any way.

       See {int:index:type:keymap|Keymaps} for a list of all predefined keymaps and related information.

       @diff PCC 1.x allows at most one parent keymap; PCC2 allows multiple parents.
       @see Bind, UseKeymap, Key
       @since PCC2 1.99.9, PCC 1.0.12 */

    /* CreateKeymap keymap(parent...), ...
           pushlit "keymap"
           ukeycreate
          [pushlit "parent
           ukeylookup
           bkeyaddparent]
           drop 1 */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    while (1) {
        compileNameString(bco, scc, "keymap name");
        bco.addInstruction(Opcode::maUnary, unKeyCreate, 0);
        if (tok.checkAdvance(tok.tLParen)) {
            if (!tok.checkAdvance(tok.tRParen)) {
                while (1) {
                    compileNameString(bco, scc, "parent keymap name");
                    bco.addInstruction(Opcode::maUnary, unKeyLookup, 0);
                    bco.addInstruction(Opcode::maBinary, biKeyAddParent, 0);
                    if (tok.checkAdvance(tok.tComma)) {
                        // continue
                    } else if (tok.checkAdvance(tok.tRParen)) {
                        // end
                        break;
                    } else {
                        throw Error::expectSymbol(",", ")");
                    }
                }
            }
        }
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);

        if (!parseNext(tok)) {
            break;
        }
    }
    return CompiledStatement;
}

/** Compile "CreateShipProperty" or "CreatePlanetProperty" statement.
    \param bco Bytecode output
    \param scc Statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileCreateProperty(BytecodeObject& bco, const StatementCompilationContext& /*scc*/, Opcode::Special minor, const char* prefix)
{
    // ex IntStatementCompiler::compileCreateProperty
    /* @q CreateShipProperty name,... (Elementary Command), CreatePlanetProperty name,... (Elementary Command)
       Create new property.
       Parameter to this command is a list of names for the new ship/planet properties.

       The properties will start out EMPTY.
       For example, after
       <pre class="ccscript">
         CreatePlanetProperty happy.goal
       </pre>
       all planets will have an empty property %happy.goal. You can assign to it with
       <pre class="ccscript">
         Planet(19).happy.goal := 94
         % ... or ...
         With Planet(19) Do happy.goal := 94
       </pre>
       If a property you create with either of these commands was already created, nothing happens.

       Properties created with these commands "shadow" the normal built-in properties.
       That is, if you create a property with the same name as a built-in property,
       the built-in property will become inaccessible. Be careful.

       Properties are saved in the starcharts file (<tt>chartX.cc</tt>).
       If the starcharts file contains an undeclared property with an interesting value (non-EMPTY),
       the property is automatically declared to avoid data loss.
       To get rid of a property forever, set all its values to EMPTY and do no longer declare it.
       @since PCC 1.0.8, PCC2 1.99.9 */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    while (1) {
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("property name");
        }
        bco.addInstruction(Opcode::maSpecial, minor, bco.addName(stripPrefix(tok.getCurrentString(), prefix)));
        tok.readNextToken();
        if (!parseNext(tok)) {
            break;
        }
    }
    return CompiledStatement;
}

/** Compile "Dim" statement.
    \param bco Bytecode output
    \param scc Statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileDim(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileDim
    /* @q Dim [Local|Static|Shared] name [initializer],... (Elementary Command)
       Create a variable.

       You can create variables of different kind:
       - %Local variables variables exist during the current subroutine or file only. This is the default.
       - %Static variables exist during the current script execution.
       - %Shared variables exist for all scripts you execute.
       If you do specify a variable kind, you can omit the %Dim keyword, i.e.
       <tt>Dim Static a</tt> is equivalent to <tt>Static a</tt>.

       If the variable you create is indeed new, it will be initialized with the %initializer
       (if no initializer is specified, it will start EMPTY).
       If the variable already exists, the initializer will be evaluated,
       but the variable keeps its original value.

       The initializer can have the following forms:
       <table>
        <tr><td width="16"><tt><font color="dim">name</font>(expr, ...)</tt></td>
            <td width="18">An <a href="int:index:type:array">array</a>
             of the specified dimensions, all values EMPTY.</td></tr>
        <tr><td><tt><font color="dim">name</font>(expr, ...) As type</tt></td>
            <td>An <a href="int:index:type:array">array</a> of the specified dimensions,
             all values initialized with the specified type (see below).</td></tr>
        <tr><td><tt><font color="dim">name</font> := expression</tt></td>
            <td>Initialized with the specified expression.</td></tr>
        <tr><td><tt><font color="dim">name</font> As type</tt></td>
            <td>Initialized with the default value for the specified type.</td></tr>
       </table>

       The type can be a structure name defined with {Struct} to initialize the variable
       (or the array elements) with fresh instances of that structure, or one of the following:
       <table>
        <tr><td width="8">Any</td>    <td width="26">Allow any type, initialize with EMPTY.</td></tr>
        <tr><td width="8">Double</td> <td width="26"><a href="int:index:type:num">Fractional</a>, initialize to 0.0.</td></tr>
        <tr><td width="8">Float</td>  <td width="26"><a href="int:index:type:num">Fractional</a>, initialize to 0.0.</td></tr>
        <tr><td width="8">Hash</td>   <td width="26"><a href="int:index:type:hash">Hash</a>, initialize to a blank hash.</td></tr>
        <tr><td width="8">Integer</td><td width="26"><a href="int:index:type:int">Integer</a>, initialize to 0.</td></tr>
        <tr><td width="8">Long</td>   <td width="26"><a href="int:index:type:int">Integer</a>, initialize to 0.</td></tr>
        <tr><td width="8">Single</td> <td width="26"><a href="int:index:type:num">Fractional</a>, initialize to 0.0.</td></tr>
        <tr><td width="8">String</td> <td width="26"><a href="int:index:type:str">String</a>, initialize to "".</td></tr>
       </table>

       Examples:
       <pre class="ccscript">
         Dim a, b, c              % Three local variables
         Dim four = 4             % Local variable with value 4
         Dim i As Integer         % Local variable, integer
         Dim mat(10,10)           % 10x10 matrix (2-D array)
         Dim ps As MyStruct       % Structure
         Dim Shared gv            % Shared variable
       </pre>

       @diff PCC 1.x supports only simple value initialisations,
       and does not support arrays, hashes, or %As initialisation.
       @since PCC2 1.99.8, PCC 1.0.6
       @see Dim (Elementary Function) */

    /* Read scope */
    Opcode::Scope scope = Opcode::sLocal;
    Tokenizer& tok = m_commandSource.tokenizer();
    if (tok.checkAdvance("LOCAL")) {
        scope = Opcode::sLocal;
    } else if (tok.checkAdvance("STATIC")) {
        scope = Opcode::sStatic;
    } else if (tok.checkAdvance("SHARED")) {
        scope = Opcode::sShared;
    } else {
        /* nix */
    }

    /* Compile variable definitions */
    compileVariableDefinition(bco, scc, scope);
    return CompiledStatement;
}

/** Compile "Do" (generic loop) statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileDo(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileDo
    /* @q Do (Elementary Command)
       @noproto
       | Do [While c|Until c]
       |   statements
       | Loop [While c|Until c]
       Executes the statements in a loop.
       Loop conditions can be put at the top of the loop, or at the bottom, or even both.
       The top (%Do) condition is checked before each iteration and determines whether the iteration begins.
       The bottom (%Loop) condition is checked after each iteration and determines whether another iteration is tried.

       The conditions (%c) evaluate to <a href="int:index:type:bool">bool</a>.
       A %While condition expects a True result to enter/continue the loop,
       a %Until condition expects a False result.

       If no condition is specified, the loop runs infinitely and can only be stopped with {Break}.

       @since PCC2 1.99.8, PCC 1.0.6
       @see Break, Continue, For */

    /* Do [While <a1>|Until <a2>] / <body> / Loop [While <e1>|Until <e2>]

       again:
         <a1/a2>
       do:
         <body>
       continue:
         <e1/e2>/j again
       break: */

    /* Context for child code */
    struct DoStatementCompilationContext : public StatementCompilationContext {
        DoStatementCompilationContext(const StatementCompilationContext& parent, BytecodeObject::Label_t lcontinue, BytecodeObject::Label_t lbreak)
            : StatementCompilationContext(parent),
              m_continueLabel(lcontinue),
              m_breakLabel(lbreak)
            {
                withoutFlag(LinearExecution);
                setBlockSyntax();
            }
        void compileBreak(BytecodeObject& bco) const
            { bco.addJump(Opcode::jAlways, m_breakLabel); }
        void compileContinue(BytecodeObject& bco) const
            { bco.addJump(Opcode::jAlways, m_continueLabel); }
        void compileCleanup(BytecodeObject& bco) const
            { defaultCompileCleanup(bco); }

        BytecodeObject::Label_t m_continueLabel, m_breakLabel;
    };

    /* Allowed? */
    validateMultiline(scc);

    /* Make labels */
    BytecodeObject::Label_t lagain    = bco.makeLabel();
    BytecodeObject::Label_t ldo       = bco.makeLabel();
    BytecodeObject::Label_t lbreak    = bco.makeLabel();
    BytecodeObject::Label_t lcontinue = bco.makeLabel();
    bco.addLabel(lagain);

    /* Compile head condition */
    // @change It seems PCC1 treats EMPTY as always-fail (does not enter loop, does not continue loop).
    // This interpreter treats EMPTY the same as false.
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    if (tok.checkAdvance("WHILE")) {
        compileArgumentCondition(bco, scc, ldo, lbreak);
    } else if (tok.checkAdvance("UNTIL")) {
        compileArgumentCondition(bco, scc, lbreak, ldo);
    }
    parseEndOfLine();

    /* Compile body */
    m_commandSource.readNextLine();
    bco.addLabel(ldo);
    compileList(bco, DoStatementCompilationContext(scc, lcontinue, lbreak));
    if (!tok.checkAdvance("LOOP")) {
        throw Error::expectKeyword("Loop");
    }

    /* Compile tail condition */
    bco.addLabel(lcontinue);
    if (tok.checkAdvance("UNTIL")) {
        compileArgumentCondition(bco, scc, lbreak, lagain);
    } else if (tok.checkAdvance("WHILE")) {
        compileArgumentCondition(bco, scc, lagain, lbreak);
    } else {
        bco.addJump(Opcode::jAlways, lagain);
    }
    parseEndOfLine();

    bco.addLabel(lbreak);
    return CompiledBlock;
}

/** Compile "Eval" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileEval(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileEval
    /* @q Eval stmt:Str... (Elementary Command)
       Evaluate a statement given as string.
       If multiple parameters are given, they are evaluated as a statement list or multiline command.
       A single line is evaluated as a single-line command.
       @since PCC 1.0.16, PCC2 1.99.9 */

    /* Skip over "Eval" token */
    m_commandSource.tokenizer().readNextToken();

    /* Read arguments */
    afl::container::PtrVector<interpreter::expr::Node> args;
    parseArgumentList(args);
    if (args.size() == 0)
        throw Error("Too few arguments to 'Eval'");

    /* Compile */
    for (size_t i = 0; i < args.size(); ++i) {
        args[i]->compileValue(bco, scc);
    }
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEvalStatement, uint16_t(args.size()));

    return CompiledStatement;
}

/** Compile "For" (counting loop) statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileFor(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileFor
    /* @q For (Elementary Command)
       @noproto
       | For var := start To end Do command
       |
       | For var := start To end [Do]
       |   commands
       | Next
       Counting loop.
       The variable %var, which must have been declared before,
       starts at %start and counts up in steps of 1 until it reaches %end.
       For each value, the command (or command list) is executed.

       For example,
       | For i:=1 To 5 Do Print i
       prints the numbers 1, 2, 3, 4 and 5.

       @see Break, Continue, ForEach
       @since PCC2 1.99.9, PCC 1.0.12 */

    /* For <var> := <start> To <end> Do <body> */

    /* Context for child block */
    struct ForStatementCompilationContext : public StatementCompilationContext {
        ForStatementCompilationContext(const StatementCompilationContext& parent,
                                       bool mustdrop,
                                       BytecodeObject::Label_t lcontinue,
                                       BytecodeObject::Label_t lbreak)
            : StatementCompilationContext(parent),
              mustdrop(mustdrop),
              lcontinue(lcontinue),
              lbreak(lbreak)
            {
                withoutFlag(LinearExecution);
            }
        void compileBreak(BytecodeObject& bco) const
            {
                if (mustdrop)
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
                bco.addJump(Opcode::jAlways, lbreak);
            }
        void compileContinue(BytecodeObject& bco) const
            {
                bco.addJump(Opcode::jAlways, lcontinue);
            }
        void compileCleanup(BytecodeObject& bco) const
            {
                if (mustdrop)
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
                defaultCompileCleanup(bco);
            }

        bool mustdrop;
        BytecodeObject::Label_t lcontinue, lbreak;
    };

    /* Parse it */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();

    /* ...induction variable... */
    if (tok.getCurrentToken() != tok.tIdentifier) {
        throw Error::expectIdentifier("variable name");
    }
    const String_t var = tok.getCurrentString();
    tok.readNextToken();

    if (!tok.checkAdvance(tok.tEQ) && !tok.checkAdvance(tok.tAssign)) {
        throw Error::expectSymbol("=", ":=");
    }

    /* ...start expression... */
    std::auto_ptr<interpreter::expr::Node> start(interpreter::expr::Parser(tok).parse());
    if (!tok.checkAdvance("TO")) {
        throw Error::expectKeyword("To");
    }

    /* ...end expression... */
    std::auto_ptr<interpreter::expr::Node> end(interpreter::expr::Parser(tok).parse());

    /* Generate code for head */
    /*
                  <end>                 ; b             ; break must include drop
                 [upos]                                 ; ensure it is a number
                  <start>               ; b:a
                 [upos]                                 ; ensure it is a number
           again: store <var>           ; b:a
                  dup 1                 ; b:a:b
                  bcmple                ; b:res
                  jfep out              ; b
                  <body>
        continue: push <var>            ; b:i
                  uinc                  ; b:1+i
                  j again
             out: drop 1                ;
           break:

       Alternate version if <end> is a literal:
                  <start>
                 [upos]
           again: store <var>           ; a
                  <end>                 ; a:b
                  bcmple                ; res
                  jfep break            ;
                  <body>
        continue: push <var>            ; i
                  uinc                  ; 1+i
                  j again
           break:

       Alternate version if <start> and <end> are literals and we can prove the loop runs once:
                  <start>
                  pop <var>
           again: <body>
        continue: push <var>
                  uinc
                  store <var>
                  <end>
                  bcmple
                  jtp again
           break: */

    BytecodeObject::Label_t lagain    = bco.makeLabel();
    BytecodeObject::Label_t lcontinue = bco.makeLabel();
    BytecodeObject::Label_t lout      = bco.makeLabel();
    BytecodeObject::Label_t lbreak    = bco.makeLabel();

    bool endIsLiteral = (dynamic_cast<interpreter::expr::LiteralNode*>(end.get()) != 0) && m_optimisationLevel >= 0;

    if (!endIsLiteral) {
        end->compileValue(bco, scc);
        bco.addInstruction(Opcode::maUnary, unPos, 0);
    }
    start->compileValue(bco, scc);
    bco.addInstruction(Opcode::maUnary, unPos, 0);
    bco.addLabel(lagain);
    bco.addVariableReferenceInstruction(Opcode::maStore, var, scc);
    if (endIsLiteral) {
        end->compileValue(bco, scc);
    } else {
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, 1);
    }
    bco.addInstruction(Opcode::maBinary, biCompareLE, 0);          /* no need to handle CaseBlind, we're dealing with numbers */
    bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, lout);

    ForStatementCompilationContext subcc(scc, !endIsLiteral, lcontinue, lbreak);

    /* Body */
    Result result = compileLoopBody(bco, subcc);

    /* Compile tail */
    bco.addLabel(lcontinue);
    bco.addVariableReferenceInstruction(Opcode::maPush, var, scc);
    bco.addInstruction(Opcode::maUnary, unInc, 0);
    bco.addJump(Opcode::jAlways, lagain);
    bco.addLabel(lout);
    if (!endIsLiteral) {
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    }
    bco.addLabel(lbreak);

    return result;
}

/** Compile "ForEach" (set loop) statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileForEach(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileForEach
    /* @q ForEach (Elementary Command)
       @noproto
       | ForEach set Do command
       |
       | ForEach set As name Do command
       |
       | ForEach set [As name] [Do]
       |   commands
       | Next
       Iteration over object array.
       The %set is an array of objects, such as {Ship} or {Planet}.
       The loop will iterate through all objects in that set,
       and execute the command or command list for each of them.

       By default, with no <tt>As name</tt> clause,
       the commands will be executed in an appropriate context,
       as if {With} were used.
       For example,
       | ForEach Minefield Do
       |   If LastScan < Turn-10 Then Delete
       | Next
       will delete all minefields not seen within the last 10 turns.

       If <tt>As name</tt> is given, %name is a name of a variable.
       For each iteration, %name will be set refer to the respective object.
       For example,
       | Dim mf
       | ForEach Minefield As mf Do
       |   If Distance(mf, 1010, 1020) < mf->Radius Then Print mf->Id
       | Next
       will print the Ids of all minefields that cover (1010,1020).
       This syntax is available since PCC2 2.40.7.

       @see Break, Continue, For, Do, Count(), Find()
       @since PCC2 1.99.9, PCC 1.0.6 */

    /*  <expr>         break: endindex
        sfirstindex           j end
        jfep end
     again:
        <body>
     continue:
        snextindex
        jtp again
     end: */

    Tokenizer& tok = m_commandSource.tokenizer();

    /* Make labels */
    const BytecodeObject::Label_t lagain    = bco.makeLabel();
    const BytecodeObject::Label_t lend      = bco.makeLabel();
    const BytecodeObject::Label_t lcontinue = bco.makeLabel();

    /* Compile scope expression */
    tok.readNextToken();
    std::auto_ptr<interpreter::expr::Node> scope_expr(interpreter::expr::Parser(tok).parse());
    if (tok.checkAdvance("AS")) {
        /* Named iteration variable */
        /*     <expr>             break: j end
               sfirst
            again:
               storevar <var>
               jfe end
               <body>
            continue:
               snext
               j again
            end:
               drop 1 */
        // FIXME: potential future extension: "ForEach Ship As Local S" --> automatic implied "Dim S"
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("variable name");
        }
        String_t name = tok.getCurrentString();
        validateName(scc, name);
        tok.readNextToken();

        /* Context for child code */
        class ForEachStatementCompilationContext : public StatementCompilationContext {
         public:
            ForEachStatementCompilationContext(const StatementCompilationContext& parent,
                                               BytecodeObject::Label_t lcontinue,
                                               BytecodeObject::Label_t lend)
                : StatementCompilationContext(parent),
                  lcontinue(lcontinue),
                  lend(lend)
                {
                    withoutFlag(LinearExecution);
                }
            void compileBreak(BytecodeObject& bco) const
                {
                    /* This will leave the induction variable set to the current value.
                       This is not an explicitly documented feature for now; leaving it anyway for
                       now because it may allow for something clever. */
                    bco.addJump(Opcode::jAlways, lend);
                }
            void compileContinue(BytecodeObject& bco) const
                { bco.addJump(Opcode::jAlways, lcontinue); }
            void compileCleanup(BytecodeObject& bco) const
                {
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
                    defaultCompileCleanup(bco);
                }
         private:
            BytecodeObject::Label_t lcontinue, lend;
        };

        ForEachStatementCompilationContext subcc(scc, lcontinue, lend);

        /* Compile loop head */
        scope_expr->compileValue(bco, scc);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialFirst, 0);
        bco.addLabel(lagain);
        bco.addVariableReferenceInstruction(Opcode::maStore, name, scc);
        bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty, lend);

        /* Compile loop body */
        Result result = compileLoopBody(bco, subcc);

        /* Compile loop tail */
        bco.addLabel(lcontinue);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNext, 0);
        bco.addJump(Opcode::jAlways, lagain);
        bco.addLabel(lend);
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
        return result;
    } else {
        /* Context for child code */
        class ForEachStatementCompilationContext : public StatementCompilationContext {
         public:
            ForEachStatementCompilationContext(const StatementCompilationContext& parent,
                                               BytecodeObject::Label_t lcontinue,
                                               BytecodeObject::Label_t lend)
                : StatementCompilationContext(parent),
                  lcontinue(lcontinue),
                  lend(lend)
                {
                    withoutFlag(LocalContext);
                    withoutFlag(LinearExecution);
                    withContextProvider(0);
                }
            void compileBreak(BytecodeObject& bco) const
                {
                    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndIndex, 0);
                    bco.addJump(Opcode::jAlways, lend);
                }
            void compileContinue(BytecodeObject& bco) const
                {
                    bco.addJump(Opcode::jAlways, lcontinue);
                }
            void compileCleanup(BytecodeObject& bco) const
                {
                    defaultCompileCleanup(bco);
                }
         private:
            BytecodeObject::Label_t lcontinue, lend;
        };

        ForEachStatementCompilationContext subcc(scc, lcontinue, lend);

        /* Compile loop head */
        scope_expr->compileValue(bco, scc);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialFirstIndex, 0);
        bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty | Opcode::jPopAlways, lend);
        bco.addLabel(lagain);

        /* Compile loop body */
        Result result = compileLoopBody(bco, subcc);

        /* Compile loop tail */
        bco.addLabel(lcontinue);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNextIndex, 0);
        bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, lagain);
        bco.addLabel(lend);
        return result;
    }
}

/** Compile "If" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileIf(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileIf
    /* @q If (Elementary Command)
       @noproto
       | If cond Then command
       |
       | If cond [Then]
       |   commands
       | Else If cond [Then]
       |   commands
       | Else
       |   commands
       | EndIf
       Conditional execution.
       The %If condition is evaluated.
       If it yields True, the first (or only) set of commands is executed.
       If it yields False or EMPTY, the <tt>Else If</tt> condition, if any, is checked,
       and the first matching set of commands is executed.
       If neither condition yields True, the %Else commands are executed.

       There can be any number of <tt>Else If</tt> blocks (including none at all),
       and zero or one %Else blocks.

       @diff <tt>Else If</tt> is supported since PCC 1.1.13.
       @since PCC2 1.99.9, PCC 1.0.6
       @see Select, If (Elementary Function) */

    Tokenizer& tok = m_commandSource.tokenizer();

    /* Make labels */
    BytecodeObject::Label_t ift = bco.makeLabel();
    BytecodeObject::Label_t iff = bco.makeLabel();

    /* Read expression */
    tok.readNextToken();
    compileArgumentCondition(bco, scc, ift, iff);
    bco.addLabel(ift);

    /* Does this look like a one-liner? */
    bool oneliner = tok.checkAdvance("THEN");

    /* Single or multiple lines? */
    if (tok.getCurrentToken() != tok.tEnd) {
        /* Single line */
        if (!oneliner) {
            throw Error::expectKeyword("Then");
        }

        /* Compile 'Then' (will return CompiledStatement) */
        compile(bco, DefaultStatementCompilationContext(scc).setOneLineSyntax().withoutFlag(CompilationContext::LinearExecution));
        bco.addLabel(iff);

        /* Process result */
        return CompiledStatement;
    } else {
        /* Multiple lines */
        validateMultiline(scc);
        m_commandSource.readNextLine();

        /* Compile 'Then' part. */
        compileList(bco, DefaultStatementCompilationContext(scc).setBlockSyntax().withoutFlag(CompilationContext::LinearExecution));

        /* Compile 'Else' and 'Else If' parts. hadElse is true when we had an Else part,
           and do not allow another. 'iff' will always be placed at the Else part. */
        BytecodeObject::Label_t endif = bco.makeLabel();
        bool hadElse = false;
        while (1) {
            if (tok.checkAdvance("ELSE")) {
                bco.addJump(Opcode::jAlways, endif);
                bco.addLabel(iff);
                if (tok.checkAdvance("IF")) {
                    /* If / ... / Else If expr / ... */
                    ift = bco.makeLabel();
                    iff = bco.makeLabel();
                    compileArgumentCondition(bco, scc, ift, iff);
                    bco.addLabel(ift);
                    tok.checkAdvance("THEN");
                } else {
                    if (hadElse) {
                        throw Error::misplacedKeyword("Else");
                    }
                    hadElse = true;
                }
                parseEndOfLine();
                compileList(bco, DefaultStatementCompilationContext(scc).setBlockSyntax().withoutFlag(CompilationContext::LinearExecution));
            } else if (tok.checkAdvance("ENDIF")) {
                parseEndOfLine();
                bco.addLabel(endif);
                if (!hadElse) {
                    bco.addLabel(iff);
                }
                break;
            } else {
                throw Error::expectKeyword("Else", "EndIf");
            }
        }
        return CompiledBlock;
    }
}

/** Compile "Load" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileLoad(BytecodeObject& bco, const StatementCompilationContext& scc, bool mustSucceed)
{
    // ex IntStatementCompiler::compileLoad
    /* @q Load name:Str (Elementary Command), TryLoad name:Str (Elementary Command)
       Load a script.
       The parameter is a file name.
       That file is loaded and executed, as if its content were part of a subroutine.

       For %Load, it is an error if the file cannot be found.

       For %TryLoad, it is not an error if the file cannot be found,
       but errors during its execution are still reported
       (whereas <tt>Try Load file</tt> would "swallow" all errors).
       This makes it ideal for loading optional files.

       @since PCC2 1.99.9, PCC 1.0.6 */

    /*      <expr>
            sload
           [je 1F
            uthrow
       1H:] drop 1 */

    /* Parse it */
    m_commandSource.tokenizer().readNextToken();
    std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(m_commandSource.tokenizer()).parse());
    parseEndOfLine();

    /* Precompilation */
    bool precompiled = false;
    if (scc.hasFlag(scc.PreexecuteLoad)) {
        if (interpreter::expr::LiteralNode* lit = dynamic_cast<interpreter::expr::LiteralNode*>(node.get())) {
            if (afl::data::Value* value = lit->getValue()) {
                String_t fileName(toString(value, false));
                afl::base::Ptr<afl::io::Stream> file = scc.world().openLoadFile(fileName);
                if (file.get() != 0) {
                    // File opened successfully. Compile it.
                    // FIXME: deal with recursion. Right now, recursion is implicitly broken because compileFile does not set PreexecuteLoad.
                    // However, it is desirable to compile a nested file with PreexecuteLoad, too, as long as recursion is prevented.
                    scc.world().logListener().write(afl::sys::LogListener::Trace, "script.trace", afl::string::Format("Preloading \"%s\"...", fileName));
                    SubroutineValue subv(scc.world().compileFile(*file, bco.getOrigin(), m_optimisationLevel));
                    bco.addPushLiteral(&subv);
                    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 0);
                    precompiled = true;
                }
            }
        }
    }

    /* Generate code */
    if (!precompiled) {
        node->compileValue(bco, scc);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialLoad, 0);
        if (mustSucceed) {
            BytecodeObject::Label_t lab = bco.makeLabel();
            bco.addJump(Opcode::jIfEmpty, lab);
            bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialThrow, 0);
            bco.addLabel(lab);
        }
        bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    }

    return CompiledStatement;
}

/** Compile "On" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileOn(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileOn
    /* @q On event:Hook Do command (Elementary Command)
       @noproto
       | On event Do command
       |
       | On event Do
       |   commands
       | EndDo
       Execute command on event.
       Stores the specified command to be executed when the %event happens.

       The %event is an identifier or {ByName()} expression.
       Predefined identifiers for %event are listed <a href="int:index:type:hook">here</a>.

       You can define any number of commands for each event.
       You can also invent your own events, and trigger them using {RunHook}.

       @diff PCC 1.x allows canceling execution of event handlers registered later on
       using a command such as <tt>On event Do Return</tt>.
       This was never documented, and does not work in PCC2.

       @diff The multi-line form is supported since PCC2 2.0.8 and 2.40.8 (c2ng).

       @see RunHook
       @since PCC2 1.99.9, PCC 1.0.9 */

    /* Parse */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    compileNameString(bco, scc, "hook name");

    bool oneliner = tok.checkAdvance("DO");

    /* Context for embedded command: behaves like regular function body, Return not allowed. */
    class HookCompilationContext : public StatementCompilationContext {
     public:
        HookCompilationContext(World& world)
            : StatementCompilationContext(world)
            {
                withFlag(LocalContext);
                withFlag(LinearExecution);
            }
        // defaultCompileBreak/defaultCompileCleanup will throw because it has not parent SCC to dispatch to.
        void compileBreak(BytecodeObject& bco) const
            { defaultCompileBreak(bco); }
        void compileContinue(BytecodeObject& bco) const
            { defaultCompileContinue(bco); }
        void compileCleanup(BytecodeObject& /*bco*/) const
            { throw Error::misplacedKeyword("Return"); }
    };

    /* Compile embedded command */
    BCORef_t nbco = *new BytecodeObject();
    nbco->setIsProcedure(true);
    nbco->setFileName(bco.getFileName());
    nbco->setOrigin(bco.getOrigin());

    Result r;
    if (tok.getCurrentToken() != tok.tEnd) {
        /* Single line */
        if (!oneliner) {
            throw Error::expectKeyword("Do");
        }
        compile(*nbco, HookCompilationContext(scc.world()).setOneLineSyntax());
        r = CompiledStatement;
    } else {
        /* Multiple lines */
        validateMultiline(scc);
        m_commandSource.readNextLine();

        /* Compile content */
        compileList(*nbco, HookCompilationContext(scc.world()).setBlockSyntax());

        /* EndOn command */
        if (!tok.checkAdvance("ENDON")) {
            throw Error::expectKeyword("EndOn");
        }
        parseEndOfLine();
        r = CompiledBlock;
    }

    /* Compile this command */
    SubroutineValue subv(nbco);
    bco.addPushLiteral(&subv);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialAddHook, 0);
    finishBCO(*nbco, scc);

    return r;
}

/** Compile "Option" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileOption(BytecodeObject& /*bco*/, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileOption
    /* @q Option (Elementary Command)
       @noproto
       | Option opt(param), ...
       Set interpreter options.
       This command is an "escape" mechanism to give an instruction to the script interpreter.
       It does not by itself do something, but affect how the interpreter reads and executes your script.
       If the interpreter understands this instruction, it will honor it,
       otherwise the instruction will be ignored.

       <h2>General</h2>
       The %Option command should used be only at places it is intended for.
       It may not work correctly when used elsewhere.

       Since it usually interpreted when the script is read, not when it is executed,
       parameters to commands cannot be expressions, nor does it make sense to execute
       %Option conditionally within an %If or %Try.

       <h2>Option Encoding</h2>
       | Option Encoding("type")
       Defines the script encoding.
       If you have strings written in a particular character set, name that character set using this command.
       For example,
       | Option Encoding("koi8-r")
       says that you wrote your strings in a cyrillic character set.

       Place the command at the beginning of your script.
       It will affect all lines after it.

       This option is supported by PCC2 1.99.12 (desktop version), ignored by PCC2 Web and PCC 1.x.

       <h2>Option Optimize</h2>
       | Option Optimize(level)
       Set the optimisation level.
       The parameter is a numeric literal.
       When PCC2 reads the script and compiles it into an internal representation,
       it can perform some transformations that make the script quicker to execute.
       Possible optimisation levels are:
       - 0 (no optimisation, but some standard code selection intelligence is still used)
       - 1 (normal optimisation, default)
       - 2 (enable more expensive optimisations)
       - 3 (enable optimisations that may change behaviour in boundary case,
         e.g. generate different error messages than normal)
       - -1 (generate most naive code possible. This setting is not intended for normal
         use, but as a way out if I broke something and optimisation breaks your script.)

       As of PCC2 1.99.22, no level 2 or 3 optimisations are implemented.

       As of PCC2 2.40.6, level 2 enables code merging.
       This will reduce the precision of line numbers given in error messages and is thus not enabled by default.

       Place the command at the beginning of your script or subroutine.
       It will affect this script/subroutine and everything defined within,
       but not other subroutines following yours.

       This option is supported by PCC2 1.99.22.

       <h2>Option LocalSubs / LocalTypes</h2>
       | Option LocalSubs(flag)
       | Option LocalTypes(flag)
       Set availability of the {int:appendix:experimental|experimental features}
       Local Subroutines and Local Types, see there.
       The parameter is either 0 (off, default) or 1 (on).

       Place the command at the beginning of your script or subroutine.
       It will affect this script/subroutine and everything defined within,
       but not other subroutines following yours.

       These options are supported by PCC2 1.99.22.

       @since PCC2 1.99.9, PCC 1.0.19 */

    /* Command permitted only in multiline context, to refuse silly things
       such as "If 0 Then Option ...." */
    validateMultiline(scc);

    /* Parse it */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    while (1) {
        /* Read name */
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("option name");
        }
        String_t opname = tok.getCurrentString();
        tok.readNextToken();

        /* Process option */
        if (opname == "ENCODING") {
            /* "Encoding('string')" */
            if (!tok.checkAdvance(tok.tLParen)) {
                throw Error::expectSymbol("(");
            }
            if (tok.getCurrentToken() != tok.tString) {
                throw Error("Expecting string");
            }
            String_t encname = tok.getCurrentString();
            tok.readNextToken();
            if (!tok.checkAdvance(tok.tRParen)) {
                throw Error::expectSymbol(")");
            }

            /* Interpret it */
            afl::charset::Charset* cs = util::CharsetFactory().createCharset(encname);
            if (cs == 0) {
                throw Error("Unknown encoding, " + encname);
            }
            if (!m_commandSource.setCharsetNew(cs)) {
                throw Error::misplacedKeyword("Option Encoding");
            }
        } else if (opname == "LOCALTYPES") {
            /* "LocalTypes(0/1)" */
            m_allowLocalTypes = (parseOptionArgument(tok, 0, 1) != 0);
        } else if (opname == "LOCALSUBS") {
            /* "LocalSubs(0/1)" */
            m_allowLocalSubs = (parseOptionArgument(tok, 0, 1) != 0);
        } else if (opname == "OPTIMIZE") {
            /* "Optimize(0-3) */
            m_optimisationLevel = parseOptionArgument(tok, MIN_OPTIMISATION_LEVEL, MAX_OPTIMISATION_LEVEL);
        } else {
            /* Unrecognized option. Skip brace pair. */
            if (tok.checkAdvance(tok.tLParen)) {
                int level = 1;
                while (level != 0) {
                    if (tok.getCurrentToken() == tok.tLParen) {
                        ++level;
                    } else if (tok.getCurrentToken() == tok.tRParen) {
                        --level;
                    } else if (tok.getCurrentToken() == tok.tEnd) {
                        throw Error::expectSymbol(")");
                    } else {
                        // skip
                    }
                    tok.readNextToken();
                }
            }
        }

        /* Another one? */
        if (!parseNext(tok)) {
            break;
        }
    }

    return CompiledStatement;
}

/** Compile "Print" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compilePrint(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compilePrint
    /* @q Print (Elementary Command)
       @noproto
       | Print item, ...
       | Print #file:File, item, ...
       Print text to console or file.
       Evaluates all parameters, concatenates them to a string, and prints them to the console.
       EMPTY values are ignored (but if all values are EMPTY, no line is printed at all).

       With the second form, the line is written to the specified file.

       @since PCC2 1.99.9, PCC 1.0.6 */

    /* Parse it */
    afl::container::PtrVector<interpreter::expr::Node> nodes;
    m_commandSource.tokenizer().readNextToken();
    parseArgumentList(nodes);

    /* Check for "#fd" argument */
    size_t first = 0;
    interpreter::expr::SimpleNode* ex;
    if (nodes.size() != 0
        && (ex = dynamic_cast<interpreter::expr::SimpleNode*>(nodes[0])) != 0
        && ex->is(Opcode::maUnary, unFileNr))
    {
        first = 1;
        nodes[0]->compileValue(bco, scc);
    }

    /* Compile remaining arguments */
    if (nodes.size() == first) {
        afl::data::StringValue sv("");
        bco.addPushLiteral(&sv);
    } else {
        nodes[first]->compileValue(bco, scc);
        for (size_t i = first+1; i < nodes.size(); ++i) {
            nodes[i]->compileValue(bco, scc);
            bco.addInstruction(Opcode::maBinary, biConcatEmpty, 0);
        }
    }

    /* Compile action */
    if (first == 0) {
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialPrint, 0);
    } else {
        /* CC$Print #fd, text */
        bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$PRINT"));
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 2);
    }
    return CompiledStatement;
}

/** Compile "ReDim" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileReDim(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileReDim
    /* @q ReDim name(dims),... (Elementary Command)
       Resize an array.

       The %name is the name of an array variable.
       %dims are the new dimensions, as a list of integer expressions.

       Note that you can change the size of the array, but not the number of dimensions:
       a one-dimensional array will stay one-dimensional, and accept only %ReDim commands
       that specify one dimension.

       Current values in the array are kept if their position also exists in the new array.
       If you enlarge the array, new positions are filled with EMPTY.
       If you shrink the array, excess positons are deleted.

       For example:
       | Dim a(10)         % Make array with 10 elements
       | ReDim a(20)       % Make it have 20 elements

       Changing an array's first (or only) dimension is very efficient.
       Changing the shape of an array will have to move data around and therefore be slow.

       @since PCC2 1.99.22 */

    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    while (1) {
        /* Read name and compile it */
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("array name");
        }
        bco.addVariableReferenceInstruction(Opcode::maPush, tok.getCurrentString(), scc);
        tok.readNextToken();

        /* Read dimensions */
        if (!tok.checkAdvance(tok.tLParen)) {
            throw Error::expectSymbol("(");
        }
        int numDims = 0;
        while (1) {
            compileArgumentExpression(bco, scc);
            ++numDims;
            if (tok.checkAdvance(tok.tRParen)) {
                break;
            }
            if (!tok.checkAdvance(tok.tComma)) {
                throw Error::expectSymbol(",", ")");
            }
        }

        /* Do it */
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialResizeArray, uint16_t(numDims));
        if (!parseNext(tok)) {
            break;
        }
    }
    return CompiledStatement;
}

/** Compile "Return" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileReturn(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileReturn
    /* @q Return Optional value:Any (Elementary Command)
       Return from subroutine or function.

       If used within a {Sub|subroutine}, there must not be a parameter.
       The subroutine returns when this command is executed.

       If used within a {Function|function}, the parameter must be specified.
       The function returns when this command is executed, and gives the value to its caller.

       @since PCC2 1.99.9, PCC 1.0.6
       @see Sub, Function */

    /* Prepare */
    scc.compileCleanup(bco);

    /* Compile instruction */
    if (!bco.isProcedure()) {
        compileArgumentExpression(bco, scc);
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    } else {
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 0);
    }

    /* Must now be at end */
    parseEndOfLine();

    return CompiledStatement;
}

/** Compile "RunHook" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileRunHook(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileRunHook
    /* @q RunHook event:Hook (Elementary Command)
       Run hook commands.
       Executes the commands registered for the %event using {On}.
       The %event is an identifier or {ByName()} expression.
       If no commands are registered for that event, nothing happens.
       @since PCC2 1.99.9, PCC 1.0.9 */

    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    compileNameString(bco, scc, "hook name");
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialRunHook, 0);

    /* Most now be at end */
    parseEndOfLine();

    return CompiledStatement;
}

/** Compile a scope keyword statement ("Local", "Static", "Shared").
    \param bco [out] bytecode output
    \param scc [in] statement compilation context
    \param scope [in] the scope */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileScope(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope)
{
    // ex IntStatementCompiler::compileScope
    /* @q Local name [initializer],... (Elementary Command)
       Create a local variable.
       Same as <tt>{Dim} Local</tt>, see there.
       @since PCC2 1.99.8, PCC 1.0.6 */

    /* @q Shared name [initializer],... (Elementary Command)
       Create a local variable.
       Same as <tt>{Dim} Shared</tt>, see there.
       @since PCC2 1.99.8, PCC 1.0.6 */

    /* @q Static name [initializer],... (Elementary Command)
       Create a static variable.
       Same as <tt>{Dim} Static</tt>, see there.
       @since PCC2 1.99.8, PCC 1.0.6 */

    // Skip the keyword
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();

    // Struct, Sub or Function? Do not skip the keyword yet,
    // this is done by compileSub/compileStruct!
    if (tok.getCurrentToken() == tok.tIdentifier) {
        if (m_allowLocalSubs) {
            if (tok.getCurrentString() == "SUB") {
                return compileSub(bco, scc, true, scope);
            }
            if (tok.getCurrentString() == "FUNCTION") {
                return compileSub(bco, scc, false, scope);
            }
        }
        if (m_allowLocalTypes) {
            if (tok.getCurrentString() == "STRUCT") {
                return compileStruct(bco, scc, scope);
            }
        }
    }

    // Compile variable definition
    compileVariableDefinition(bco, scc, scope);
    return CompiledStatement;
}

/** Compile "Select Case" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileSelect(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileSelect
    /* @q Select (Elementary Command)
       @noproto
       | Select Case expr
       |   Case value, value...
       |     commands
       |   Case Is <= value
       |     commands
       |   Case Else
       |     commands
       | EndSelect
       Multi-part decision.
       The expression %expr is evaluated and compared to each of the %Case blocks.
       The first matching block's commands are executed.

       There can be any number of %Case branches, each of which lists a number of values to match.
       For example,
       | Case 1, 3, 5
       matches numbers one, three or five.
       Using the %Is keyword, you can also match on relations, as in
       | Case Is >= 9
       which matches all numbers greater or equal than nine.
       Each %Case can contain any number of selectors separated by comma.
       Although these examples all use integer numbers, you can also select on real numbers or strings.

       Cases are evaluated from top to bottom, the first matching one is taken.
       If no case matches, the <tt>Case Else</tt>, if present, is run.

       Values in %Case expressions should be constants, although this is not currently enforced.

       Example:
       | Select Case i
       |   Case 1
       |     Print "one"
       |   Case 2,3,4
       |     Print "two to four"
       |   Case Is &lt; 10
       |     Print "below ten, but not one to four"
       |   Case Else
       |     Print "anything else"
       | EndSelect

       @since PCC2 1.99.9, PCC 1.1.13
       @see If */

    /* The selector expression is placed on the stack.
       Each case has three relevant labels:
         ldo    .. yes, take this case (before block content)
         ldont  .. no, don't take this one (after block content)
         lout   .. we jump here after having taken one block
       We must keep the selector expression on the stack to evaluate the expressions,
       therefore we dup it each time. Before exiting the Select statement, we must drop
       it again. There are two ways of handling this:
       - drop it upon entry into each block
       - drop it after the Select
       For now, use way number one. This generates more code, but doesn't require us
       to provide own Break/Continue/Return statements. */

    /* Parse head */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    if (!tok.checkAdvance("CASE")) {
        throw Error::expectKeyword("Case");
    }
    compileArgumentExpression(bco, scc);
    parseEndOfLine();

    /* Is it valid? */
    if (scc.hasFlag(scc.RefuseBlocks)) {
        throw Error::invalidMultiline();
    }

    /* Find first case */
    while (1) {
        m_commandSource.readNextLine();
        if (m_commandSource.isEOF()) {
            /* End of file */
            throw Error("Unexpected end of script");
        } else if (tok.getCurrentToken() == tok.tEnd) {
            /* Ok, blank line */
        } else if (tok.checkAdvance("ENDSELECT")) {
            /* Simple quick case */
            parseEndOfLine();
            bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            return CompiledBlock;
        } else if (tok.checkAdvance("CASE")) {
            /* Start work */
            break;
        } else {
            throw Error::expectKeyword("Case");
        }
    }

    /* Compile cases. At entrance into this loop, we have parsed the "Case" keyword.
       We exit the loop when seeing EndSelect. */
    BytecodeObject::Label_t lout = bco.makeLabel();
    while (1) {
        if (tok.checkAdvance("ELSE")) {
            /* Special case, must be last one. */
            parseEndOfLine();
            m_commandSource.readNextLine();
            bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            compileList(bco, DefaultStatementCompilationContext(scc).setBlockSyntax().withoutFlag(CompilationContext::LinearExecution));
            bco.addJump(Opcode::jAlways, lout);
            if (!tok.checkAdvance("ENDSELECT")) {
                throw Error::expectKeyword("EndSelect");
            }
            break;
        } else {
            /* Possibly multi-part condition. */
            BytecodeObject::Label_t ldo = bco.makeLabel();
            BytecodeObject::Label_t ldont = bco.makeLabel();
            compileSelectCondition(bco, scc, ldo);
            bco.addJump(Opcode::jAlways, ldont);
            bco.addLabel(ldo);
            bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
            compileList(bco, DefaultStatementCompilationContext(scc).setBlockSyntax().withoutFlag(CompilationContext::LinearExecution));
            bco.addJump(Opcode::jAlways, lout);
            bco.addLabel(ldont);
            if (tok.checkAdvance("ENDSELECT")) {
                break;
            }
            if (!tok.checkAdvance("CASE")) {
                throw Error::expectKeyword("EndSelect", "Case");
            }
        }
    }
    parseEndOfLine();
    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
    bco.addLabel(lout);
    return CompiledBlock;
}

/** Compile "SelectionExec" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context
    \param proc [in] true if this is going to be a procedure (Sub), false if it's a function */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileSelectionExec(BytecodeObject& bco, const StatementCompilationContext& /*scc*/)
{
    // ex IntStatementCompiler::compileSelectionExec
    /* @q SelectionExec (Elementary Command)
       @noproto
       | SelectionExec [target :=] expr
       Modify selection.
       Executes a selection expression, and assigns the result to %target
       (or the current selection, if %target is omitted).

       The %target must be a selection layer name, namely
       - %Current to name the current layer
       - %A .. %H for a named layer

       For a description of the selection expression, see the
       <a href="pcc2:selectionmgr">Selection Manager</a> help page.

       @since PCC2 1.99.10, PCC 1.0.10 */
    int target = 0;
    String_t expr;

    /* Read expression or target */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();

    SelectionExpression::compile(tok, expr);
    if (tok.checkAdvance(tok.tAssign) || tok.checkAdvance(tok.tEQ)) {
        /* It is an assignment */
        if (expr.size() != 1) {
            throw Error::notAssignable();
        }

        if (expr[0] == SelectionExpression::opCurrent) {
            target = 0;
        } else if (expr[0] >= SelectionExpression::opFirstLayer && expr[0] < SelectionExpression::opFirstLayer + SelectionExpression::NUM_SELECTION_LAYERS) {
            target = expr[0] - SelectionExpression::opFirstLayer + 1;
        } else {
            throw Error::notAssignable();
        }

        /* Read actual expression */
        expr.clear();
        SelectionExpression::compile(tok, expr);
    }
    parseEndOfLine();

    /* Generate code for a call to "CC$SELECTIONEXEC target, expr" */
    afl::data::StringValue sv(expr);
    bco.addInstruction(Opcode::maPush, Opcode::sInteger, uint16_t(target));
    bco.addPushLiteral(&sv);
    bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$SELECTIONEXEC"));
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall + Opcode::miIMRefuseFunctions, 2);

    return CompiledStatement;
}

/** Compile "Sub/Function" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context
    \param proc [in] true if this is going to be a procedure (Sub), false if it's a function
    \param scope [in] Target scope */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileSub(BytecodeObject& bco, const StatementCompilationContext& scc, bool proc, Opcode::Scope scope)
{
    // ex IntStatementCompiler::compileSub
    /* @q Sub (Elementary Command)
       @noproto
       | Sub name(param, param, Optional param, rest())
       |   commands
       | EndSub
       Define a subroutine.
       The subroutine can take parameters.
       The names of these parameters are specified in parentheses after the subroutine name.

       If one parameter is preceded by %Optional,
       all parameters following it are optional and can be omitted by the caller.
       They will report EMPTY when read.

       The last parameter can be followed by <tt>()</tt>.
       This allows the caller to specify any number of values (including none at all) for this parameter,
       which will be packed into an array (making this a "varargs subroutine", for C programmers).

       A subroutine can be called by listing its name, followed by the parameters:
       | Sub test(a)
       |   Print a
       | EndSub
       | test "hello, world"

       If there already is a subroutine or function with the same name as this subroutine,
       it will be replaced by the new definition.

       @diff PCC 1.x does not support the <tt>rest()</tt> form.
       @see Function
       @since PCC2 1.99.9, PCC 1.0.6 */

    /* @q Function (Elementary Command)
       @noproto
       | Function name(param, param, Optional param, rest())
       |   commands
       | EndFunction
       Define a function.
       The function can take parameters.
       The names of these parameters are specified in parentheses after the function name.

       If one parameter is preceded by %Optional,
       all parameters following it are optional and can be omitted by the caller.
       They will report EMPTY when read.

       The last parameter can be followed by <tt>()</tt>.
       This allows the caller to specify any number of values (including none at all) for this parameter,
       which will be packed into an array (making this a "varargs function", for C programmers).

       A function can be called from expressions, by writing its name followed by parameters in parentheses.
       It will be called when the expression is evaluated, and its {Return} value be inserted into the expression.
       | Function twice(a)
       |   Return 2*a
       | EndSub
       | Print twice(17)      % prints 34
       Note that if a function takes no parameters, an empty pair of parentheses must still be specified
       (<tt>func()</tt>) to call the function.

       If there already is a subroutine or function with the same name as this function,
       it will be replaced by the new definition.

       @see Sub
       @since PCC2 1.99.9 */

    if (scc.hasFlag(scc.RefuseBlocks)) {
        throw Error::invalidMultiline();
    }

    /* Read function name */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    if (tok.getCurrentToken() != tok.tIdentifier) {
        throw Error::expectIdentifier(proc ? "subroutine name" : "function name");
    }
    const String_t name = tok.getCurrentString();
    validateName(scc, name);
    tok.readNextToken();

    /* Create new BCO */
    BCORef_t nbco = *new BytecodeObject();
    nbco->setIsProcedure(proc);
    nbco->setSubroutineName(name);
    nbco->setFileName(bco.getFileName());
    nbco->setOrigin(bco.getOrigin());

    /* Read parameters */
    if (tok.checkAdvance(tok.tLParen) && !tok.checkAdvance(tok.tRParen)) {
        bool optional = false;
        while (1) {
            if (tok.checkAdvance("OPTIONAL")) {
                if (optional) {
                    throw Error::misplacedKeyword("Optional");
                }
                optional = true;
            }
            if (tok.getCurrentToken() != tok.tIdentifier) {
                throw Error::expectIdentifier("parameter name");
            }
            String_t name = tok.getCurrentString();
            validateName(scc, name);
            tok.readNextToken();

            if (tok.checkAdvance(tok.tLParen)) {
                /* Varargs: must have two closing parens now, one for the varargs
                   thing, one to close the parameter list */
                if (!tok.checkAdvance(tok.tRParen) || !tok.checkAdvance(tok.tRParen)) {
                    throw Error::expectSymbol(")");
                }
                nbco->addLocalVariable(name);
                nbco->setIsVarargs(true);
                break;
            }

            nbco->addArgument(name, optional);
            if (tok.checkAdvance(tok.tRParen)) {
                break;
            }
            if (!tok.checkAdvance(tok.tComma)) {
                throw Error::expectSymbol(",", ")");
            }
        }
    }
    parseEndOfLine();

    /* Header has been read; now read content. Use a new compilation context
       and a new statement compiler. */
    m_commandSource.readNextLine();
    StatementCompiler subSC(*this);
    subSC.compileList(*nbco, DefaultStatementCompilationContext(scc.world())
                      .setBlockSyntax()
                      .withFlag(CompilationContext::LocalContext)
                      .withFlag(CompilationContext::LinearExecution));

    /* If it is a function, make sure it returns anything */
    if (!proc) {
        nbco->addPushLiteral(0);
    }
    subSC.finishBCO(*nbco, scc);

    if (!tok.checkAdvance(proc ? "ENDSUB" : "ENDFUNCTION")) {
        throw Error::expectKeyword(proc ? "EndSub" : "EndFunction");
    }
    tok.checkAdvance(name.c_str());
    parseEndOfLine();

    /* Routine has been compiled. Generate code. */
    compileSubroutineDefinition(bco, scc, nbco, name, scope);

    return CompiledBlock;
}

/** Compile "Struct" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context
    \param scope [in] Target scope */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileStruct(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope)
{
    // ex IntStatementCompiler::compileStruct
    /* @q Struct (Elementary Command)
       @noproto
       | Struct name
       |   field [initializer], field [initializer], ...
       | EndStruct
       Define a structure.
       A structure is a blueprint for a series of objects with an identical set of properties.

       Lines after the %Struct keyword define the properties (fields) that are part of the structure.
       Each line defines one or more fields, separated by commas.

       A structure is instantiated with {Dim}:
       | Struct Pair
       |   First, Second
       | EndStruct
       | Dim p As Pair
       | p->First := 1          % Set a field
       | With p Do Second := 2  % Alternative version

       Each field can have an optional initializer.
       See {Dim} for allowed forms of initializers.
       The initializer defines the initial value of the structure field.
       If no initializer is given, the field starts out EMPTY.

       Internally, a structure is implemented as a <em>constructor function</em>.
       Instead of using <tt>Dim...As</tt>, you could also call the constructor function directly:
       <tt>p := Pair()</tt>.

       @see Dim
       @since PCC2 1.99.19 */

    if (scc.hasFlag(scc.RefuseBlocks)) {
        throw Error::invalidMultiline();
    }

    /* Read structure name */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    if (tok.getCurrentToken() != tok.tIdentifier) {
        throw Error::expectIdentifier("structure name");
    }

    String_t name = tok.getCurrentString();
    validateName(scc, name);
    if (identifyType(name) != tkNone) {
        throw Error("\"" + name + "\" is a reserved type name");
    }
    tok.readNextToken();
    parseEndOfLine();

    /* We create a structure and a constructor function */
    StructureType typeValue(*new StructureTypeData());
    BCORef_t ctorBCO = *new BytecodeObject();
    ctorBCO->setIsProcedure(false);
    ctorBCO->setFileName(bco.getFileName());
    ctorBCO->setOrigin(bco.getOrigin());
    ctorBCO->addLineNumber(m_commandSource.getLineNumber());
    ctorBCO->addPushLiteral(&typeValue);
    ctorBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialInstance, 0);
    ctorBCO->setSubroutineName(name);

    /* Read content */
    bool reading = true;
    while (reading) {
        m_commandSource.readNextLine();
        if (m_commandSource.isEOF()) {
            throw Error("Unexpected end of script");
        }
        if (tok.getCurrentToken() == tok.tEnd) {
            /* Blank line */
        } else if (tok.getCurrentToken() == tok.tIdentifier) {
            /* Identifier */
            ctorBCO->addLineNumber(m_commandSource.getLineNumber());
            switch (lookupKeyword(tok.getCurrentString())) {
             case kwEndStruct:
                tok.readNextToken();
                parseEndOfLine();
                reading = false;
                break;
             case kwNone:
                /* Read variables */
                while (1) {
                    /* Read name */
                    if (tok.getCurrentToken() != tok.tIdentifier) {
                        throw Error::expectIdentifier("variable name");
                    }
                    String_t field = tok.getCurrentString();
                    validateName(scc, field);
                    if (typeValue.getType()->names().getIndexByName(field) != afl::data::NameMap::nil)
                        throw Error("Duplicate field name");
                    typeValue.getType()->names().add(field);
                    tok.readNextToken();

                    /* Read value */
                    if (compileInitializer(*ctorBCO,
                                           DefaultStatementCompilationContext(scc.world())
                                           .setBlockSyntax()
                                           .withFlag(CompilationContext::LocalContext)
                                           .withFlag(CompilationContext::LinearExecution)))
                    {
                        ctorBCO->addInstruction(Opcode::maStack, Opcode::miStackDup, 1);
                        ctorBCO->addInstruction(Opcode::maMemref, Opcode::miIMPop, ctorBCO->addName(field));
                    }
                    if (!parseNext(tok)) {
                        break;
                    }
                }
                break;
             default:
                throw Error::misplacedKeyword(tok.getCurrentString().c_str());
            }
        } else {
            /* Error */
            throw Error("Invalid structure definition");
        }
    }

    /* Finish up */
    ctorBCO->addInstruction(Opcode::maSpecial, Opcode::miSpecialReturn, 1);
    finishBCO(*ctorBCO, scc);

    /* Generate code */
    compileSubroutineDefinition(bco, scc, ctorBCO, name, scope);

    return CompiledBlock;
}

/** Compile "Try" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileTry(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileTry
    /* @q Try (Elementary Command)
       @noproto
       | Try command
       |
       | Try
       |   commands
       | Else
       |   commands
       | EndTry
       Catch errors.
       The commands after the %Try are executed.
       If any command produces an error, either by doing something bad such as dividing by zero
       or using an undefined property, or by using the {Abort} command, the %Else part is executed.
       If there is no %Else part, the error is silently ignored.

       In any case, the error message is assigned to the {System.Err} variable where it can be examined.

       @diff In PCC 1.x, {System.Err} is a global property.
       In PCC2, {System.Err} is a global variable, and you can define a local version of it
       to avoid modifying the global one.

       @since PCC2 1.99.9, PCC 1.0.6 */

    Tokenizer& tok = m_commandSource.tokenizer();

    /* Compilation context */
    struct TryStatementCompilationContext : public StatementCompilationContext {
        TryStatementCompilationContext(const StatementCompilationContext& parent)
            : StatementCompilationContext(parent)
            {
                withoutFlag(LinearExecution);
            }
        void compileContinue(BytecodeObject& bco) const
            {
                bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
                defaultCompileContinue(bco);
            }
        void compileBreak(BytecodeObject& bco) const
            {
                bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
                defaultCompileBreak(bco);
            }
        void compileCleanup(BytecodeObject& bco) const
            {
                defaultCompileCleanup(bco);
            }
    };

    /* Make labels */
    BytecodeObject::Label_t lcatch = bco.makeLabel();
    BytecodeObject::Label_t lend = bco.makeLabel();

    bco.addJump(Opcode::jCatch, lcatch);

    if (tok.readNextToken() == tok.tEnd) {
        /* Multi-line */
        validateMultiline(scc);
        m_commandSource.readNextLine();
        compileList(bco, TryStatementCompilationContext(scc).setBlockSyntax());
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        if (tok.checkAdvance("ELSE")) {
            bco.addJump(Opcode::jAlways, lend);
            bco.addLabel(lcatch);
            bco.addVariableReferenceInstruction(Opcode::maPop, "SYSTEM.ERR", scc);
            parseEndOfLine();
            m_commandSource.readNextLine();
            compileList(bco, TryStatementCompilationContext(scc).setBlockSyntax());
            bco.addLabel(lend);
        } else {
            bco.addJump(Opcode::jAlways, lend);
            bco.addLabel(lcatch);
            bco.addVariableReferenceInstruction(Opcode::maPop, "SYSTEM.ERR", scc);
            bco.addLabel(lend);
        }
        if (!tok.checkAdvance("ENDTRY")) {
            throw Error::expectKeyword("EndTry");
        }
        parseEndOfLine();
        return CompiledBlock;
    } else {
        /* One-liner */
        compile(bco, TryStatementCompilationContext(scc).setOneLineSyntax());
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialUncatch, 0);
        bco.addJump(Opcode::jAlways, lend);
        bco.addLabel(lcatch);
        bco.addVariableReferenceInstruction(Opcode::maPop, "SYSTEM.ERR", scc);
        bco.addLabel(lend);
        return CompiledStatement;
    }
}

/** Compile "UseKeymap" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileUseKeymap(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileUseKeymap
    /* @q UseKeymap name:Keymap (Global Command)
       Temporarily enable a secondary keymap.
       The next keypress will be processed according to the specified keymap
       instead of the normal keymap for the current place.
       This way, you can create multi-keystroke commands.

       For example,
       | CreateKeymap CtrlXMap
       | Bind CtrlXMap 'C-s' := 'SaveGame'
       will create a keymap %CtrlXMap in which <kbd>Ctrl-S</kbd> invokes the {SaveGame} command.
       You can now bind that keymap to a key:
       | Bind ControlScreen 'C-x' := 'UseKeymap CtrlXMap'
       Now, the key sequence <kbd>Ctrl-X Ctrl-S</kbd> will save the game from any control screen.

       Only one %UseKeymap command can be active at a time.
       A second command will cancel the first.

       This command does not wait for the keystroke to actually occur; it
       immediately proceeds execution of the script. The secondary keymap
       is used when PCC is waiting for input next time. As a reminder of
       the temporarily changed keybindings, a pop-up message will occur
       after little idle time, or when a key is pressed which is
       not bound in the keymap. As a quick way out, ESC cancels the
       secondary keymap, unless ESC is bound in it.

       It is recommended that you only bind direct invocations of
       %UseKeymap to keys. In particular, the <a href="pcc2:keymap">keymap debugger</a>
       can then help you to look at these alternate keymaps. Although it is
       possible to call %UseKeymap from subroutines, you should avoid that
       if you can. In particular, you should not call any complicated
       user-interface command after %UseKeymap; this will not always do
       what you want.

       @since PCC2 1.99.22, PCC 1.1.10 */
    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();
    compileNameString(bco, scc, "keymap name");
    bco.addInstruction(Opcode::maUnary, unKeyLookup, 0);

    /* For simplicity, push prefix */
    bco.addVariableReferenceInstruction(Opcode::maPush, "UI.PREFIX", scc);

    /* Call worker */
    bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName("CC$USEKEYMAP"));
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall, 2);

    /* Finish */
    parseEndOfLine();
    return CompiledStatement;
}

/** Compile "With" statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileWith(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileWith
    /* @q With (Elementary Command)
       @noproto
       | With obj:Obj Do command
       |
       | With obj:Obj [Do]
       |   commands
       | EndWith
       Evaluate command in object context.
       The expression %obj specifies an object, such as a planet (<tt>Planet(14)</tt>).
       That object's context is activated, and all commands are executed within it.
       For example, within a planet context, %SetFCode would change the planet's friendly code,
       and the %FCode property would return it.

       @since PCC2 1.99.9, PCC 1.0.6 */

    Tokenizer& tok = m_commandSource.tokenizer();
    tok.readNextToken();

    /* Compilation context */
    struct WithStatementCompilationContext : public StatementCompilationContext {
        WithStatementCompilationContext(const StatementCompilationContext& parent)
            : StatementCompilationContext(parent)
            {
                withoutFlag(LocalContext);
                withContextProvider(0);
            }
        void compileContinue(BytecodeObject& bco) const
            {
                bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
                defaultCompileContinue(bco);
            }
        void compileBreak(BytecodeObject& bco) const
            {
                bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
                defaultCompileBreak(bco);
            }
        void compileCleanup(BytecodeObject& bco) const
            {
                defaultCompileCleanup(bco);
            }
    };

    /* Expression */
    compileArgumentExpression(bco, scc);
    bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialWith, 0);

    /* Check remaining form */
    bool oneliner = tok.checkAdvance("DO");
    if (tok.getCurrentToken() == tok.tEnd) {
        /* Multi-line */
        validateMultiline(scc);
        m_commandSource.readNextLine();
        compileList(bco, WithStatementCompilationContext(scc).setBlockSyntax());
        if (!tok.checkAdvance("ENDWITH")) {
            throw Error::expectKeyword("EndWith");
        }
        parseEndOfLine();
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        return CompiledBlock;
    } else {
        /* One line */
        if (!oneliner) {
            throw Error::expectKeyword("Do");
        }
        compile(bco, WithStatementCompilationContext(scc).setOneLineSyntax());
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialEndWith, 0);
        return CompiledStatement;
    }
}

/** Compile expression statement.
    \param bco [out] bytecode output
    \param scc [in] statement compilation context */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileExpressionStatement(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileExpressionStatement(BytecodeObject& bco, const IntStatementCompilationContext& scc)
    /* Parse expression */
    std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(m_commandSource.tokenizer()).parse());
    if (m_commandSource.tokenizer().getCurrentToken() != Tokenizer::tEnd) {
        throw Error::garbageAtEnd(true);
    }

    /* If the topmost node is a comparison for equality, compile an assignment instead. */
    if (interpreter::expr::CaseNode* cen = dynamic_cast<interpreter::expr::CaseNode*>(node.get())) {
        if (interpreter::expr::Node* nn = cen->convertToAssignment()) {
            node.reset(nn);
        }
    }

    /* Compile it */
    if (scc.hasFlag(CompilationContext::ExpressionsAreStatements)) {
        node->compileEffect(bco, scc);
        return CompiledStatement;
    } else {
        node->compileValue(bco, scc);
        return CompiledExpression;
    }
}

/** Compile procedure call. Assumes current token being the procedure name.
    \param bco [out] Bytecode goes here
    \param scc [in] Compilation flags */
interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileProcedureCall(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileProcedureCall(BytecodeObject& bco, const IntStatementCompilationContext& scc)
    /* First is an identifier. */
    Tokenizer& tok = m_commandSource.tokenizer();
    String_t name = tok.getCurrentString();
    tok.readNextToken();

    /* Compile args */
    afl::container::PtrVector<interpreter::expr::Node> args;
    parseArgumentList(args);
    for (size_t i = 0; i != args.size(); ++i) {
        args[i]->compileValue(bco, scc);
    }

    /* Call */
    bco.addVariableReferenceInstruction(Opcode::maPush, name, scc);
    bco.addInstruction(Opcode::maIndirect, Opcode::miIMCall + Opcode::miIMRefuseFunctions, uint16_t(args.size()));

    return CompiledStatement;
}

interpreter::StatementCompiler::Result
interpreter::StatementCompiler::compileLoopBody(BytecodeObject& bco, StatementCompilationContext& subcc)
{
    Tokenizer& tok = m_commandSource.tokenizer();
    bool oneliner = tok.checkAdvance("DO");
    Result result;
    if (tok.getCurrentToken() != tok.tEnd) {
        /* Single line */
        if (!oneliner) {
            throw Error::expectKeyword("Do");
        }
        subcc.setOneLineSyntax();
        compile(bco, subcc);
        result = CompiledStatement;
    } else {
        /* Multi-line */
        validateMultiline(subcc);
        subcc.setBlockSyntax();
        m_commandSource.readNextLine();
        compileList(bco, subcc);
        if (!tok.checkAdvance("NEXT")) {
            throw Error::expectKeyword("Next");
        }
        // FIXME: 'Next i' syntax?
        parseEndOfLine();
        result = CompiledBlock;
    }
    return result;
}

/** Compile variable definition.
    Reads a list of variables and initializers.
    Tokenizer starts looking at first variable name.
    \param bco [in/out] Bytecode goes here
    \param scc [in] Statement compilation context
    \param scope [in] Scope for the variables */
void
interpreter::StatementCompiler::compileVariableDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope)
{
    // ex IntStatementCompiler::compileVariableDefinition
    /* Read variables */
    Tokenizer& tok = m_commandSource.tokenizer();
    while (1) {
        /* Read name */
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("variable name");
        }
        String_t name = tok.getCurrentString();
        if (scope == Opcode::sShared) {
            name = stripPrefix(name, "GLOBAL.");
        }
        validateName(scc, name);
        tok.readNextToken();

        /* Read value */
        bool isNull = !compileInitializer(bco, scc);

        /* Optimisation: if this is going to be a local variable, and we are linearly
           executing in local context, we can pre-allocate this name, to allow future
           references to use the address instead of a costly name lookup. We only do
           this if the variable hasn't been mentioned yet, to avoid retroactively turning
           global references into locals. This is overly pessimistic because it prevents
           things like 'Local name = Ship(Id).Name' from being optimized ('name' has
           already been mentioned by the above compileExpression before this check,
           as a member name), but it should be safe.

           Note that a variable can also be used by code called from this subroutine.
           This is checked by hasUserCall(). */
        if (m_optimisationLevel >= 0
            && scope == Opcode::sLocal
            && scc.hasFlag(CompilationContext::LinearExecution)
            && scc.hasFlag(CompilationContext::LocalContext)
            && !bco.hasName(name)
            && !bco.hasUserCall())
        {
            /* Optimized version */
            if (bco.hasLocalVariable(name)) {
                /* We know that this is a duplicate, so throw away the value */
                if (!isNull) {
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
                }
                Error e(afl::string::Format("Duplicate local variable name '%s'", name));
                m_commandSource.addTraceTo(e, afl::string::Translator::getSystemInstance());
                scc.world().logError(afl::sys::LogListener::Warn, e);
            } else {
                /* We know that this is a new variable, so initialize it */
                bco.addLocalVariable(name);
                if (!isNull) {
                    bco.addVariableReferenceInstruction(Opcode::maPop, name, scc);
                }
            }
        } else {
            /* General version */
            if (isNull) {
                bco.addPushLiteral(0);
            }
            bco.addInstruction(Opcode::maDim, scope, bco.addName(name));
        }

        if (!parseNext(tok)) {
            break;
        }
    }
}

/** Compile optional initializer. If an initializer is present, compiles it and returns true.
    If no initializer is present, returns false.
    \param bco [out] Bytecode goes here
    \param scc [in] Compilation flags */
bool
interpreter::StatementCompiler::compileInitializer(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileInitializer
    // Initializer forms:
    //   = expr
    //   as type
    //   [as type = expr]
    //   ( dimension )
    //   ( dimension ) as type
    Tokenizer& tok = m_commandSource.tokenizer();
    if (tok.checkAdvance(tok.tLParen)) {
        int n = 0;
        while (1) {
            compileArgumentExpression(bco, scc);
            ++n;
            if (tok.checkAdvance(tok.tRParen)) {
                break;
            }
            if (!tok.checkAdvance(tok.tComma)) {
                throw Error::expectSymbol(",", ")");
            }
        }
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewArray, uint16_t(n));
        if (tok.checkAdvance("AS")) {
            // Type initializer
            if (tok.getCurrentToken() != tok.tIdentifier) {
                throw Error::expectIdentifier("type name");
            }
            String_t typeName = tok.getCurrentString();
            tok.readNextToken();
            if (identifyType(typeName) != tkAny) {
                // Compile to a loop that initializes the array, by iterating from the end
                // (this means it is reallocated only once).
                std::vector<BytecodeObject::Label_t> label_skip;
                std::vector<BytecodeObject::Label_t> label_loop;
                for (int i = 0; i < n; ++i) {
                    label_skip.push_back(bco.makeLabel());
                    label_loop.push_back(bco.makeLabel());
                }

                // Loop heads:
                //     dup N         (duplicate array)
                //     pushint n     (index)
                //     barraydim
                //     jf skipn
                //  loopn:
                //     udec
                for (int i = 0; i < n; ++i) {
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDup, uint16_t(i));
                    bco.addInstruction(Opcode::maPush,  Opcode::sInteger, uint16_t(n - i));
                    bco.addInstruction(Opcode::maBinary, biArrayDim, 0);
                    bco.addJump(Opcode::jIfFalse | Opcode::jIfEmpty, label_skip[n-1-i]);
                    bco.addLabel(label_loop[n-1-i]);
                    bco.addInstruction(Opcode::maUnary, unDec, 0);
                }

                // Loop body:
                //     dup 2N        (duplicate indexes)
                //     <initializer>
                //     dup 2N+1      (duplicate array)
                //     popind N
                for (int i = 0; i < n; ++i) {
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDup, uint16_t(2*i));
                }
                if (!compileTypeInitializer(bco, scc, typeName)) {
                    // Cannot happen, compileTypeInitializer() returns false only for tkAny
                    bco.addPushLiteral(0);
                }
                bco.addInstruction(Opcode::maStack, Opcode::miStackDup, uint16_t(2*n+1));
                bco.addInstruction(Opcode::maIndirect, Opcode::miIMPop, uint16_t(n));

                // Loop tails:
                //     jt loopn
                //  skipn:
                //     drop 1
                for (int i = 0; i < n; ++i) {
                    bco.addJump(Opcode::jIfTrue, label_loop[i]);
                    bco.addLabel(label_skip[i]);
                    bco.addInstruction(Opcode::maStack, Opcode::miStackDrop, 1);
                }
            }
        }
        return true;
    } else if (tok.checkAdvance(tok.tEQ) || tok.checkAdvance(tok.tAssign)) {
        compileArgumentExpression(bco, scc);
        return true;
    } else if (tok.checkAdvance("AS")) {
        if (tok.getCurrentToken() != tok.tIdentifier) {
            throw Error::expectIdentifier("type name");
        }
        String_t typeName = tok.getCurrentString();
        tok.readNextToken();
        return compileTypeInitializer(bco, scc, typeName);
    } else {
        return false;
    }
}

/** Compile type initializer. */
bool
interpreter::StatementCompiler::compileTypeInitializer(BytecodeObject& bco, const StatementCompilationContext& scc, const String_t& typeName)
{
    // ex IntStatementCompiler::compileTypeInitializer
    switch (identifyType(typeName)) {
     case tkNone:
        // user-defined type: call constructor
        if (m_allowLocalTypes) {
            bco.addVariableReferenceInstruction(Opcode::maPush, typeName, scc);
        } else {
            bco.addInstruction(Opcode::maPush, Opcode::sNamedShared, bco.addName(typeName));
        }
        bco.addInstruction(Opcode::maIndirect, Opcode::miIMLoad, 0);
        return true;
     case tkAny:
        // ANY: don't initialize, keep initialized to null
        return false;
     case tkInteger:
        // INTEGER: initialize to 0
        bco.addInstruction(Opcode::maPush, Opcode::sInteger, 0);
        return true;
     case tkFloat:
        // FLOAT: initialize to 0.0
        {
            afl::data::FloatValue fv(0.0);
            bco.addPushLiteral(&fv);
        }
        return true;
     case tkString:
        // STRING: initialize to ""
        {
            afl::data::StringValue sv("");
            bco.addPushLiteral(&sv);
        }
        return true;
     case tkHash:
        // HASH: initialize with new hash
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialNewHash, 0);
        return true;
    }
    return false;
}

/** Compile argument expression. Parse one expression, and compile it
    into a value. Expects current token being first of expression.
    Leaves current token the one after the expression.
    \param bco [out] Bytecode goes here
    \param scc [in] Compilation flags */
void
interpreter::StatementCompiler::compileArgumentExpression(BytecodeObject& bco, const StatementCompilationContext& scc)
{
    // ex IntStatementCompiler::compileArgumentExpression
    std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(m_commandSource.tokenizer()).parse());
    node->compileValue(bco, scc);
}

/** Compile argument condition. Parse one expression, and compile it
    into a yes/no decision. Expects current token being first of expression.
    Leaves current token the one after the expression.
    \param bco [out] Bytecode goes here
    \param scc [in] Compilation flags
    \param ift [in] If result is true, generate jump here.
    \param iff [in] If result is false (or empty), generate jump here. */
void
interpreter::StatementCompiler::compileArgumentCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff)
{
    // ex IntStatementCompiler::compileArgumentCondition
    std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(m_commandSource.tokenizer()).parse());
    node->compileCondition(bco, scc, ift, iff);
}

/** Compile condition in "Case". Expects current token being the one following "Case".
    Leaves input pointer at end of line (=end of condition).
    \param bco [out] Bytecode goes here
    \param scc [in] Compilation flags
    \param ldo [in] Generated code jumps here if the case matches */
void
interpreter::StatementCompiler::compileSelectCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ldo)
{
    // ex IntStatementCompiler::compileSelectCondition
    Tokenizer& tok = m_commandSource.tokenizer();
    while (1) {
        uint8_t relation = biCompareEQ_NC;
        if (tok.checkAdvance("IS")) {
            /* Relation sign plus expression */
            if (tok.checkAdvance(tok.tGT)) {
                relation = biCompareGT_NC;
            } else if (tok.checkAdvance(tok.tGE)) {
                relation = biCompareGE_NC;
            } else if (tok.checkAdvance(tok.tLT)) {
                relation = biCompareLT_NC;
            } else if (tok.checkAdvance(tok.tLE)) {
                relation = biCompareLE_NC;
            } else if (tok.checkAdvance(tok.tNE)) {
                relation = biCompareNE_NC;
            } else if (tok.checkAdvance(tok.tEQ)) {
                relation = biCompareEQ_NC;
            } else {
                throw Error("Expecting relation");
            }
        }
        /* Single expression */
        bco.addInstruction(Opcode::maStack, Opcode::miStackDup, 0);
        compileArgumentExpression(bco, scc);
        bco.addInstruction(Opcode::maBinary, relation, 0);
        bco.addJump(Opcode::jIfTrue | Opcode::jPopAlways, ldo);

        if (!parseNext(tok)) {
            break;
        }
    }
}

/** Compile a name.
    \param bco Target BCO
    \param scc Statement compilation context
    \param ttl Name for error messages

    This is used for keymap names and hook names.
    Before 2.0.8 / 2.40.8, keymap/hook names always were symbols.
    This made it impossible to write a function that takes a keymap as parameter without resorting to Eval,
    although the bytecode would permit that. */
void
interpreter::StatementCompiler::compileNameString(BytecodeObject& bco, const StatementCompilationContext& scc, const char* ttl)
{
    /* @q ByName():Keymap (Elementary Function)
       @noproto
       | ByName(name:Str):Keymap
       | ByName(name:Str):Hook
       Commands that operate on keymaps and hooks take the keymap or hook name as an identifier.
       That is, <tt>RunHook X</tt> will run the hook named "X", even if X is a variable.

       If you wish to fetch the hook/keymap name from a variable or expression, you can write <tt>ByName(expression)</tt>,
       where the %expression produces the actual name.

       <b>Note:</b>
       <tt>ByName()</tt> can only be used at places where keymap or hook names are required, nowhere else.

       @since PCC2 2.40.8, PCC2 2.0.8
       @rettype Hook */

    // ex IntStatementCompiler::compileKeymapName (sort-of)
    // also see builtinfunction.cpp
    Tokenizer& tok = m_commandSource.tokenizer();
    if (tok.getCurrentToken() != tok.tIdentifier) {
        throw Error::expectIdentifier(ttl);
    }
    String_t name = tok.getCurrentString();
    tok.readNextToken();
    if (tok.getCurrentToken() == tok.tLParen && name == "BYNAME") {
        // ByName(expr) syntax
        tok.readNextToken();
        std::auto_ptr<interpreter::expr::Node> node(interpreter::expr::Parser(m_commandSource.tokenizer()).parse());
        if (!tok.checkAdvance(tok.tRParen)) {
            throw Error::expectSymbol(")");
        }
        node->compileValue(bco, scc);
        bco.addInstruction(Opcode::maUnary, unUCase, 0);
    } else {
        // Leave it at the name
        afl::data::StringValue sv(name);
        bco.addPushLiteral(&sv);
    }
}

/** Compile definition of a subroutine.
    This is used for structures, subs and functions.
    \param bco Target BCO
    \param scc Statement compilation context
    \param sub The subroutine to be defined
    \param name Name of subroutine
    \param scope Scope to define the subroutine in */
void
interpreter::StatementCompiler::compileSubroutineDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, BCORef_t sub, const String_t& name, Opcode::Scope scope)
{
    // ex IntStatementCompiler::compileSubroutineDefinition
    SubroutineValue subv(sub);
    bco.addPushLiteral(&subv);

    if (m_optimisationLevel >= 0
        && scope == Opcode::sLocal
        && scc.hasFlag(CompilationContext::LinearExecution)
        && scc.hasFlag(CompilationContext::LocalContext)
        && !bco.hasName(name)
        && !bco.hasUserCall())
    {
        // We're creating a local Sub/Function/Struct, and we control
        // the symbol table completely until here.
        if (!bco.hasLocalVariable(name)) {
            bco.addLocalVariable(name);
        }
        bco.addVariableReferenceInstruction(Opcode::maPop, name, scc);
    } else if (scope == Opcode::sLocal || scope == Opcode::sStatic) {
        // We're creating a non-global Sub/Function/Struct, but we don't
        // control the symbol table completely.
        bco.addPushLiteral(0);
        bco.addInstruction(Opcode::maDim, scope, bco.addName(name));

        // FIXME: We don't have a sNamedStatic, and we cannot refer to the variable by address
        // as would be required for sStatic/sLocal. Thus, all we can do is to pop and hope
        // that it ends up at the right place.
        bco.addInstruction(Opcode::maPop, Opcode::sNamedVariable, bco.addName(name));
    } else {
        // Default scope (shared, normally)
        bco.addInstruction(Opcode::maSpecial, Opcode::miSpecialDefSub, bco.addName(name));
    }
}

/** Parse argument list.
    \see parseCommandArgumentList
    \param args [out] Arguments expression trees are accumulated here */
void
interpreter::StatementCompiler::parseArgumentList(afl::container::PtrVector<interpreter::expr::Node>& args)
{
    // ex IntStatementCompiler::parseArgumentList(ptr_vector<IntExprNode>& args)
    parseCommandArgumentList(m_commandSource.tokenizer(), args);
}

/** Ensure that line ends here. */
void
interpreter::StatementCompiler::parseEndOfLine()
{
    // ex IntStatementCompiler::parseEndOfLine()
    if (m_commandSource.tokenizer().getCurrentToken() != Tokenizer::tEnd) {
        throw Error::garbageAtEnd(false);
    }
}

/** Validate name. Throws error if the name should not be used. */
void
interpreter::StatementCompiler::validateName(const StatementCompilationContext& scc, const String_t& name)
{
    // ex IntStatementCompiler::validateName(const string_t& name)
    // Fundamental functions are permitted as variable and procedure names,
    // because they are only recognized when followed by a "(".
    if (lookupKeyword(name) != kwNone || scc.world().lookupSpecialCommand(name) != 0) {
        throw Error("\"" + name + "\" is a reserved name");
    }
}



// Parse argument list.
void
interpreter::parseCommandArgumentList(Tokenizer& tok, afl::container::PtrVector<interpreter::expr::Node>& args)
{
    if (tok.getCurrentToken() != tok.tEnd) {
        /* We have some arguments */
        while (1) {
            args.pushBackNew(interpreter::expr::Parser(tok).parse());
            if (!parseNext(tok)) {
                break;
            }
        }
    }
}
