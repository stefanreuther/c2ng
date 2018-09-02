/**
  *  \file game/ref/nullpredicate.cpp
  */

#include "game/ref/nullpredicate.hpp"

int
game::ref::NullPredicate::compare(const Reference& /*a*/, const Reference& /*b*/) const
{
    return 0;
}

String_t
game::ref::NullPredicate::getClass(const Reference& /*a*/) const
{
    return String_t();
}
