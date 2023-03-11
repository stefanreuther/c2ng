/**
  *  \file util/rich/visitor.cpp
  *  \brief Base class util::rich::Visitor
  */

#include "util/rich/visitor.hpp"
#include "util/rich/text.hpp"

util::rich::Visitor&
util::rich::Visitor::visit(const Text& text)
{
    return text.visit(*this);
}
