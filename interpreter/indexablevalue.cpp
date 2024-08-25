/**
  *  \file interpreter/indexablevalue.cpp
  *  \brief Class interpreter::IndexableValue
  */

#include "interpreter/indexablevalue.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
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
interpreter::IndexableValue::getAll(afl::data::Segment& out, size_t startAt)
{
    // It's an array
    if (getDimension(0) != 1) {
        throw Error::typeError(Error::ExpectArray);
    }
    for (size_t i = startAt, n = getDimension(1); i < n && i < 0x7FFFFFFF; ++i) {
        // Construct "(i)" arguments
        afl::data::Segment argSeg;
        argSeg.pushBackInteger(int32_t(i));
        Arguments args(argSeg, 0, 1);

        // Fetch value. This may throw.
        out.pushBackNew(get(args));
    }
}

void
interpreter::IndexableValue::rejectSet(Arguments& /*args*/, const afl::data::Value* /*value*/) const
{
    throw Error::notAssignable();
}
