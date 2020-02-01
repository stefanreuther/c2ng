/**
  *  \file interpreter/propertyacceptor.cpp
  *  \brief Class interpreter::PropertyAcceptor
  */

#include "interpreter/propertyacceptor.hpp"

// Utility function: enumerate a NameMap object.
void
interpreter::PropertyAcceptor::enumNames(const afl::data::NameMap& names)
{
    // ex IntPropertyAcceptor::enumNames
    // ex ccexpr.pas:EnumNames
    for (afl::data::NameMap::Index_t i = 0, e = names.getNumNames(); i < e; ++i) {
        addProperty(names.getNameByIndex(i), thNone);
    }
}

// Utility function: enumerate a NameTable array.
void
interpreter::PropertyAcceptor::enumTable(afl::base::Memory<const NameTable> tab)
{
    // ex IntPropertyAcceptor::enumMapping
    // ex ccexpr.pas:EnumIdl (sort-of)
    while (const NameTable* p = tab.eat()) {
        addProperty(p->name, p->type);
    }
}
