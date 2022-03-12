/**
  *  \file interpreter/statementcompiler.hpp
  *  \brief Class interpreter::StatementCompiler
  */
#ifndef C2NG_INTERPRETER_STATEMENTCOMPILER_HPP
#define C2NG_INTERPRETER_STATEMENTCOMPILER_HPP

#include <vector>
#include "afl/base/deleter.hpp"
#include "interpreter/bytecodeobject.hpp"
#include "interpreter/expr/node.hpp"
#include "interpreter/opcode.hpp"

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
        /** Result of a compilation. */
        enum Result {
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
        Result compile(BytecodeObject& bco, const StatementCompilationContext& scc);

        /** Compile statement list.
            \param bco [out] Bytecode object that contains code
            \param scc [in] Statement compilation context
            \retval EndOfInput We ended the compilation because the input ended (only if !scc.hasFlag(WantTerminators))
            \retval Terminator We ended the compilation because we found a terminator (only if scc.hasFlag(WantTerminators)) */
        Result compileList(BytecodeObject& bco, const StatementCompilationContext& scc);

        /** Set optimisation level.
            Optimisation levels:
            - -1 (avoid all optimisation in instruction selection, do not linearize)
            - 0 (some smarter instruction selection)
            - 1 (default optimisations; see optimize())
            - 2 (more expensive optimisations; see optimize())
            \param level Level [MIN_OPTIMISATION_LEVEL,MAX_OPTIMISATION_LEVEL] */
        void setOptimisationLevel(int level);

        /** Finish a BCO.
            Performs configured optimisations.
            \param bco [in/out] Bytecode object produced by compile/compileList
            \param scc [in] Statement compilation context */
        void finishBCO(BytecodeObject& bco, const StatementCompilationContext& scc) const;

     private:
        Result compileAmbiguousStatement(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileAmbiguousSingleWord(const String_t& name, BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileAmbiguousRuntimeSwitch(const String_t& name, bool paren, BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileAbort(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileBind(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileCall(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileCreateKeymap(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileCreateProperty(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Special minor, const char* prefix);
        Result compileDim(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileDo(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileEval(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileFor(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileForEach(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileIf(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileLoad(BytecodeObject& bco, const StatementCompilationContext& scc, bool mustSucceed);
        Result compileOn(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileOption(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compilePrint(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileReDim(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileReturn(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileRunHook(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileScope(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        Result compileSelect(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileSelectionExec(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileSub(BytecodeObject& bco, const StatementCompilationContext& scc, bool proc, Opcode::Scope scope);
        Result compileStruct(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        Result compileTry(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileUseKeymap(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileWith(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileExpressionStatement(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileProcedureCall(BytecodeObject& bco, const StatementCompilationContext& scc);
        Result compileLoopBody(BytecodeObject& bco, StatementCompilationContext& subcc);
        void compileVariableDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, Opcode::Scope scope);
        bool compileInitializer(BytecodeObject& bco, const StatementCompilationContext& scc);
        bool compileTypeInitializer(BytecodeObject& bco, const StatementCompilationContext& scc, const String_t& typeName);
        void compileArgumentExpression(BytecodeObject& bco, const StatementCompilationContext& scc);
        void compileArgumentCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ift, BytecodeObject::Label_t iff);
        void compileSelectCondition(BytecodeObject& bco, const StatementCompilationContext& scc, BytecodeObject::Label_t ldo);
        void compileNameString(BytecodeObject& bco, const StatementCompilationContext& scc, const char* ttl);
        void compileSubroutineDefinition(BytecodeObject& bco, const StatementCompilationContext& scc, BCORef_t sub, const String_t& name, Opcode::Scope scope);
        void parseArgumentList(std::vector<const interpreter::expr::Node*>& args, afl::base::Deleter& del);
        void parseEndOfLine();
        void validateName(const StatementCompilationContext& scc, const String_t& name);

        CommandSource& m_commandSource;
        bool m_allowLocalTypes;
        bool m_allowLocalSubs;
        int m_optimisationLevel;
    };

    /** Parse argument list.
        Parses comma-separated list of expressions.
        Terminates successfully when finding end of line.
        Note that this only parses, it does not compile the expressions.
        \param tok  [in] Tokenizer
        \param args [out] Arguments expression trees are accumulated here
        \param del  [out] Deleter that owns the produced objects */
    void parseCommandArgumentList(Tokenizer& tok, std::vector<const interpreter::expr::Node*>& args, afl::base::Deleter& del);
}

#endif
