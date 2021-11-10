/**
  *  \file interpreter/genericvalue.hpp
  *  \brief Template class interpreter::GenericValue
  */
#ifndef C2NG_INTERPRETER_GENERICVALUE_HPP
#define C2NG_INTERPRETER_GENERICVALUE_HPP

#include "interpreter/basevalue.hpp"
#include "interpreter/error.hpp"

namespace interpreter {

    /** Generic value.
        Wraps a value of a given type that has no methods, properties, or array elements of itself.
        Values of this type can be stored and passed around, and examined by C++ compiled code.
        They cannot be serialized or examined by script code.

        A GenericValue is immutable.
        When cloned, its embedded value is copied as well.
        To get a mutable value, specify T as Ref<X> and store your data in the given extra object.

        \tparam T Contained type */
    template<typename T>
    class GenericValue : public BaseValue {
     public:
        /** Constructor.
            \param value Value */
        GenericValue(const T& value);

        /** Destructor. */
        ~GenericValue();

        // BaseValue:
        virtual String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;
        virtual GenericValue* clone() const;

        /** Access contained value.
            \return value */
        const T& get() const;

     private:
        T m_value;
    };

}

template<typename T>
inline
interpreter::GenericValue<T>::GenericValue(const T& value)
    : m_value(value)
{ }

template<typename T>
interpreter::GenericValue<T>::~GenericValue()
{ }

template<typename T>
String_t
interpreter::GenericValue<T>::toString(bool /*readable*/) const
{
    return "#<builtin>";
}

template<typename T>
void
interpreter::GenericValue<T>::store(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    throw Error::notSerializable();
}

template<typename T>
interpreter::GenericValue<T>*
interpreter::GenericValue<T>::clone() const
{
    return new GenericValue(m_value);
}

template<typename T>
inline const T&
interpreter::GenericValue<T>::get() const
{
    return m_value;
}

#endif
