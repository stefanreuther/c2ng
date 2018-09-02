/**
  *  \file game/ref/sortbyid.cpp
  */

#include "game/ref/sortbyid.hpp"

int
game::ref::SortById::compare(const Reference& a, const Reference& b) const
{
    // ex game/objl-sort.cc:sortById
    int aid = a.getId();
    int bid = b.getId();
    if (aid < bid) {
        return -1;
    } else if (aid > bid) {
        return +1;
    } else {
        return 0;
    }
}

String_t
game::ref::SortById::getClass(const Reference& /*a*/) const
{
    return String_t();
}
