/**
  *  \file interpreter/basevalue.cpp
  *  \brief Base class interpreter::BaseValue
  */

#include "interpreter/basevalue.hpp"
#include "afl/data/visitor.hpp"

void
interpreter::BaseValue::visit(afl::data::Visitor& visitor) const
{
    visitor.visitOther(*this);
}
