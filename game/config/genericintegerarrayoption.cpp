/**
  *  \file game/config/genericintegerarrayoption.cpp
  */

#include "game/config/genericintegerarrayoption.hpp"
#include "game/config/valueparser.hpp"

game::config::GenericIntegerArrayOption::GenericIntegerArrayOption(const ValueParser& parser)
    : m_parser(parser)
{ }

game::config::GenericIntegerArrayOption::~GenericIntegerArrayOption()
{ }

bool
game::config::GenericIntegerArrayOption::isAllTheSame() const
{
    // ex ConfigIntArrayBaseOption::isAllTheSame
    // This needs a const_cast because our getArray() is non-const.
    afl::base::Memory<const int32_t> mem = const_cast<GenericIntegerArrayOption*>(this)->getArray();
    if (const int32_t* pFirst = mem.eat()) {
        int32_t first = *pFirst;
        while (const int32_t* pNext = mem.eat()) {
            if (first != *pNext) {
                return false;
            }
        }
    }
    return true;
}

void
game::config::GenericIntegerArrayOption::set(int index, int32_t value)
{
    // ex ConfigIntArrayOption<N>::set
    afl::base::Memory<int32_t> mem = getArray();
    if (int32_t* p = mem.at(index-1)) {
        if (*p != value) {
            *p = value;
            markChanged();
        }
    }
}

void
game::config::GenericIntegerArrayOption::set(int32_t value)
{
    // ex ConfigIntArrayOption<N>::set
    afl::base::Memory<int32_t> mem = getArray();
    while (int32_t* p = mem.eat()) {
        *p = value;
    }
    markChanged();
}

int32_t
game::config::GenericIntegerArrayOption::operator()(int index) const
{
    // ex ConfigIntArrayOption<N>::operator()
    // If index is out of bounds, return last value (i.e. Colony).
    // This makes more sense then returning the first (Fed),
    // which differs in more ways from standard than colony, at least for options important to us.
    afl::base::Memory<const int32_t> mem = const_cast<GenericIntegerArrayOption*>(this)->getArray();
    if (const int32_t* p = mem.at(index-1)) {
        return *p;
    } else if (const int32_t* p = mem.at(mem.size() - 1)) {
        return *p;
    } else {
        return 0;
    }
}

void
game::config::GenericIntegerArrayOption::set(String_t value)
{
    // ex ConfigIntArrayOption<N>::set
    removeComment(value);
    m_parser.parseArray(value, getArray());
    markChanged();
}

const game::config::ValueParser&
game::config::GenericIntegerArrayOption::parser() const
{
    return m_parser;
}
