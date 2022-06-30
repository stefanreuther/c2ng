/**
  *  \file interpreter/functionvalue.hpp
  *  \brief Class interpreter::FunctionValue
  */
#ifndef C2NG_INTERPRETER_FUNCTIONVALUE_HPP
#define C2NG_INTERPRETER_FUNCTIONVALUE_HPP

#include "interpreter/indexablevalue.hpp"

namespace interpreter {

    /** Indexable value, function form.
        This is the base class for items that are callable as functions ("Whatever(arg)").
        It is a convenience class that reduces the number of methods you have to override to two (get, clone),
        and has no special behaviour for the interpreter. */
    class FunctionValue : public IndexableValue {
     public:
        virtual afl::data::Value* get(Arguments& args) = 0;
        virtual CallableValue* clone() const = 0;

        // CallableValue:
        virtual void set(Arguments& args, afl::data::Value* value);
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;
    };

}

#endif
