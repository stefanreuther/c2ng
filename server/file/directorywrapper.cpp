/**
  *  \file server/file/directorywrapper.cpp
  *  \brief Class server::file::DirectoryWrapper
  */

#include "server/file/directorywrapper.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/messages.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/fileitem.hpp"
#include "util/serverdirectory.hpp"

inline
server::file::DirectoryWrapper::DirectoryWrapper(DirectoryItem& item)
    : m_item(item)
{ }

// Get file content.
void
server::file::DirectoryWrapper::getFile(String_t name, afl::base::GrowableBytes_t& data)
{
    if (FileItem* p = m_item.findFile(name)) {
        afl::base::Ref<afl::io::FileMapping> map = m_item.getFileContent(*p);
        data.append(map->get());
    } else {
        // Should not happen
        throw afl::except::FileProblemException(name, afl::string::Messages::fileNotFound());
    }
}

// Store file content.
void
server::file::DirectoryWrapper::putFile(String_t /*name*/, afl::base::ConstBytes_t /*data*/)
{ }

// Erase a file.
void
server::file::DirectoryWrapper::eraseFile(String_t /*name*/)
{ }

// Get content of directory.
void
server::file::DirectoryWrapper::getContent(std::vector<util::ServerDirectory::FileInfo>& result)
{
    size_t index = 0;
    while (FileItem* p = m_item.getFileByIndex(index)) {
        result.push_back(util::ServerDirectory::FileInfo(p->getName(), p->getInfo().size.orElse(0), true));
        ++index;
    }
}

// Check validity of a file name.
bool
server::file::DirectoryWrapper::isValidFileName(String_t /*name*/) const
{
    return true;
}

// Check permission to write.
bool
server::file::DirectoryWrapper::isWritable() const
{
    return false;
}

/*
 *  Main entry point
 */

// Create Directory object wrapping the given item.
afl::base::Ref<afl::io::Directory>
server::file::DirectoryWrapper::create(DirectoryItem& item)
{
    return util::ServerDirectory::create(*new DirectoryWrapper(item), item.getName(), 0);
}
