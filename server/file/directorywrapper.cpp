/**
  *  \file server/file/directorywrapper.cpp
  *  \brief Class server::file::DirectoryWrapper
  */

#include "server/file/directorywrapper.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/constmemorystream.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/messages.hpp"
#include "server/file/directoryitem.hpp"
#include "server/file/fileitem.hpp"

/** Implementation of a file for DirectoryWrapper.
    This is just a ConstMemoryStream, but we need a way to keep the FileMapping alive. */
class server::file::DirectoryWrapper::File : public afl::io::ConstMemoryStream {
 public:
    File(afl::base::Ref<afl::io::FileMapping> map)
        : ConstMemoryStream(map->get()),
          m_map(map)
        { }
    ~File()
        { }
 private:
    afl::base::Ref<afl::io::FileMapping> m_map;
};

/** Implementation of DirectoryEntry for DirectoryWrapper. */
class server::file::DirectoryWrapper::Entry : public afl::io::DirectoryEntry {
 public:
    Entry(afl::base::Ref<DirectoryWrapper> parent, server::file::FileItem& item)
        : m_parent(parent), m_item(item)
        { }
    virtual String_t getTitle()
        { return m_item.getName(); }
    virtual String_t getPathName()
        { return String_t(); }
    virtual afl::base::Ref<afl::io::Stream> openFile(afl::io::FileSystem::OpenMode mode)
        {
            if (mode != afl::io::FileSystem::OpenRead) {
                throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotWrite());
            }
            return *new File(m_parent->m_item.getFileContent(m_item));
        }
    virtual afl::base::Ref<afl::io::Directory> openDirectory()
        { throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotAccessDirectories()); }
    virtual afl::base::Ref<afl::io::Directory> openContainingDirectory()
        { return m_parent; }
    virtual void updateInfo(uint32_t /*requested*/)
        {
            setFileType(tFile);
            if (const int32_t* p = m_item.getInfo().size.get()) {
                setFileSize(*p);
            }
        }
    virtual void doRename(String_t /*newName*/)
        { throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotWrite()); }
    virtual void doErase()
        { throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotWrite()); }
    virtual void doCreateAsDirectory()
        { throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotWrite()); }
    virtual void doSetFlag(FileFlag /*flag*/, bool /*value*/)
        { throw afl::except::FileProblemException(getTitle(), afl::string::Messages::cannotWrite()); }
 private:
    afl::base::Ref<DirectoryWrapper> m_parent;
    server::file::FileItem& m_item;
};

/** Implementation of Enum for DirectoryWrapper. */
class server::file::DirectoryWrapper::Enum : public afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > {
 public:
    Enum(afl::base::Ref<DirectoryWrapper> parent)
        : m_parent(parent),
          m_index(0)
        { }
    virtual bool getNextElement(afl::base::Ptr<afl::io::DirectoryEntry>& result)
        {
            if (server::file::FileItem* p = m_parent->m_item.getFileByIndex(m_index)) {
                ++m_index;
                result = new Entry(m_parent, *p);
                return true;
            } else {
                result = 0;
                return false;
            }
        }
 private:
    afl::base::Ref<DirectoryWrapper> m_parent;
    size_t m_index;
};

/**************************** DirectoryWrapper ***************************/

afl::base::Ref<server::file::DirectoryWrapper>
server::file::DirectoryWrapper::create(DirectoryItem& item)
{
    return *new DirectoryWrapper(item);
}

server::file::DirectoryWrapper::DirectoryWrapper(DirectoryItem& item)
    : m_item(item)
{ }

server::file::DirectoryWrapper::~DirectoryWrapper()
{ }

afl::base::Ref<afl::io::DirectoryEntry>
server::file::DirectoryWrapper::getDirectoryEntryByName(String_t name)
{
    if (server::file::FileItem* p = m_item.findFile(name)) {
        return *new Entry(*this, *p);
    } else {
        throw afl::except::FileProblemException(name, afl::string::Messages::cannotAccessFiles());
    }
}


afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
server::file::DirectoryWrapper::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
server::file::DirectoryWrapper::getParentDirectory()
{
    return 0;
}

String_t
server::file::DirectoryWrapper::getDirectoryName()
{
    return String_t();
}

String_t
server::file::DirectoryWrapper::getTitle()
{
    return m_item.getName();
}
