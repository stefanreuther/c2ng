/**
  *  \file interpreter/nametable.cpp
  *  \brief Structure interpreter::NameTable
  */

#include "interpreter/nametable.hpp"

// Look up name in table.
bool
interpreter::lookupName(const afl::data::NameQuery& name, afl::base::Memory<const NameTable> tab, interpreter::Context::PropertyIndex_t& index)
{
    // ex int/if/ifutil.h:lookupName
    size_t low = 0;
    while (tab.size() > 3) {
        size_t mid = tab.size() / 2;
        if (const NameTable* ele = tab.at(mid)) {
            if (name.before(ele->name)) {
                tab.trim(mid);
            } else {
                tab.split(mid);
                low += mid;
            }
        } else {
            // cannot happen
            break;
        }
    }

    while (const NameTable* ele = tab.eat()) {
        if (name.match(ele->name)) {
            index = low;
            return true;
        }
        ++low;
    }
    return false;
}
