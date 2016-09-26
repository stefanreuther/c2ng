/**
  *  \file interpreter/nametable.cpp
  */

#include "interpreter/nametable.hpp"

// /** Look up a name in a mapping list.
//     \param name    Name of property to find
//     \param mapping Mapping list, must be sorted
//     \param size    Size of mapping list
//     \return index such that mapping[return].name == name, or -1 */
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
