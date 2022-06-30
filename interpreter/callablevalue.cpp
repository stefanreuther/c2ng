/**
  *  \file interpreter/callablevalue.cpp
  *  \brief Class interpreter::CallableValue
  */

#include "interpreter/callablevalue.hpp"
#include "interpreter/error.hpp"

interpreter::Context*
interpreter::CallableValue::rejectFirstContext() const
{
    throw Error::typeError(Error::ExpectIterable);
}
