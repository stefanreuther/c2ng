/**
  *  \file interpreter/subroutinevalue.hpp
  *  \brief Class interpreter::SubroutineValue
  */
#ifndef C2NG_INTERPRETER_SUBROUTINEVALUE_HPP
#define C2NG_INTERPRETER_SUBROUTINEVALUE_HPP

#include "interpreter/callablevalue.hpp"
#include "interpreter/bytecodeobject.hpp"

namespace interpreter {

    /** Subroutine value.
        Value referring to a subroutine (BytecodeObject). */
    class SubroutineValue : public CallableValue {
     public:
        /** Constructor.
            \param bco Bytecode object */
        explicit SubroutineValue(BCORef_t bco);

        /** Destructor. */
        ~SubroutineValue();

        /** Get bytecode object.
            \return BytecodeObject */
        BCORef_t getBytecodeObject() const;

        // CallableValue:
        virtual void call(Process& proc, afl::data::Segment& args, bool wantResult);
        virtual bool isProcedureCall() const;
        virtual int32_t getDimension(int32_t which) const;
        virtual Context* makeFirstContext();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;
        virtual SubroutineValue* clone() const;

     private:
        BCORef_t m_bco;
    };

}

#endif
