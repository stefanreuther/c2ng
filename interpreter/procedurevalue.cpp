/**
  *  \file interpreter/procedurevalue.cpp
  *  \brief Class interpreter::ProcedureValue
  */

#include "interpreter/procedurevalue.hpp"
#include "interpreter/process.hpp"

// CallableValue:
void
interpreter::ProcedureValue::call(Process& proc, afl::data::Segment& args, bool wantResult)
{
    Arguments a(args, 0, args.size());
    call(proc, a);
    if (wantResult) {
        proc.pushNewValue(0);
    }
}

bool
interpreter::ProcedureValue::isProcedureCall() const
{
    return true;
}

size_t
interpreter::ProcedureValue::getDimension(size_t /*which*/) const
{
    return 0;
}

interpreter::Context*
interpreter::ProcedureValue::makeFirstContext()
{
    return rejectFirstContext();
}

// BaseValue:
String_t
interpreter::ProcedureValue::toString(bool /*readable*/) const
{
    return "#<procedure>";
}

void
interpreter::ProcedureValue::store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const
{
    rejectStore(out, aux, ctx);
}
