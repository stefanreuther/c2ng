/**
  *  \file interpreter/indexablevalue.hpp
  */
#ifndef C2NG_INTERPRETER_INDEXABLEVALUE_HPP
#define C2NG_INTERPRETER_INDEXABLEVALUE_HPP

#include "interpreter/callablevalue.hpp"

namespace interpreter {

    class Arguments;
    class Process;

    /** Indexable value.
        This value can be used in two forms, "value(args)" and "ForEach value Do ...". */
    class IndexableValue : public CallableValue {
     public:
        /** Call.
            This implements "a := value(args)".
            \param dseg  Data segment containing parameters
            \param index Index of first parameter
            \param nargs Number of parameters
            \return obtained value. Throws IntError if request is invalid */
        virtual afl::data::Value* get(Arguments& args) = 0;

        /** Set indexed value.
            This implements "value(args) := a".
            \param dseg  Data segment containing parameters
            \param index Index of first parameter
            \param nargs Number of parameters
            \param value Value to assign.
            \return obtained value. Throws IntError if request is invalid */
        virtual void set(Arguments& args, afl::data::Value* value) = 0;

        virtual bool isProcedureCall() const;
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);
    };

}

#endif
