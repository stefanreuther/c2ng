/**
  *  \file interpreter/compilationcontext.hpp
  *  \brief Class interpreter::CompilationContext
  */
#ifndef C2NG_INTERPRETER_COMPILATIONCONTEXT_HPP
#define C2NG_INTERPRETER_COMPILATIONCONTEXT_HPP

#include "afl/bits/smallset.hpp"

namespace interpreter {

    class World;

    /** Compilation context.
        Contains flags in effect for current compilation.

        CompilationContext is used for expressions but can hold all option flags, including those for statements.
        These flags generally provide information about the compilation environment (e.g. "we are compiling a multi-line statement"),
        not user options ("optimisation level").

        StatementCompilationContext extends CompilationContext to contain additional parameters and behaviour for compiling statements. */
    class CompilationContext {
     public:
        /** Compilation flag. */
        enum Flag {
            /** Expressions: If set, string operations are case-blind ("NC" opcodes). */
            CaseBlind,
            /** Expressions: If set, code will execute directly in the BCO's context; no 'With' or 'ForEach' active.
                This allows the code generator to generate 'pushloc' instead of 'pushvar' instructions
                for local names. */
            LocalContext,
            /** Expressions: If this is set along with LocalContext, the parent of the BCO's context will be the
                shared variables. */
            AlsoGlobalContext,
            /** Statements: if set, expressions are statements, and that's it. The expression will be compiled
                into its side-effect, discarding the result. If clear, we want expression results. */
            ExpressionsAreStatements,
            /** Statements: restrict to one-liners. If set, multi-line blocks are refused. */
            RefuseBlocks,
            /** Statements: accept terminators. If set, terminators are reported to the caller of the compiler;
                EOF is an error. If clear, terminators are not expected and cause an error, EOF terminates
                compilation. */
            WantTerminators,
            /** Statements: linear execution until here. If set, it is guaranteed that the statements within this
                block are guaranteed to be executed in their entirety, linearly once from top to bottom. */
            LinearExecution,
            /** Statements: execute "Load" at compile time.
                If set, statements of the form "Load <literal>" are executed at compilation time. */
            PreexecuteLoad
        };

        /** Constructor.
            \param world World in which this compilation takes place */
        explicit CompilationContext(World& world);

        /** Add a flag.
            \param flag Flag to add
            \return *this (for chainability) */
        CompilationContext& withFlag(Flag flag);

        /** Remove a flag.
            \param flag Flag to remove
            \return *this (for chainability) */
        CompilationContext& withoutFlag(Flag flag);

        /** Check presence of a flag.
            \param flag Flag to check
            \return true if flag is set */
        bool hasFlag(Flag flag) const;

        /** Access associated world.
            \return world */
        World& world() const;

     private:
        // ex compilation_flags
        afl::bits::SmallSet<Flag> m_compilationFlags;

        World& m_world;
    };

}

// Constructor.
inline
interpreter::CompilationContext::CompilationContext(World& world)
    : m_compilationFlags(CaseBlind),
      m_world(world)
{ }

// Add a flag.
inline interpreter::CompilationContext&
interpreter::CompilationContext::withFlag(Flag flag)
{
    m_compilationFlags += flag;
    return *this;
}

// Remove a flag.
inline interpreter::CompilationContext&
interpreter::CompilationContext::withoutFlag(Flag flag)
{
    m_compilationFlags -= flag;
    return *this;
}

// Check presence of a flag.
inline bool
interpreter::CompilationContext::hasFlag(Flag flag) const
{
    return m_compilationFlags.contains(flag);
}

// Access associated world.
inline interpreter::World&
interpreter::CompilationContext::world() const
{
    // World is always modifyable, constness of the context means we don't modify the context
    return m_world;
}

#endif
