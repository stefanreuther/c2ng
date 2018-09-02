/**
  *  \file game/ref/sortpredicate.cpp
  */

#include "game/ref/sortpredicate.hpp"

game::ref::SortPredicate::CombinedPredicate::CombinedPredicate(const SortPredicate& first, const SortPredicate& second)
    : m_first(first),
      m_second(second)
{ }

int
game::ref::SortPredicate::CombinedPredicate::compare(const Reference& a, const Reference& b) const
{
    int result = m_first.compare(a, b);
    if (result == 0) {
        result = m_second.compare(a, b);
    }
    return result;
}

String_t
game::ref::SortPredicate::CombinedPredicate::getClass(const Reference& a) const
{
    return m_first.getClass(a);
}

game::ref::SortPredicate::CombinedPredicate
game::ref::SortPredicate::then(const SortPredicate& other) const
{
    return CombinedPredicate(*this, other);
}
