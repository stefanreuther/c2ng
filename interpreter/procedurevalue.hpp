/**
  *  \file interpreter/procedurevalue.hpp
  *  \brief Class interpreter::ProcedureValue
  */
#ifndef C2NG_INTERPRETER_PROCEDUREVALUE_HPP
#define C2NG_INTERPRETER_PROCEDUREVALUE_HPP

#include "interpreter/arguments.hpp"
#include "interpreter/callablevalue.hpp"

namespace interpreter {

    /** Callable value, procedure form.
        This is the base class for items that are callable as procedures ("DoSomething arg, arg, arg").
        It is a convenience class that reduces the number of methods you have to override to two (call, clone),
        and has no special behaviour for the interpreter. */
    class ProcedureValue : public CallableValue {
     public:
        /** Call procedure.
            \param proc Process. Can be used to examine the current context or to set global variables. Do not modify the stack.
            \param args Input arguments. */
        virtual void call(Process& proc, Arguments& args) = 0;
        virtual ProcedureValue* clone() const = 0;

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool wantResult);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;
    };

}

#endif
