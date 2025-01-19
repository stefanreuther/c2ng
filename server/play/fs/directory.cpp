/**
  *  \file server/play/fs/directory.cpp
  *  \brief Class server::play::fs::Directory
  */

#include "server/play/fs/directory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/filemapping.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/io/multiplexablestream.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"
#include "server/interface/baseclient.hpp"
#include "server/interface/filebaseclient.hpp"

using afl::base::Ref;
using afl::string::PosixFileNames;
using server::interface::FileBase;
using server::interface::FileBaseClient;

namespace {
    /* Externally provided file names must start with a slash,
       but file names used in the server communication must not start with a slash. */
    String_t trimSlash(String_t pathName)
    {
        if (pathName.empty() || pathName[0] != '/') {
            throw afl::except::FileProblemException(pathName, "<invalid name>");
        }
        return pathName.substr(1);
    }
}

/*
 *  Stream implementation
 *
 *  Delegates most operations to an InternalStream, but implement flush/close to write back to server.
 */

class server::play::fs::Directory::Stream : public afl::io::MultiplexableStream {
 public:
    Stream(Ref<Directory> parent, String_t fileName, bool writable);
    ~Stream();

    virtual size_t read(Bytes_t m);
    virtual size_t write(ConstBytes_t m);
    virtual void flush();
    virtual void setPos(FileSize_t pos);
    virtual FileSize_t getPos();
    virtual FileSize_t getSize();
    virtual uint32_t getCapabilities();
    virtual String_t getName();
    virtual afl::base::Ptr<afl::io::FileMapping> createFileMapping(FileSize_t limit);

    void setContent(const String_t& content);

 private:
    Ref<Directory> m_parent;
    String_t m_fileName;
    bool m_writable;
    afl::io::InternalStream m_worker;
};

server::play::fs::Directory::Stream::Stream(Ref<Directory> parent, String_t fileName, bool writable)
    : m_parent(parent), m_fileName(fileName), m_writable(writable)
{ }

server::play::fs::Directory::Stream::~Stream()
{
    try {
        Stream::flush();
    }
    catch (...) {
        // Now what?
    }
}

size_t
server::play::fs::Directory::Stream::read(Bytes_t m)
{
    return m_worker.read(m);
}

size_t
server::play::fs::Directory::Stream::write(ConstBytes_t m)
{
    if (!m_writable) {
        throw afl::except::FileProblemException(*this, afl::string::Messages::cannotWrite());
    }
    return m_worker.write(m);
}

void
server::play::fs::Directory::Stream::flush()
{
    if (m_writable) {
        FileBaseClient(m_parent->m_session->fileClient()).putFile(m_parent->makePathName(m_fileName), afl::string::fromBytes(m_worker.getContent()));
    }
}

void
server::play::fs::Directory::Stream::setPos(FileSize_t pos)
{
    m_worker.setPos(pos);
}

afl::io::Stream::FileSize_t
server::play::fs::Directory::Stream::getPos()
{
    return m_worker.getPos();
}

afl::io::Stream::FileSize_t
server::play::fs::Directory::Stream::getSize()
{
    return m_worker.getSize();
}

uint32_t
server::play::fs::Directory::Stream::getCapabilities()
{
    uint32_t result = m_worker.getCapabilities();
    if (!m_writable) {
        result &= ~CanWrite;
    }
    return result;
}

String_t
server::play::fs::Directory::Stream::getName()
{
    return PosixFileNames().makePathName(m_parent->m_dirName, m_fileName);
}

afl::base::Ptr<afl::io::FileMapping>
server::play::fs::Directory::Stream::createFileMapping(FileSize_t limit)
{
    return m_worker.createFileMapping(limit);
}

void
server::play::fs::Directory::Stream::setContent(const String_t& content)
{
    m_worker.write(afl::string::toBytes(content));
    m_worker.setPos(0);
}



/*
 *  DirectoryEntry implementation
 */

class server::play::fs::Directory::Entry : public afl::io::DirectoryEntry {
 public:
    Entry(Ref<Directory> parent, const String_t& name);
    ~Entry();
    virtual String_t getTitle();
    virtual String_t getPathName();
    virtual Ref<afl::io::Stream> openFile(afl::io::FileSystem::OpenMode mode);
    virtual Ref<afl::io::Directory> openDirectory();
    virtual Ref<afl::io::Directory> openContainingDirectory();
    virtual void updateInfo(uint32_t requested);
    virtual void doRename(String_t newName);
    virtual void doErase();
    virtual void doCreateAsDirectory();
    virtual void doSetFlag(FileFlag flag, bool value);
    virtual void doMoveTo(afl::io::Directory& dir, String_t name);

    void setInfo(const FileBase::Info& info);
    void throwUnsupported();
 private:
    Ref<Directory> m_parent;
    String_t m_name;
};

server::play::fs::Directory::Entry::Entry(Ref<Directory> parent, const String_t& name)
    : m_parent(parent), m_name(name)
{ }

server::play::fs::Directory::Entry::~Entry()
{ }

String_t
server::play::fs::Directory::Entry::getTitle()
{
    return m_name;
}

String_t
server::play::fs::Directory::Entry::getPathName()
{
    return PosixFileNames().makePathName(m_parent->m_dirName, m_name);
}

Ref<afl::io::Stream>
server::play::fs::Directory::Entry::openFile(afl::io::FileSystem::OpenMode mode)
{
    switch (mode) {
     case afl::io::FileSystem::OpenRead: {
        Ref<Stream> s = *new Stream(m_parent, m_name, false);
        s->setContent(FileBaseClient(m_parent->m_session->fileClient()).getFile(m_parent->makePathName(m_name)));
        return s;
     }

     case afl::io::FileSystem::OpenWrite: {
        Ref<Stream> s = *new Stream(m_parent, m_name, true);
        s->setContent(FileBaseClient(m_parent->m_session->fileClient()).getFile(m_parent->makePathName(m_name)));
        return s;
     }

     case afl::io::FileSystem::Create:
        return *new Stream(m_parent, m_name, true);

     case afl::io::FileSystem::CreateNew:
        // Not implemented - not needed
        break;
    }
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
}

Ref<afl::io::Directory>
server::play::fs::Directory::Entry::openDirectory()
{
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
}

Ref<afl::io::Directory>
server::play::fs::Directory::Entry::openContainingDirectory()
{
    return m_parent;
}

void
server::play::fs::Directory::Entry::updateInfo(uint32_t requested)
{
    if ((requested & (InfoType | InfoSize)) != 0) {
        setInfo(FileBaseClient(m_parent->m_session->fileClient()).getFileInformation(m_parent->makePathName(m_name)));
    }
}

void
server::play::fs::Directory::Entry::doRename(String_t /*newName*/)
{
    throwUnsupported();
}

void
server::play::fs::Directory::Entry::doErase()
{
    FileBaseClient(m_parent->m_session->fileClient()).removeFile(m_parent->makePathName(m_name));
}

void
server::play::fs::Directory::Entry::doCreateAsDirectory()
{
    throwUnsupported();
}

void
server::play::fs::Directory::Entry::doSetFlag(FileFlag /*flag*/, bool /*value*/)
{
    throwUnsupported();
}

void
server::play::fs::Directory::Entry::doMoveTo(afl::io::Directory& /*dir*/, String_t /*name*/)
{
    throwUnsupported();
}

void
server::play::fs::Directory::Entry::setInfo(const FileBase::Info& info)
{
    // File type
    FileType ft = tOther;
    switch (info.type) {
     case FileBase::IsFile:       ft = tFile;      break;
     case FileBase::IsDirectory:  ft = tDirectory; break;
     case FileBase::IsUnknown:                     break;
    }
    setFileType(ft);

    // Visibility -> not mapped

    // Size
    if (const int32_t* pSize = info.size.get()) {
        setFileSize(*pSize);
    }

    // Content Id -> not mapped
}

void
server::play::fs::Directory::Entry::throwUnsupported()
{
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
}



/*
 *  Enum implementation
 */

class server::play::fs::Directory::Enum : public afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > {
 public:
    Enum(Ref<Directory> parent);
    virtual ~Enum();
    virtual bool getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result);

 private:
    Ref<Directory> m_parent;
    FileBase::ContentInfoMap_t m_content;
    FileBase::ContentInfoMap_t::const_iterator m_iterator;
};

server::play::fs::Directory::Enum::Enum(Ref<Directory> parent)
    : m_parent(parent),
      m_content(),
      m_iterator()
{
    FileBaseClient(m_parent->m_session->fileClient()).getDirectoryContent(trimSlash(m_parent->m_dirName), m_content);
    m_iterator = m_content.begin();
}

server::play::fs::Directory::Enum::~Enum()
{ }

bool
server::play::fs::Directory::Enum::getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result)
{
    if (m_iterator != m_content.end()) {
        Ref<Entry> e = *new Entry(m_parent, m_iterator->first);
        e->setInfo(*m_iterator->second);
        result = e.asPtr();
        ++m_iterator;
        return true;
    } else {
        return false;
    }
}


/*
 *  Directory
 */

afl::base::Ref<server::play::fs::Directory>
server::play::fs::Directory::create(afl::base::Ref<Session> session, String_t dirName)
{
    return *new Directory(session, dirName);
}

inline
server::play::fs::Directory::Directory(const afl::base::Ref<Session>& session, const String_t& dirName)
    : m_session(session),
      m_dirName(dirName)
{ }

afl::base::Ref<afl::io::DirectoryEntry>
server::play::fs::Directory::getDirectoryEntryByName(String_t name)
{
    return *new Entry(*this, name);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
server::play::fs::Directory::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
server::play::fs::Directory::getParentDirectory()
{
    return 0;
}

String_t
server::play::fs::Directory::getDirectoryName()
{
    return PosixFileNames().getFileName(m_dirName);
}

String_t
server::play::fs::Directory::getTitle()
{
    return m_dirName;
}

String_t
server::play::fs::Directory::makePathName(const String_t& fileName) const
{
    return trimSlash(PosixFileNames().makePathName(m_dirName, fileName));
}
