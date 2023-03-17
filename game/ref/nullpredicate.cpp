/**
  *  \file game/ref/nullpredicate.cpp
  *  \brief Class game::ref::NullPredicate
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
