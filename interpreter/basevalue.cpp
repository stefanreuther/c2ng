/**
  *  \file interpreter/basevalue.cpp
  *  \brief Base class interpreter::BaseValue
  */

#include "interpreter/basevalue.hpp"
#include "afl/data/visitor.hpp"
#include "interpreter/error.hpp"

void
interpreter::BaseValue::visit(afl::data::Visitor& visitor) const
{
    visitor.visitOther(*this);
}

void
interpreter::BaseValue::rejectStore(TagNode& /*out*/, afl::io::DataSink& /*aux*/, SaveContext& /*ctx*/) const
{
    throw Error::notSerializable();
}
