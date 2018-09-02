/**
  *  \file server/host/hostfile.cpp
  *  \brief Class server::host::HostFile
  */

#include "server/host/hostfile.hpp"
#include "server/host/file/item.hpp"

using server::host::file::Item;
using server::interface::HostFile;

namespace {
    HostFile::Info completeInfo(Item& root, Item::ItemVector_t& vec)
    {
        HostFile::Info baseInfo = root.getInfo();
        for (size_t i = 0, n = vec.size(); i < n; ++i) {
            if (Item* p = vec[i]) {
                HostFile::Info itemInfo = p->getInfo();
                HostFile::mergeInfo(itemInfo, baseInfo);
                baseInfo = itemInfo;
            }
        }
        return baseInfo;
    }
}


server::host::HostFile::HostFile(server::host::file::Item& item)
    : m_item(item)
{ }

String_t
server::host::HostFile::getFile(String_t fileName)
{
    // Just fetch the content
    Item::ItemVector_t vec;
    Item& it = m_item.resolvePath(fileName, vec);
    return it.getContent();
}

void
server::host::HostFile::getDirectoryContent(String_t dirName, InfoVector_t& result)
{
    // Obtain item
    Item::ItemVector_t vec;
    Item& it = m_item.resolvePath(dirName, vec);

    // Obtain complete information
    const Info baseInfo = completeInfo(m_item, vec);

    // Produce output
    Item::ItemVector_t contentVec;
    it.listContent(contentVec);
    for (size_t i = 0, n = contentVec.size(); i < n; ++i) {
        if (Item* p = contentVec[i]) {
            Info itemInfo = p->getInfo();
            mergeInfo(itemInfo, baseInfo);
            result.push_back(itemInfo);
        }
    }
}

server::interface::HostFile::Info
server::host::HostFile::getFileInformation(String_t fileName)
{
    // Obtain item
    Item::ItemVector_t vec;
    m_item.resolvePath(fileName, vec);

    // Complete information
    return completeInfo(m_item, vec);
}

void
server::host::HostFile::getPathDescription(String_t dirName, InfoVector_t& result)
{
    // Obtain item
    Item::ItemVector_t vec;
    m_item.resolvePath(dirName, vec);

    // Complete information, producing output on each step
    Info baseInfo = m_item.getInfo();
    for (size_t i = 0, n = vec.size(); i < n; ++i) {
        if (Item* p = vec[i]) {
            Info itemInfo = p->getInfo();
            mergeInfo(itemInfo, baseInfo);
            baseInfo = itemInfo;
            result.push_back(baseInfo);
        }
    }
}
