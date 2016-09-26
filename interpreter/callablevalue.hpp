/**
  *  \file interpreter/callablevalue.hpp
  *
  *  Original comment:
  *
  *  IntCallableValue is the basic type for an item which can be
  *  called. Derived from it is IntIndexableValue, which can appear as
  *  a function call "value(args)" on the left or right side of an
  *  assignment, or in iteration "ForEach value". 
  *  IntSimpleIndexableValue implements the common case of such a
  *  value, which is a global function with no associated data. 
  *  Likewise, IntSimpleProcedureValue implements the common case of a
  *  global procedure.
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
        They can have the syntactic form of a procedure or of a function. */
    class CallableValue : public BaseValue {
     public:
        /** Call.
            This implements invocation of a subroutine, in the form "a := value(args)".
            Subroutines without return value have to return null instead.
            \param proc Process
            \param args Data segment containing parameters
            \param want_result True if a result is required (use exc.pushNewValue()) */
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result) = 0;

        /** Check syntactic form. */
        virtual bool isProcedureCall() = 0;

        /** Array reflection. Implementation of the IsArray() and Dim() builtins.
            \param which 0=get number of dimensions, 1..n=get that dimension */
        virtual int32_t getDimension(int32_t which) = 0;

        /** Get context for first item in iteration. This should be equivalent to
            <tt>dynamic_cast<IntContext*>(call(exc, args))</tt>, where
            args are the parameters needed to address the first object.
            Caller assumes lifetime management for the context.
            \return context. Return 0 if set is empty. Throws IntError if request is invalid. */
        virtual Context* makeFirstContext() = 0;

        // Value:
        virtual CallableValue* clone() const = 0;
    };

}

#endif
