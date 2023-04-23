/**
  *  \file interpreter/functionvalue.cpp
  *  \brief Class interpreter::FunctionValue
  */

#include "interpreter/functionvalue.hpp"

void
interpreter::FunctionValue::set(Arguments& args, const afl::data::Value* value)
{
    rejectSet(args, value);
}

int32_t
interpreter::FunctionValue::getDimension(int32_t /*which*/) const
{
    return 0;
}

interpreter::Context*
interpreter::FunctionValue::makeFirstContext()
{
    return rejectFirstContext();
}

String_t
interpreter::FunctionValue::toString(bool /*readable*/) const
{
    return "#<function>";
}

void
interpreter::FunctionValue::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
