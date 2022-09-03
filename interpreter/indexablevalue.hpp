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
            \return obtained value. Throws Error if request is invalid */
        virtual void set(Arguments& args, afl::data::Value* value) = 0;

        virtual bool isProcedureCall() const;
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);

     protected:
        /** Reject set().
            Throws an Error::notAssignable().
            \param args Parameters; pass from set()
            \param value Value to assign; pass from set() */
        void rejectSet(Arguments& args, afl::data::Value* value) const;
    };

}

#endif
