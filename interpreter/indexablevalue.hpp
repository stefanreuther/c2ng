/**
  *  \file interpreter/indexablevalue.hpp
  *  \brief Class interpreter::IndexableValue
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
            \param args Parameters
            \return obtained value, newly-allocated. Throws Error if request is invalid */
        virtual afl::data::Value* get(Arguments& args) = 0;

        /** Set indexed value.
            This implements "value(args) := a".
            \param args Parameters
            \param value Value to assign, owned by caller.
            \throw Error if request is invalid */
        virtual void set(Arguments& args, const afl::data::Value* value) = 0;

        // CallableValue:
        virtual bool isProcedureCall() const;
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);

        /** Get all elements.
            Requires that this IndexableValue represents a one-dimensional array.
            \param [out] out      Result goes here
            \param [in]  startAt  First index
            \throw Error if this is not a one-dimensional array, or it is too big */
        void getAll(afl::data::Segment& out, size_t startAt);

     protected:
        /** Reject set().
            Throws an Error::notAssignable().
            \param args Parameters; pass from set()
            \param value Value to assign; pass from set() */
        void rejectSet(Arguments& args, const afl::data::Value* value) const;
    };

}

#endif
