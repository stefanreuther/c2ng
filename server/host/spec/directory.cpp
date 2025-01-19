/**
  *  \file server/host/spec/directory.cpp
  *  \brief Class server::host::spec::Directory
  */

#include "server/host/spec/directory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/messages.hpp"
#include "afl/string/posixfilenames.hpp"

using afl::base::Ref;
using afl::io::InternalStream;
using afl::string::PosixFileNames;
using server::interface::FileBase;

/*
 *  DirectoryEntry implementation
 */

class server::host::spec::Directory::Entry : public afl::io::DirectoryEntry {
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

server::host::spec::Directory::Entry::Entry(Ref<Directory> parent, const String_t& name)
    : m_parent(parent), m_name(name)
{ }

server::host::spec::Directory::Entry::~Entry()
{ }

String_t
server::host::spec::Directory::Entry::getTitle()
{
    return m_name;
}

String_t
server::host::spec::Directory::Entry::getPathName()
{
    return PosixFileNames().makePathName(m_parent->m_dirName, m_name);
}

Ref<afl::io::Stream>
server::host::spec::Directory::Entry::openFile(afl::io::FileSystem::OpenMode mode)
{
    /* File must
       - exist in directory
       - actually be a file
       - file access must be allowed
       - user must request OpenRead (no writing of any kind) */
    ContentInfoMap_t::const_iterator it = m_parent->find(m_name);
    if (it != m_parent->m_content.end()
        && it->second != 0
        && it->second->type == FileBase::IsFile
        && m_parent->m_enabled
        && mode == afl::io::FileSystem::OpenRead)
    {
        Ref<InternalStream> s = *new InternalStream();
        s->setName(m_name);
        s->fullWrite(afl::string::toBytes(m_parent->m_filer.getFile(m_parent->makePathName(it->first))));
        s->setPos(0);
        s->setWritePermission(false);
        return s;
    } else {
        throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
    }
}

Ref<afl::io::Directory>
server::host::spec::Directory::Entry::openDirectory()
{
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
}

Ref<afl::io::Directory>
server::host::spec::Directory::Entry::openContainingDirectory()
{
    return m_parent;
}

void
server::host::spec::Directory::Entry::updateInfo(uint32_t /*requested*/)
{
    ContentInfoMap_t::const_iterator it = m_parent->find(m_name);
    if (it != m_parent->m_content.end() && it->second != 0) {
        setInfo(*it->second);
    }
}

void
server::host::spec::Directory::Entry::doRename(String_t /*newName*/)
{
    return throwUnsupported();
}

void
server::host::spec::Directory::Entry::doErase()
{
    return throwUnsupported();
}

void
server::host::spec::Directory::Entry::doCreateAsDirectory()
{
    return throwUnsupported();
}

void
server::host::spec::Directory::Entry::doSetFlag(FileFlag /*flag*/, bool /*value*/)
{
    return throwUnsupported();
}

void
server::host::spec::Directory::Entry::doMoveTo(afl::io::Directory& /*dir*/, String_t /*name*/)
{
    return throwUnsupported();
}

void
server::host::spec::Directory::Entry::setInfo(const FileBase::Info& info)
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
server::host::spec::Directory::Entry::throwUnsupported()
{
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::invalidOperation());
}


/*
 *  Enum implementation
 */

class server::host::spec::Directory::Enum : public afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > {
 public:
    Enum(Ref<Directory> parent);
    virtual ~Enum();
    virtual bool getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result);

 private:
    Ref<Directory> m_parent;
    ContentInfoMap_t::const_iterator m_iterator;
};

server::host::spec::Directory::Enum::Enum(Ref<Directory> parent)
    : m_parent(parent),
      m_iterator(m_parent->m_content.begin())
{ }

server::host::spec::Directory::Enum::~Enum()
{ }

bool
server::host::spec::Directory::Enum::getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result)
{
    if (m_iterator != m_parent->m_content.end()) {
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

afl::base::Ref<server::host::spec::Directory>
server::host::spec::Directory::create(server::interface::FileBase& filer, String_t dirName)
{
    return *new Directory(filer, dirName);
}

server::host::spec::Directory::Directory(server::interface::FileBase& filer, String_t dirName)
    : afl::io::Directory(),
      m_filer(filer),
      m_dirName(dirName),
      m_enabled(true),
      m_content()
{
    m_filer.getDirectoryContent(m_dirName, m_content);
}

void
server::host::spec::Directory::setEnabled(bool flag)
{
    m_enabled = flag;
}

afl::base::Ref<afl::io::DirectoryEntry>
server::host::spec::Directory::getDirectoryEntryByName(String_t name)
{
    return *new Entry(*this, name);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
server::host::spec::Directory::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
server::host::spec::Directory::getParentDirectory()
{
    return 0;
}

String_t
server::host::spec::Directory::getDirectoryName()
{
    return m_dirName;
}

String_t
server::host::spec::Directory::getTitle()
{
    return PosixFileNames().getFileName(m_dirName);
}

String_t
server::host::spec::Directory::makePathName(const String_t& fileName) const
{
    return PosixFileNames().makePathName(m_dirName, fileName);
}

server::host::spec::Directory::ContentInfoMap_t::const_iterator
server::host::spec::Directory::find(const String_t& fileName) const
{
    ContentInfoMap_t::const_iterator it = m_content.find(fileName);
    if (it == m_content.end()) {
        // If pconfig.src does not exist, but pconfig.src.frag does, use that instead.
        it = m_content.find(fileName + ".frag");
    }
    return it;
}
