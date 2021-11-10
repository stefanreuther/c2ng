/**
  *  \file interpreter/closure.hpp
  */
#ifndef C2NG_INTERPRETER_CLOSURE_HPP
#define C2NG_INTERPRETER_CLOSURE_HPP

#include "interpreter/callablevalue.hpp"
#include "afl/base/ptr.hpp"

namespace interpreter {

    /** Closure. */
    class Closure : public CallableValue {
     public:
        Closure();
        ~Closure();

        // Closure:
        void setNewFunction(CallableValue* function);
        void addNewArgument(afl::data::Value* value);
        void addNewArgumentsFrom(afl::data::Segment& seg, size_t nargs);

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool want_result);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
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
