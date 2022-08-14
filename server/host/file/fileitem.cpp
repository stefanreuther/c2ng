/**
  *  \file server/host/file/fileitem.cpp
  *  \brief Class server::host::file::FileItem
  */

#include <cstring>
#include <algorithm>
#include "server/host/file/fileitem.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"

using server::host::file::FileItem;
using server::interface::BaseClient;
using server::interface::FileBaseClient;

namespace {
    /* Create FileItem from information in filer.
       @param filer         Filer
       @param fileInfo      Information retrieved from filer
       @param fileName      File name (without path name)
       @param fullName      Full name on filer
       @param userName      User name for access checking
       @return newly-allocated FileItem */
    FileItem* createFileItem(afl::net::CommandHandler& filer, const FileBaseClient::Info& fileInfo, const String_t& fileName, const String_t& fullName, const String_t& userName)
    {
        FileItem::Info_t i;
        i.name = fileName;
        static_cast<FileBaseClient::Info&>(i) = fileInfo;
        return new FileItem(filer, fullName, userName, i);
    }
}


server::host::file::FileItem::FileItem(afl::net::CommandHandler& filer, String_t fullName, String_t userName, Info_t info)
    : m_filer(filer),
      m_fullName(fullName),
      m_userName(userName),
      m_info(info)
{ }

String_t
server::host::file::FileItem::getName()
{
    return m_info.name;
}

server::host::file::Item::Info_t
server::host::file::FileItem::getInfo()
{
    return m_info;
}

server::host::file::Item*
server::host::file::FileItem::find(const String_t& name)
{
    return defaultFind(name);
}

void
server::host::file::FileItem::listContent(ItemVector_t& out)
{
    defaultList(out);
}

String_t
server::host::file::FileItem::getContent()
{
    BaseClient(m_filer).setUserContext(m_userName);
    return FileBaseClient(m_filer).getFile(m_fullName);
}

void
server::host::file::FileItem::listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, ItemVector_t& out)
{
    FileBaseClient::ContentInfoMap_t files;
    BaseClient(filer).setUserContext(userName);
    FileBaseClient(filer).getDirectoryContent(pathName, files);
    for (FileBaseClient::ContentInfoMap_t::const_iterator it = files.begin(); it != files.end(); ++it) {
        if (const FileBaseClient::Info* p = it->second) {
            if (p->type == FileBaseClient::IsFile) {
                out.pushBackNew(createFileItem(filer, *p, it->first, pathName + "/" + it->first, userName));
            }
        }
    }
}

void
server::host::file::FileItem::listFileServerContent(afl::net::CommandHandler& filer, const String_t& pathName, const String_t& userName, const afl::data::StringList_t& filter, ItemVector_t& out)
{
    if (filter.empty()) {
        // We will not match anything.
    } else if (filter.size() == 1) {
        // We will match only a single item. Try it.
        BaseClient(filer).setUserContext(userName);
        try {
            const String_t fullName = pathName + "/" + filter[0];
            const FileBaseClient::Info fi = FileBaseClient(filer).getFileInformation(fullName);
            out.pushBackNew(createFileItem(filer, fi, filter[0], fullName, userName));
        }
        catch (std::exception& e) {
            // If it's a 404, we cannot find the file, i.e. don't throw and report no file found.
            // Other errors indicate different problems and should be forwarded to the caller.
            if (std::strncmp(e.what(), "404 ", 4) != 0) {
                throw;
            }
        }
    } else {
        // General case
        FileBaseClient::ContentInfoMap_t files;
        BaseClient(filer).setUserContext(userName);
        FileBaseClient(filer).getDirectoryContent(pathName, files);
        for (FileBaseClient::ContentInfoMap_t::const_iterator it = files.begin(); it != files.end(); ++it) {
            if (const FileBaseClient::Info* p = it->second) {
                if (p->type == FileBaseClient::IsFile && std::find(filter.begin(), filter.end(), it->first) != filter.end()) {
                    out.pushBackNew(createFileItem(filer, *p, it->first, pathName + "/" + it->first, userName));
                }
            }
        }
    }
}
