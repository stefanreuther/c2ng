/**
  *  \file interpreter/subroutinevalue.cpp
  */

#include "interpreter/subroutinevalue.hpp"
#include "interpreter/process.hpp"
#include "interpreter/error.hpp"
#include "interpreter/savecontext.hpp"


interpreter::SubroutineValue::SubroutineValue(BCORef_t bco)
    : m_bco(bco)
{
    // IntSubroutineValue::IntSubroutineValue
}

interpreter::SubroutineValue::~SubroutineValue()
{ }

interpreter::BCORef_t
interpreter::SubroutineValue::getBytecodeObject() const
{
    // ex IntSubroutineValue::getBytecodeObject
    return m_bco;
}

void
interpreter::SubroutineValue::call(Process& proc, afl::data::Segment& args, bool wantResult)
{
    // ex IntSubroutineValue::call
    proc.handleFunctionCall(m_bco, args, wantResult);
}

bool
interpreter::SubroutineValue::isProcedureCall() const
{
    // ex IntSubroutineValue::isProcedureCall
    return m_bco->isProcedure();
}

int32_t
interpreter::SubroutineValue::getDimension(int32_t /*which*/) const
{
    // ex IntSubroutineValue::getDimension
    return 0;
}

interpreter::Context*
interpreter::SubroutineValue::makeFirstContext()
{
    // ex IntSubroutineValue::makeFirstContext
    throw Error::typeError(Error::ExpectIterable);
}

String_t
interpreter::SubroutineValue::toString(bool /*readable*/) const
{
    // ex IntSubroutineValue::toString
    String_t name = m_bco->getName();
    String_t result = "#<subr";
    if (name.size()) {
        result += ":";
        result += name;
    }
    return result + ">";
}

void
interpreter::SubroutineValue::store(TagNode& out, afl::io::DataSink& /*aux*/, afl::charset::Charset& /*cs*/, SaveContext& ctx) const
{
    // ex IntSubroutineValue::store
    out.tag   = TagNode::Tag_BCO;
    out.value = ctx.addBCO(*m_bco);
}

interpreter::SubroutineValue*
interpreter::SubroutineValue::clone() const
{
    return new SubroutineValue(m_bco);
}
