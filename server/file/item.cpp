/**
  *  \file server/file/item.cpp
  */

#include "server/file/item.hpp"

// /** Constructor.
//     \param name Name of this object (basename) */
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
