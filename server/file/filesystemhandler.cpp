/**
  *  \file server/file/filesystemhandler.cpp
  */

#include <stdexcept>
#include "server/file/filesystemhandler.hpp"
#include "afl/io/stream.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internalfilemapping.hpp"

server::file::FileSystemHandler::FileSystemHandler(afl::io::FileSystem& fs, String_t name)
    : m_fileSystem(fs),
      m_name(name)
{ }

server::file::FileSystemHandler::~FileSystemHandler()
{ }

String_t
server::file::FileSystemHandler::getName()
{
    return m_name;
}

afl::base::Ref<afl::io::FileMapping>
server::file::FileSystemHandler::getFile(const Info& info)
{
    return getFileByName(info.name);
}

afl::base::Ref<afl::io::FileMapping>
server::file::FileSystemHandler::getFileByName(String_t name)
{
    // We return a FileMapping to avoid copying.
    // In addition, using a file mapping is a convenient way to load a file in one go.
    // We could use a native FileMapping (createVirtualMapping()):
    //     return m_fileSystem.openFile(m_fileSystem.makePathName(m_name, name), afl::io::FileSystem::OpenRead)->createVirtualMapping();
    // While in theory this could be faster by working directly on the Kernel's buffer cache, in practice it is slower than using an InternalFileMapping.
    // Test case: repeat 10000 get u/streu/xmmsctl.sh, 20170218
    //    c2file classic:                  0.20s
    //    c2file-ng with native mapping:   0.35s
    //    c2file-ng with internal mapping: 0.25s
    // (c2file classic's advantage most likely comes from more aggressively avoiding copies in the RESP serializer later on.)
    // In addition, using a native mapping has the disadvantage that its only means of reporting errors is SIGBUS.
    // Therefore we use the InternalFileMapping which is just a wrapper around a malloced buffer + read.
    return *new afl::io::InternalFileMapping(*m_fileSystem.openFile(m_fileSystem.makePathName(m_name, name), afl::io::FileSystem::OpenRead));
}

server::file::FileSystemHandler::Info
server::file::FileSystemHandler::createFile(String_t name, afl::base::ConstBytes_t content)
{
    // Create it
    m_fileSystem.openFile(m_fileSystem.makePathName(m_name, name), afl::io::FileSystem::Create)->fullWrite(content);

    // Produce result
    Info result(name, IsFile);
    result.size = convertSize(content.size());
    return result;
}

void
server::file::FileSystemHandler::removeFile(String_t name)
{
    m_fileSystem.openDirectory(m_name)->erase(name);
}

afl::base::Optional<server::file::DirectoryHandler::Info>
server::file::FileSystemHandler::copyFile(ReadOnlyDirectoryHandler& /*source*/, const Info& /*sourceInfo*/, String_t /*name*/)
{
    // The best we can do is as good as the naive implementation, so don't optimize this.
    return afl::base::Nothing;
}

void
server::file::FileSystemHandler::readContent(Callback& callback)
{
    using afl::io::DirectoryEntry;
    using afl::base::Ref;
    using afl::base::Ptr;
    using afl::base::Enumerator;
    Ref<Enumerator<Ptr<DirectoryEntry> > > it = m_fileSystem.openDirectory(m_name)->getDirectoryEntries();
    Ptr<DirectoryEntry> p;
    while (it->getNextElement(p)) {
        Info i(p->getTitle(), IsUnknown);
        switch (p->getFileType()) {
         case DirectoryEntry::tFile:
            i.size = convertSize(p->getFileSize());
            i.type = IsFile;
            break;
         case DirectoryEntry::tDirectory:
            i.type = IsDirectory;
            break;

         case DirectoryEntry::tUnknown:
         case DirectoryEntry::tArchive:
         case DirectoryEntry::tDevice:
         case DirectoryEntry::tRoot:
         case DirectoryEntry::tOther:
            i.type = IsUnknown;
            break;
        }
        callback.addItem(i);
    }
}

server::file::DirectoryHandler*
server::file::FileSystemHandler::getDirectory(const Info& info)
{
    return new FileSystemHandler(m_fileSystem, m_fileSystem.makePathName(m_name, info.name));
}

server::file::FileSystemHandler::Info
server::file::FileSystemHandler::createDirectory(String_t name)
{
    m_fileSystem.openDirectory(m_name)->getDirectoryEntryByName(name)->createAsDirectory();

    return Info(name, IsDirectory);
}

void
server::file::FileSystemHandler::removeDirectory(String_t name)
{
    m_fileSystem.openDirectory(m_name)->erase(name);
}

server::file::DirectoryHandler::SnapshotHandler*
server::file::FileSystemHandler::getSnapshotHandler()
{
    return 0;
}

afl::base::Ptr<afl::io::Directory>
server::file::FileSystemHandler::getDirectory()
{
    return m_fileSystem.openDirectory(m_name).asPtr();
}
