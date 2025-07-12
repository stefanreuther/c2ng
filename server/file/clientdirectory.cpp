/**
  *  \file server/file/clientdirectory.cpp
  *  \brief Class server::file::ClientDirectory
  *
  *  Implementation Notes:
  *  It is important to convert the exceptions that come from the FileBaseClient into actual FileProblemException's.
  *  Directory's contract requires that file problems are FileProblemException's.
  *  If they are not derived from FileProblemException, functions such as openFileNT will not work.
  */

#include <stdexcept>
#include "server/file/clientdirectory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/unchangeabledirectoryentry.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Ref;
using afl::base::Ptr;
using afl::io::FileSystem;
using afl::string::PosixFileNames;
using server::interface::FileBase;
using server::interface::FileBaseClient;

/************************* ClientDirectory::Entry ************************/

/*
 *  DirectoryEntry implementation
 */
class server::file::ClientDirectory::Entry : public afl::io::UnchangeableDirectoryEntry {
 public:
    Entry(Ref<server::file::ClientDirectory> parent, String_t childName);

    virtual String_t getTitle();
    virtual String_t getPathName();
    virtual afl::base::Ref<afl::io::Stream> openFileForReading();
    virtual afl::base::Ref<afl::io::Directory> openDirectory();
    virtual afl::base::Ref<afl::io::Directory> openContainingDirectory();
    virtual void updateInfo(uint32_t requested);

    void setInfo(const FileBase::Info& info);

 private:
    String_t getFullName() const;

    Ref<server::file::ClientDirectory> m_parent;
    String_t m_childName;
    bool m_hasInfo;
};

server::file::ClientDirectory::Entry::Entry(Ref<server::file::ClientDirectory> parent, String_t childName)
    : UnchangeableDirectoryEntry(afl::string::Messages::cannotWrite()),
      m_parent(parent),
      m_childName(childName),
      m_hasInfo(false)
{ }

String_t
server::file::ClientDirectory::Entry::getTitle()
{
    return m_childName;
}

String_t
server::file::ClientDirectory::Entry::getPathName()
{
    return String_t();
}

afl::base::Ref<afl::io::Stream>
server::file::ClientDirectory::Entry::openFileForReading()
{
    String_t fullName = getFullName();
    try {
        afl::base::Ref<afl::io::InternalStream> s = *new afl::io::InternalStream();
        s->setName(fullName);
        s->fullWrite(afl::string::toBytes(FileBaseClient(m_parent->m_commandHandler).getFile(fullName)));
        s->setPos(0);
        s->setWritePermission(false);
        return s;
    }
    catch (std::exception& e) {
        throw afl::except::FileProblemException(fullName, e.what());
    }
}

afl::base::Ref<afl::io::Directory>
server::file::ClientDirectory::Entry::openDirectory()
{
    return *new server::file::ClientDirectory(m_parent->m_commandHandler, getFullName(), m_parent.asPtr());
}

afl::base::Ref<afl::io::Directory>
server::file::ClientDirectory::Entry::openContainingDirectory()
{
    return m_parent;
}

void
server::file::ClientDirectory::Entry::updateInfo(uint32_t /*requested*/)
{
    if (!m_hasInfo) {
        String_t fullName = getFullName();
        try {
            setInfo(FileBaseClient(m_parent->m_commandHandler).getFileInformation(fullName));
        }
        catch (std::exception& e) {
            throw afl::except::FileProblemException(fullName, e.what());
        }
    }
}

void
server::file::ClientDirectory::Entry::setInfo(const FileBase::Info& info)
{
    m_hasInfo = true;
    switch (info.type) {
     case FileBase::IsFile:       setFileType(tFile); break;
     case FileBase::IsDirectory:  setFileType(tDirectory); break;
     case FileBase::IsUnknown:    setFileType(tOther); break;
    }

    if (const int32_t* p = info.size.get()) {
        if (*p >= 0) {
            setFileSize(*p);
        }
    }
}

String_t
server::file::ClientDirectory::Entry::getFullName() const
{
    return PosixFileNames().makePathName(m_parent->m_basePath, m_childName);
}


/************************* ClientDirectory::Enum *************************/

class server::file::ClientDirectory::Enum : public afl::base::Enumerator<Ptr<afl::io::DirectoryEntry> > {
 public:
    Enum(Ref<server::file::ClientDirectory> parent);

    virtual bool getNextElement(Ptr<afl::io::DirectoryEntry>& result);

 private:
    Ref<server::file::ClientDirectory> m_parent;
    FileBase::ContentInfoMap_t m_content;
    FileBase::ContentInfoMap_t::iterator m_current;
};

server::file::ClientDirectory::Enum::Enum(Ref<server::file::ClientDirectory> parent)
    : m_parent(parent),
      m_content(),
      m_current()
{
    try {
        FileBaseClient(m_parent->m_commandHandler).getDirectoryContent(m_parent->m_basePath, m_content);
        m_current = m_content.begin();
    }
    catch (std::exception& e) {
        throw afl::except::FileProblemException(m_parent->m_basePath, e.what());
    }
}

bool
server::file::ClientDirectory::Enum::getNextElement(Ptr<afl::io::DirectoryEntry>& result)
{
    if (m_current != m_content.end()) {
        Ptr<Entry> e = new Entry(m_parent, m_current->first);
        e->setInfo(*m_current->second);
        result = e;
        ++m_current;
        return true;
    } else {
        return false;
    }
}

/**************************** ClientDirectory ****************************/

server::file::ClientDirectory::ClientDirectory(afl::net::CommandHandler& commandHandler, String_t basePath)
    : Directory(),
      m_commandHandler(commandHandler),
      m_basePath(basePath),
      m_parent()
{ }

server::file::ClientDirectory::ClientDirectory(afl::net::CommandHandler& commandHandler, String_t basePath, afl::base::Ptr<ClientDirectory> parent)
    : Directory(),
      m_commandHandler(commandHandler),
      m_basePath(basePath),
      m_parent(parent)
{ }

server::file::ClientDirectory::~ClientDirectory()
{ }

afl::base::Ref<server::file::ClientDirectory>
server::file::ClientDirectory::create(afl::net::CommandHandler& commandHandler, String_t basePath)
{
    return *new ClientDirectory(commandHandler, basePath);
}

afl::base::Ref<afl::io::DirectoryEntry>
server::file::ClientDirectory::getDirectoryEntryByName(String_t name)
{
    return *new Entry(*this, name);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
server::file::ClientDirectory::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
server::file::ClientDirectory::getParentDirectory()
{
    return m_parent;
}

String_t
server::file::ClientDirectory::getDirectoryName()
{
    return String_t();
}

String_t
server::file::ClientDirectory::getTitle()
{
    return m_basePath;
}

void
server::file::ClientDirectory::flush()
{ }
