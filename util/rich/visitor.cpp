/**
  *  \file util/rich/visitor.cpp
  */

#include "util/rich/visitor.hpp"
#include "util/rich/text.hpp"

util::rich::Visitor&
util::rich::Visitor::visit(const Text& text)
{
    return text.visit(*this);
}
