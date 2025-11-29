/**
  *  \file server/file/item.cpp
  *  \brief Class server::file::Item
  */

#include "server/file/item.hpp"

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
