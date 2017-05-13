/**
  *  \file server/file/item.cpp
  */

#include "server/file/item.hpp"

// /** Constructor.
//     \param name Name of this object (basename)
//     \param parent Parent directory */
server::file::Item::Item(String_t name)
    : m_name(name)
{
    // ex UserItem::UserItem
}

server::file::Item::~Item()
{ }

const String_t&
server::file::Item::getName() const
{
    // ex UserItem::getName
    return m_name;
}

// UserDirectory*
// UserItem::getParent() const
// {
//     return parent;
// }
