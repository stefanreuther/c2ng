/**
  *  \file interpreter/propertyacceptor.cpp
  */

#include "interpreter/propertyacceptor.hpp"

void
interpreter::PropertyAcceptor::enumNames(const afl::data::NameMap& names)
{
    // ex IntPropertyAcceptor::enumNames
    for (afl::data::NameMap::Index_t i = 0, e = names.getNumNames(); i < e; ++i) {
        addProperty(names.getNameByIndex(i), thNone);
    }
}

void
interpreter::PropertyAcceptor::enumTable(afl::base::Memory<const NameTable> tab)
{
    // ex IntPropertyAcceptor::enumMapping
    while (const NameTable* p = tab.eat()) {
        addProperty(p->name, TypeHint(p->type));
    }
}
