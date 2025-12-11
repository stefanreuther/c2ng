/**
  *  \file server/file/clientdirectoryhandler.cpp
  *  \brief Class server::file::ClientDirectoryHandler
  */

#include "server/file/clientdirectoryhandler.hpp"
#include "server/interface/filebaseclient.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/net/reconnectable.hpp"

using server::interface::FileBase;
using server::interface::FileBaseClient;
using server::file::DirectoryHandler;

namespace {
    server::file::DirectoryHandler::Type convertType(FileBase::Type type)
    {
        DirectoryHandler::Type result = DirectoryHandler::IsUnknown;
        switch (type) {
         case FileBase::IsFile:      result = DirectoryHandler::IsFile;      break;
         case FileBase::IsDirectory: result = DirectoryHandler::IsDirectory; break;
         case FileBase::IsUnknown:   result = DirectoryHandler::IsUnknown;   break;
        }
        return result;
    }
}

// Constructor.
server::file::ClientDirectoryHandler::ClientDirectoryHandler(afl::net::CommandHandler& commandHandler, String_t basePath)
    : m_commandHandler(commandHandler),
      m_basePath(basePath)
{ }

String_t
server::file::ClientDirectoryHandler::getName()
{
    return m_basePath;
}

afl::base::Ref<afl::io::FileMapping>
server::file::ClientDirectoryHandler::getFile(const Info& info)
{
    return getFileByName(info.name);
}

afl::base::Ref<afl::io::FileMapping>
server::file::ClientDirectoryHandler::getFileByName(String_t name)
{
    String_t fileContent = FileBaseClient(m_commandHandler).getFile(makePath(name));
    afl::io::ConstMemoryStream stream(afl::string::toBytes(fileContent));
    return *new afl::io::InternalFileMapping(stream);
}

server::file::DirectoryHandler::Info
server::file::ClientDirectoryHandler::createFile(String_t name, afl::base::ConstBytes_t content)
{
    FileBaseClient(m_commandHandler).putFile(makePath(name), afl::string::fromBytes(content));

    // FIXME: this Info structure is synthetic. Server should normally supply it
    Info result(name, IsFile);
    result.size = static_cast<int32_t>(content.size());
    return result;
}

void
server::file::ClientDirectoryHandler::removeFile(String_t name)
{
    FileBaseClient(m_commandHandler).removeFile(makePath(name));
}

void
server::file::ClientDirectoryHandler::readContent(Callback& callback)
{
    FileBaseClient::ContentInfoMap_t content;
    FileBaseClient(m_commandHandler).getDirectoryContent(m_basePath, content);

    for (FileBaseClient::ContentInfoMap_t::const_iterator it = content.begin(); it != content.end(); ++it) {
        const FileBase::Info& in = *it->second;
        Info out(it->first, convertType(in.type));
        out.size = in.size;
        out.contentId = in.contentId;
        callback.addItem(out);
    }
}

server::file::DirectoryHandler*
server::file::ClientDirectoryHandler::getDirectory(const Info& info)
{
    return new ClientDirectoryHandler(m_commandHandler, makePath(info.name));
}

server::file::DirectoryHandler::Info
server::file::ClientDirectoryHandler::createDirectory(String_t name)
{
    FileBaseClient(m_commandHandler).createDirectory(makePath(name));
    return Info(name, IsDirectory);
}

void
server::file::ClientDirectoryHandler::removeDirectory(String_t name)
{
    // This is supposed to only remove empty directories.
    // FileBase::removeDirectory will also remove non-empty directories.
    // Therefore, use removeFile here.
    FileBaseClient(m_commandHandler).removeFile(makePath(name));
}

afl::base::Optional<server::file::DirectoryHandler::Info>
server::file::ClientDirectoryHandler::copyFile(ReadOnlyDirectoryHandler& source, const Info& sourceInfo, String_t name)
{
    ClientDirectoryHandler* sourceCDH = dynamic_cast<ClientDirectoryHandler*>(&source);
    if (sourceCDH != 0 && &m_commandHandler == &sourceCDH->m_commandHandler) {
        // Same CommandHandler.
        // The state (in particular, the user Id) is in the CommandHandler's other side.
        // Two ClientDirectoryHandler's with different user Ids on the same CommandHandler would not work in the naive implementation either,
        // so there's no point in detecting and refusing it here.
        FileBaseClient(m_commandHandler).copyFile(sourceCDH->makePath(sourceInfo.name), makePath(name));

        // FIXME: this Info structure is synthetic. Server should normally supply it
        Info result(name, IsFile);
        result.size = sourceInfo.size;
        result.contentId = sourceInfo.contentId;
        return result;
    } else {
        return afl::base::Nothing;
    }
}

server::file::DirectoryHandler::SnapshotHandler*
server::file::ClientDirectoryHandler::getSnapshotHandler()
{
    return 0;
}

String_t
server::file::ClientDirectoryHandler::makePath(String_t userPath)
{
    if (m_basePath.empty()) {
        return userPath;
    } else {
        return m_basePath + "/" + userPath;
    }
}
