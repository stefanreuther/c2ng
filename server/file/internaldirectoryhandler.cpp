/**
  *  \file server/file/internaldirectoryhandler.cpp
  *  \brief Class server::file::InternalDirectoryHandler
  */

#include "server/file/internaldirectoryhandler.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/internalfilemapping.hpp"
#include "afl/string/posixfilenames.hpp"

server::file::InternalDirectoryHandler::InternalDirectoryHandler(String_t name, Directory& dir)
    : m_name(name),
      m_dir(dir)
{ }

String_t
server::file::InternalDirectoryHandler::getName()
{
    return m_name;
}

afl::base::Ref<afl::io::FileMapping>
server::file::InternalDirectoryHandler::getFile(const Info& info)
{
    return getFileByName(info.name);
}

afl::base::Ref<afl::io::FileMapping>
server::file::InternalDirectoryHandler::getFileByName(String_t name)
{
    if (File* p = findFile(name)) {
        afl::io::ConstMemoryStream ms(p->content);
        return *new afl::io::InternalFileMapping(ms);
    }
    throw afl::except::FileProblemException(makeName(name), "File not found");
}

server::file::DirectoryHandler::Info
server::file::InternalDirectoryHandler::createFile(String_t name, afl::base::ConstBytes_t content)
{
    if (findDirectory(name)) {
        throw afl::except::FileProblemException(makeName(name), "Is a directory");
    }

    File* p = findFile(name);
    if (p == 0) {
        p = m_dir.files.pushBackNew(new File(name));
    }
    p->content.clear();
    p->content.append(content);

    Info result(name, IsFile);
    result.size = convertSize(p->content.size());
    return result;
}

void
server::file::InternalDirectoryHandler::removeFile(String_t name)
{
    for (size_t i = 0, n = m_dir.files.size(); i < n; ++i) {
        if (File* p = m_dir.files[i]) {
            if (p->name == name) {
                m_dir.files.erase(m_dir.files.begin() + i);
                return;
            }
        }
    }
    throw afl::except::FileProblemException(makeName(name), "No such file");
}

void
server::file::InternalDirectoryHandler::readContent(Callback& callback)
{
    for (size_t i = 0, n = m_dir.subdirectories.size(); i < n; ++i) {
        if (Directory* p = m_dir.subdirectories[i]) {
            Info info(p->name, IsDirectory);
            callback.addItem(info);
        }
    }
    for (size_t i = 0, n = m_dir.files.size(); i < n; ++i) {
        if (File* p = m_dir.files[i]) {
            Info info(p->name, IsFile);
            info.size = convertSize(p->content.size());
            callback.addItem(info);
        }
    }
}
server::file::DirectoryHandler*
server::file::InternalDirectoryHandler::getDirectory(const Info& info)
{
    if (Directory* p = findDirectory(info.name)) {
        return new InternalDirectoryHandler(m_name + "/" + info.name, *p);
    } else {
        throw afl::except::FileProblemException(makeName(info.name), "No such directory");
    }
}

server::file::DirectoryHandler::Info
server::file::InternalDirectoryHandler::createDirectory(String_t name)
{
    if (findDirectory(name) || findFile(name)) {
        throw afl::except::FileProblemException(makeName(name), "Already exists");
    }

    m_dir.subdirectories.pushBackNew(new Directory(name));

    return Info(name, IsDirectory);
}

void
server::file::InternalDirectoryHandler::removeDirectory(String_t name)
{
    for (size_t i = 0, n = m_dir.subdirectories.size(); i < n; ++i) {
        if (Directory* p = m_dir.subdirectories[i]) {
            if (p->name == name) {
                m_dir.subdirectories.erase(m_dir.subdirectories.begin() + i);
                return;
            }
        }
    }
    throw afl::except::FileProblemException(makeName(name), "No such directory");
}

afl::base::Optional<server::file::DirectoryHandler::Info>
server::file::InternalDirectoryHandler::copyFile(ReadOnlyDirectoryHandler& /*source*/, const Info& /*sourceInfo*/, String_t /*name*/)
{
    // There is no point in optimizing this.
    return afl::base::Nothing;
}

server::file::DirectoryHandler::SnapshotHandler*
server::file::InternalDirectoryHandler::getSnapshotHandler()
{
    return 0;
}

afl::base::Ptr<afl::io::Directory>
server::file::InternalDirectoryHandler::getDirectory()
{
    return 0;
}

server::file::InternalDirectoryHandler::File*
server::file::InternalDirectoryHandler::findFile(const String_t& name)
{
    for (size_t i = 0, n = m_dir.files.size(); i < n; ++i) {
        if (File* p = m_dir.files[i]) {
            if (p->name == name) {
                return p;
            }
        }
    }
    return 0;
}

server::file::InternalDirectoryHandler::Directory*
server::file::InternalDirectoryHandler::findDirectory(const String_t& name)
{
    for (size_t i = 0, n = m_dir.subdirectories.size(); i < n; ++i) {
        if (Directory* p = m_dir.subdirectories[i]) {
            if (p->name == name) {
                return m_dir.subdirectories[i];
            }
        }
    }
    return 0;
}

String_t
server::file::InternalDirectoryHandler::makeName(const String_t& childName) const
{
    return afl::string::PosixFileNames().makePathName(m_name, childName);
}
