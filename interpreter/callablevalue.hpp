/**
  *  \file interpreter/callablevalue.hpp
  *  \brief Class interpreter::CallableValue
  */
#ifndef C2NG_INTERPRETER_CALLABLEVALUE_HPP
#define C2NG_INTERPRETER_CALLABLEVALUE_HPP

#include "afl/data/segment.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter {

    class Process;
    class Context;

    /** Callable value.
        This is the base for items callable in a process context.
        They can have the syntactic form of a procedure or of a function.
        A CallableValue can also appear in a ForEach loop to provide an iterable context.

        The specialisation IndexableValue provides elements that have the syntactic form of a function,
        and can optionally be assigned.

        Further specialisations exist to bind C++ functions. */
    class CallableValue : public BaseValue {
     public:
        /** Call.
            This implements invocation of a subroutine, in the form "a := value(args)".
            Subroutines without return value have to return null instead.
            \param proc Process
            \param args Data segment containing parameters
            \param want_result True if a result is required (use proc.pushNewValue()) */
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result) = 0;

        /** Check syntactic form.
            \retval false Function ("name(a1,a2)", can have result)
            \retval true Procedure ("name a1,a2", no result) */
        virtual bool isProcedureCall() const = 0;

        /** Array reflection. Implementation of the IsArray() and Dim() builtins.
            \param which 0=get number of dimensions, 1..n=get that dimension
            \return result */
        virtual int32_t getDimension(int32_t which) const = 0;

        /** Get context for first item in iteration.
            This should be equivalent to <tt>dynamic_cast<Context*>(call(proc, args, true))</tt>,
            where args are the parameters needed to address the first object.
            Caller assumes lifetime management for the context.
            \return context. Return 0 if set is empty.
            \throw Error if request is invalid and item is not iterable. */
        virtual Context* makeFirstContext() = 0;

        // Value:
        virtual CallableValue* clone() const = 0;

     protected:
        /** Reject makeFirstContext() with a type error.
            \return Function never returns, but this makes it compatible with use as "return rejectFirstContext();"
            \throw Error */
        Context* rejectFirstContext() const;
    };

}

#endif
