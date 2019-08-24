/**
  *  \file interpreter/statementcompiler.hpp
  */
#ifndef C2NG_INTERPRETER_STATEMENTCOMPILER_HPP
#define C2NG_INTERPRETER_STATEMENTCOMPILER_HPP

#include "interpreter/bytecodeobject.hpp"
#include "interpreter/opcode.hpp"
#include "afl/container/ptrvector.hpp"
#include "interpreter/expr/node.hpp"

namespace interpreter {

    class CommandSource;
    class StatementCompilationContext;
    class Tokenizer;

    /** Statement compiler.
        Takes a CommandSource, and compiles it into a BytecodeObject using a StatementCompilationContext.

        Main entry points are compile() for a single statement, and compileList() for a statement sequence.
        Those call each other recursively for nested statements.

        Acceptance of one-line and block commands is configured using the StatementCompilationContext.

        Errors are reported by throwing Error. */
    class StatementCompiler {
     public:
        // FIXME: rename to Result
        enum StatementResult {
            EndOfInput,              ///< End of input reached. Exit. Only if not WantTerminators.
            Terminator,              ///< Terminator statement left in current token. Only if WantTerminators.
            CompiledStatement,       ///< Successfully compiled a single-line statement.
            CompiledBlock,           ///< Successfully compiled a multi-line statement. Only if not RefuseBlocks.
            CompiledExpression       ///< Successfully compiled an expression. Its result remains on stack. Only if not ExpressionsAreStatements.
        };

        static const int MIN_OPTIMISATION_LEVEL = -1;
        static const int MAX_OPTIMISATION_LEVEL = 3;
        static const int DEFAULT_OPTIMISATION_LEVEL = 1;

        /** Constructor.
            \param cs Command source that contains statements to compile. */
        explicit StatementCompiler(CommandSource& cs);

        /** Compile a statement.
            Assumes input at beginning of statement, that is, the tokenizer initialized to the current line, looking at the first token.
            Leaves input at end of statement. That is, caller must invoke cs.readNextLine() to advance to the next line.
            On a multi-line statement, this leaves the input after the end of the terminator.
            \param bco [out] Bytecode object that contains code
            \param scc [in] Statement compilation context */
        StatementResult compile(BytecodeObject& bco, const StatementCompilationContext& scc);

        /** Compile statement list.
            \param bco [out] Bytecode object that contains code
            \param scc [in] Statement compilation context
            \retval EndOfInput We ended the compilation because the input ended (only if !scc.hasFlag(WantTerminators))
            \retval Terminator We ended the compilation because we found a terminator (only if scc.hasFlag(WantTerminators)) */
        StatementResult compileList(BytecodeObject& bco, const StatementCompilationContext& scc);

        void setOptimisationLevel(int level);

        /** Finish a BCO. Performs configured optimisations. */
        void finishBCO(BytecodeObject& bco, const StatementCompilationContext& scc);

     private:
        StatementResult compileAmbiguousStatement(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileAmbiguousSingleWord(const String_t& name, BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileAmbiguousRuntimeSwitch(const String_t& name, bool paren, BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileAbort(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileBind(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileCall(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileCreateKeymap(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileCreateProperty(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Special minor, const char* prefix);
        StatementResult compileDim(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileDo(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileEval(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileFor(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileForEach(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileIf(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileLoad(BytecodeObject& bco, const StatementCompilationContext& scc, bool mustSucceed);
        StatementResult compileOn(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileOption(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compilePrint(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileReDim(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileReturn(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileRunHook(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileScope(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        StatementResult compileSelect(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileSelectionExec(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileSub(BytecodeObject& bco, const StatementCompilationContext& scc, bool proc, Opcode::Scope scope);
        StatementResult compileStruct(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        StatementResult compileTry(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileUseKeymap(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileWith(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileExpressionStatement(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileProcedureCall(BytecodeObject& bco, const StatementCompilationContext& scc);
        StatementResult compileLoopBody(BytecodeObject& bco, StatementCompilationContext& subcc);
        void compileVariableDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        bool compileInitializer(BytecodeObject& bco, const StatementCompilationContext& scc);
        bool compileTypeInitializer(BytecodeObject& bco, const StatementCompilationContext& scc, const String_t& typeName);
        void compileArgumentExpression(BytecodeObject& bco, const StatementCompilationContext& scc);
        void compileArgumentCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        void compileSelectCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ldo);
        void compileKeymapName(BytecodeObject& bco, const StatementCompilationContext& scc, const char* ttl);
        void compileSubroutineDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, BCORef_t sub, const String_t& name, Opcode::Scope scope);
        void parseArgumentList(afl::container::PtrVector<interpreter::expr::Node>& args);
        void parseEndOfLine();
        void validateName(const StatementCompilationContext& scc, const String_t& name);
    
        CommandSource& m_commandSource;
        bool m_allowLocalTypes;
        bool m_allowLocalSubs;
        int m_optimisationLevel;
    };

    void parseCommandArgumentList(Tokenizer& tok, afl::container::PtrVector<interpreter::expr::Node>& args);
}

#endif
