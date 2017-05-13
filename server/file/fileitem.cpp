/**
  *  \file server/file/fileitem.cpp
  */

#include "server/file/fileitem.hpp"

server::file::FileItem::FileItem(const DirectoryHandler::Info& itemInfo)
    : Item(itemInfo.name), m_info(itemInfo)
{ }

const server::file::DirectoryHandler::Info&
server::file::FileItem::getInfo() const
{
    // ex UserFile::getSize (well....)
    return m_info;
}

void
server::file::FileItem::setInfo(const DirectoryHandler::Info& info)
{
    m_info = info;
}
