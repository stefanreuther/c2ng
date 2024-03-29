/**
  *  \file interpreter/indexablevalue.cpp
  *  \brief Class interpreter::IndexableValue
  */

#include "interpreter/indexablevalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/process.hpp"

bool
interpreter::IndexableValue::isProcedureCall() const
{
    // ex IntIndexableValue::isProcedureCall
    return false;
}

void
interpreter::IndexableValue::call(Process& proc, afl::data::Segment& args, bool want_result)
{
    // ex IntIndexableValue::call
    Arguments a(args, 0, args.size());
    afl::data::Value* v = get(a);
    if (want_result) {
        proc.pushNewValue(v);
    } else {
        delete v;
    }
}

void
interpreter::IndexableValue::rejectSet(Arguments& /*args*/, const afl::data::Value* /*value*/) const
{
    throw Error::notAssignable();
}
