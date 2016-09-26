/**
  *  \file interpreter/subroutinevalue.hpp
  */
#ifndef C2NG_INTERPRETER_SUBROUTINEVALUE_HPP
#define C2NG_INTERPRETER_SUBROUTINEVALUE_HPP

#include "interpreter/callablevalue.hpp"
#include "interpreter/bytecodeobject.hpp"

namespace interpreter {

    /** Subroutine value.
        Value referring to a subroutine.
        This is used to implement the 'pushsub' opcode. */
    class SubroutineValue : public CallableValue {
     public:
        SubroutineValue(BCORef_t bco);
        ~SubroutineValue();
        BCORef_t getBytecodeObject() const;

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool wantResult);
        virtual bool isProcedureCall();
        virtual int32_t getDimension(int32_t which);
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext* ctx) const;
        virtual SubroutineValue* clone() const;

     private:
        BCORef_t m_bco;
    };

}

#endif
