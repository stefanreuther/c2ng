/**
  *  \file interpreter/simpleprocedurevalue.hpp
  */
#ifndef C2NG_INTERPRETER_SIMPLEPROCEDUREVALUE_HPP
#define C2NG_INTERPRETER_SIMPLEPROCEDUREVALUE_HPP

#include "interpreter/procedurevalue.hpp"

namespace interpreter {

    class World;

    class SimpleProcedureValue : public ProcedureValue {
     public:
        typedef void Call_t(World& world, Process& proc, Arguments& args);

        SimpleProcedureValue(World& world, Call_t* call);
        ~SimpleProcedureValue();

        virtual void call(Process& proc, Arguments& args);
        virtual SimpleProcedureValue* clone() const;

     private:
        World& m_world;
        Call_t* m_call;
    };

}

#endif
