/**
  *  \file interpreter/simpleprocedure.hpp
  *  \brief Template class interpreter::SimpleProcedure
  */
#ifndef C2NG_INTERPRETER_SIMPLEPROCEDURE_HPP
#define C2NG_INTERPRETER_SIMPLEPROCEDURE_HPP

#include "interpreter/procedurevalue.hpp"

namespace interpreter {

    /** Simple procedure.
        This class wraps a regular C++ procedure into an interpreter value,
        to produce an interpreter-callable procedure.
        The procedure can receive a state variable.

        The state variable is part of the SimpleProcedure object and cloned whenever the value is cloned.
        To share the state variable, use a (smart) pointer or reference.

        @tparam StateType    Type of state variable. Pass void if you do not want a state variable.
        @tparam StateArg     Type of the state variable parameter passed to the C++ procedure.
                             Defaults to `StateType`; can be `const StateArg&` to pass by const-reference. */
    template<typename StateType, typename StateArg = StateType>
    class SimpleProcedure : public ProcedureValue {
     public:
        /** Shortcut for underlying procedure. */
        typedef void Call_t(StateArg state, Process& proc, Arguments& args);

        /** Constructor.
            @param state   State parameter
            @param call    Procedure to call; should not be null */
        SimpleProcedure(StateArg state, Call_t* call);

        virtual void call(Process& proc, Arguments& args);
        virtual ProcedureValue* clone() const;

     private:
        const StateType m_state;
        Call_t* const m_call;
    };


    /*
     *  Specialisation for void
     */

    template<>
    class SimpleProcedure<void> : public ProcedureValue {
     public:
        typedef void Call_t(Process& proc, Arguments& args);

        SimpleProcedure(Call_t* call);

        virtual void call(Process& proc, Arguments& args);
        virtual ProcedureValue* clone() const;

     private:
        Call_t* const m_call;
    };

}

template<typename StateType, typename StateArg>
inline
interpreter::SimpleProcedure<StateType,StateArg>::SimpleProcedure(StateArg state, Call_t* call)
    : m_state(state), m_call(call)
{ }

template<typename StateType, typename StateArg>
void
interpreter::SimpleProcedure<StateType,StateArg>::call(Process& proc, Arguments& args)
{
    if (m_call != 0) {
        m_call(m_state, proc, args);
    }
}

template<typename StateType, typename StateArg>
interpreter::ProcedureValue*
interpreter::SimpleProcedure<StateType,StateArg>::clone() const
{
    return new SimpleProcedure(m_state, m_call);
}

inline
interpreter::SimpleProcedure<void>::SimpleProcedure(Call_t* call)
    : m_call(call)
{ }

#endif
