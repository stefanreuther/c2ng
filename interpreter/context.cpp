/**
  *  \file interpreter/context.cpp
  *  \brief Class interpreter::Context
  */

#include "interpreter/context.hpp"
#include "interpreter/error.hpp"

void
interpreter::Context::ReadOnlyAccessor::set(PropertyIndex_t /*index*/, const afl::data::Value* /*value*/)
{
    throw Error::notAssignable();
}
