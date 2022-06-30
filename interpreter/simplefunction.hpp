/**
  *  \file interpreter/simplefunction.hpp
  *  \brief Template class interpreter::SimpleFunction
  */
#ifndef C2NG_INTERPRETER_SIMPLEFUNCTION_HPP
#define C2NG_INTERPRETER_SIMPLEFUNCTION_HPP

#include "interpreter/functionvalue.hpp"

namespace interpreter {

    /** Simple function.
        This class wraps a regular C++ function into an interpreter value,
        to produce an interpreter-callable function.
        The function can receive a state variable.

        The state variable is part of the SimpleFunction object and cloned whenever the value is cloned.
        To share the state variable, use a (smart) pointer or reference.

        @tparam StateType    Type of state variable. Pass void if you do not want a state variable.
        @tparam StateArg     Type of the state variable parameter passed to the C++ function.
                             Defaults to `StateType`; can be `const StateArg&` to pass by const-reference. */
    template<typename StateType, typename StateArg = StateType>
    class SimpleFunction : public FunctionValue {
     public:
        /** Shortcut for underlying function. */
        typedef afl::data::Value* Get_t(StateArg state, Arguments& args);

        /** Constructor.
            @param state   State parameter
            @param get     Function to call; should not be null */
        SimpleFunction(StateArg state, Get_t* get);

        // IndexableValue:
        virtual afl::data::Value* get(Arguments& args);
        virtual FunctionValue* clone() const;

     private:
        const StateType m_state;
        Get_t* const m_get;
    };


    /*
     *  Specialisation for void
     */

    template<>
    class SimpleFunction<void> : public FunctionValue {
     public:
        typedef afl::data::Value* Get_t(Arguments& args);

        SimpleFunction(Get_t* get);

        virtual afl::data::Value* get(Arguments& args);
        virtual FunctionValue* clone() const;

     private:
        Get_t* const m_get;
    };

}

template<typename StateType, typename StateArg>
inline
interpreter::SimpleFunction<StateType,StateArg>::SimpleFunction(StateArg state, Get_t* get)
    : m_state(state), m_get(get)
{ }

template<typename StateType, typename StateArg>
afl::data::Value*
interpreter::SimpleFunction<StateType,StateArg>::get(Arguments& args)
{
    // ex IntSimpleIndexableValue::get
    return m_get != 0
        ? m_get(m_state, args)
        : 0;
}

template<typename StateType, typename StateArg>
interpreter::FunctionValue*
interpreter::SimpleFunction<StateType,StateArg>::clone() const
{
    // ex IntSimpleIndexableValue::clone
    return new SimpleFunction(m_state, m_get);
}

inline
interpreter::SimpleFunction<void>::SimpleFunction(Get_t* get)
    : m_get(get)
{ }

#endif
