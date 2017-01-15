/**
  *  \file interpreter/indexablevalue.cpp
  */

#include "interpreter/indexablevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/arguments.hpp"

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
