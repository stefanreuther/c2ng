/**
  *  \file interpreter/closure.hpp
  *  \brief Class interpreter::Closure
  */
#ifndef C2NG_INTERPRETER_CLOSURE_HPP
#define C2NG_INTERPRETER_CLOSURE_HPP

#include "afl/base/ptr.hpp"
#include "interpreter/callablevalue.hpp"

namespace interpreter {

    /** Closure.
        Represents a CallableValue with some arguments fixed.
        When called (i.e. call()), inserts the fixed arguments at the beginning of the argument list.
        Otherwise, behaves exactly like the underlying CallableValue.

        To create,
        - construct
        - set the underlying CallableValue using setNewFunction
        - add fixed arguments using addNewArgument(), addNewArgumentsFrom() */
    class Closure : public CallableValue {
     public:
        /** Constructor. */
        Closure();

        /** Destructor. */
        ~Closure();

        /** Set function.
            @param function Function. Should not be null. Will become owned by Closure. */
        void setNewFunction(CallableValue* function);

        /** Add single argument.
            @param value Value. Will become owned by Closure. */
        void addNewArgument(afl::data::Value* value);

        /** Add arguments by transferring from a data segment.
            Removes the last @c nargs arguments from @c seg, taking over their ownership,
            and append them to the list of fixed arguments.

            @param [in,out] seg   Segment
            @param [in]     nargs Number of arguments to transfer

            @see afl::data::Segment::transferLastTo */
        void addNewArgumentsFrom(afl::data::Segment& seg, size_t nargs);

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual size_t getDimension(size_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;

        // Value:
        virtual Closure* clone() const;

     private:
        Closure(const Closure& other);

        // Both attributes are smart pointers, so we can share them without cloning.
        // They are not modified.
        afl::base::Ptr<CallableValue> m_function;
        afl::base::Ptr<afl::data::Segment> m_fixedArgs;
    };
}

#endif
