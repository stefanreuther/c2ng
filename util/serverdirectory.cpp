/**
  *  \file util/serverdirectory.cpp
  *  \brief Class util::ServerDirectory
  */

#include "util/serverdirectory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/internalstream.hpp"
#include "afl/string/messages.hpp"

using afl::base::Ptr;
using afl::base::Ref;
using afl::except::FileProblemException;
using afl::io::Directory;
using afl::io::DirectoryEntry;
using afl::io::FileSystem;
using afl::io::InternalStream;
using afl::io::Stream;
using afl::string::Messages;

/*
 *  File - Representation of local data for a file
 */

struct util::ServerDirectory::File {
    /** Status. */
    enum State {
        /** Not a file. Cannot be read or written. */
        NotFile,

        /** File on server, has not yet been loaded (data is invalid).
            Size is reported from @c size attribute. */
        UnreadFile,

        /** Clean file. Has been loaded.
            Size is reported from @c data attribute. */
        CleanFile,

        /** Dirty file. Has been loaded and modified.
            Size is reported from @c data attribute. */
        DirtyFile,

        /** New file. Has been newly created.
            Size is reported from @c data attribute. */
        NewFile,

        /** Deleted file. */
        DeletedFile,

        /** Gone file. Has been created but then removed. */
        GoneFile
    };

    /** File name. */
    String_t name;

    /** State. */
    State state;

    /** File size for UnreadFile. */
    Stream::FileSize_t size;

    /** File data; valid/non-null for CleanFile, DirtyFile. */
    Ptr<InternalStream> data;

    File(const String_t& name, Stream::FileSize_t size, State state)
        : name(name), state(state), size(size), data()
        { }

    bool isDeleted() const
        { return state == DeletedFile || state == GoneFile; }
};

/*
 *  Entry - Implementation of DirectoryEntry
 */

class util::ServerDirectory::Entry : public DirectoryEntry {
 public:
    Entry(const Ref<ServerDirectory>& container, const String_t& name, size_t index);

    virtual String_t getTitle();
    virtual String_t getPathName();
    virtual afl::base::Ref<Stream> openFile(FileSystem::OpenMode mode);
    virtual afl::base::Ref<Directory> openDirectory();
    virtual afl::base::Ref<Directory> openContainingDirectory();

 protected:
    virtual void updateInfo(uint32_t requested);
    virtual void doRename(String_t newName);
    virtual void doErase();
    virtual void doCreateAsDirectory();
    virtual void doSetFlag(FileFlag flag, bool value);

 private:
    Ref<ServerDirectory> m_container;
    String_t m_name;
    size_t m_index;

    afl::base::Ref<Stream> create(File* p, bool createNew);
    afl::base::Ref<Stream> open(File* p, bool forWriting);
};

inline
util::ServerDirectory::Entry::Entry(const Ref<ServerDirectory>& container, const String_t& name, size_t index)
    : m_container(container),
      m_name(name),
      m_index(index)
{ }

String_t
util::ServerDirectory::Entry::getTitle()
{
    return m_name;
}

String_t
util::ServerDirectory::Entry::getPathName()
{
    return String_t();
}

afl::base::Ref<Stream>
util::ServerDirectory::Entry::openFile(FileSystem::OpenMode mode)
{
    File* p = m_container->findEntry(m_name, m_index).first;
    switch (mode) {
     case FileSystem::OpenRead:
        return open(p, false);
     case FileSystem::OpenWrite:
        m_container->checkWritable(m_name);
        return open(p, true);
     case FileSystem::Create:
        m_container->checkWritable(m_name);
        return create(p, false);
     case FileSystem::CreateNew:
        m_container->checkWritable(m_name);
        return create(p, true);
    }

    // Not reached:
    throw FileProblemException(m_name, Messages::fileNotFound());
}

afl::base::Ref<Directory>
util::ServerDirectory::Entry::openDirectory()
{
    throw FileProblemException(m_name, Messages::cannotAccessDirectories());
}

afl::base::Ref<Directory>
util::ServerDirectory::Entry::openContainingDirectory()
{
    return m_container;
}

void
util::ServerDirectory::Entry::updateInfo(uint32_t /*requested*/)
{
    if (File* p = m_container->findEntry(m_name, m_index).first) {
        switch (p->state) {
         case File::NotFile:
            setFileType(tDirectory);
            break;

         case File::UnreadFile:
            setFileType(tFile);
            setFileSize(p->size);
            break;

         case File::CleanFile:
         case File::DirtyFile:
         case File::NewFile:
            setFileType(tFile);
            setFileSize(p->data->getSize());
            break;

         case File::DeletedFile:
         case File::GoneFile:
            // Not normally reached (can be reached if a file is deleted after the Entry is made).
            break;
        }
    }
}

void
util::ServerDirectory::Entry::doRename(String_t /*newName*/)
{
    throw FileProblemException(m_name, Messages::cannotWrite());
}

void
util::ServerDirectory::Entry::doErase()
{
    // Global writability check
    m_container->checkWritable(m_name);

    // State handling
    bool ok = false;
    if (File* p = m_container->findEntry(m_name, m_index).first) {
        switch (p->state) {
         case File::NotFile:
            break;

         case File::UnreadFile:
         case File::CleanFile:
         case File::DirtyFile:
            p->state = File::DeletedFile;
            p->data.reset();
            ok = true;
            break;

         case File::NewFile:
            p->state = File::GoneFile;
            p->data.reset();
            ok = true;
            break;

         case File::DeletedFile:
         case File::GoneFile:
            break;
        }
    }
    if (!ok) {
        throw FileProblemException(m_name, Messages::cannotWrite());
    }
}

void
util::ServerDirectory::Entry::doCreateAsDirectory()
{
    throw FileProblemException(m_name, Messages::cannotWrite());
}

void
util::ServerDirectory::Entry::doSetFlag(FileFlag /*flag*/, bool /*value*/)
{
    throw FileProblemException(m_name, Messages::cannotWrite());
}

afl::base::Ref<Stream>
util::ServerDirectory::Entry::create(File* p, bool createNew)
{
    if (p == 0) {
        if (!m_container->m_transport->isValidFileName(m_name)) {
            throw FileProblemException(m_name, Messages::invalidFileName());
        }
        std::pair<File*, size_t> result = m_container->createEntry(m_name);
        p = result.first;
        m_index = result.second;
    } else {
        switch (p->state) {
         case File::NotFile:
            // Directory: fail
            throw FileProblemException(m_name, Messages::fileExists());

         case File::UnreadFile:
         case File::CleanFile:
         case File::DirtyFile:
            // Existing file
            if (createNew) {
                throw FileProblemException(m_name, Messages::fileExists());
            }
            p->state = File::DirtyFile;
            break;

         case File::NewFile:
            // Newly-created file
            if (createNew) {
                throw FileProblemException(m_name, Messages::fileExists());
            }
            break;

         case File::DeletedFile:
            // Deleted and re-created
            p->state = File::DirtyFile;
            break;

         case File::GoneFile:
            // Created, deleted, and newly-created
            p->state = File::NewFile;
            break;
        }
    }

    p->data = new InternalStream();
    p->data->setName(m_name);
    return p->data->createChild();
}

afl::base::Ref<Stream>
util::ServerDirectory::Entry::open(File* p, bool forWriting)
{
    if (p == 0) {
        throw FileProblemException(m_name, Messages::fileNotFound());
    } else {
        switch (p->state) {
         case File::NotFile:
            // Directory: fail
            throw FileProblemException(m_name, Messages::fileExists());

         case File::UnreadFile: {
            // We can make that file readable
            // FIXME: this copies the data.
            afl::base::GrowableBytes_t data;
            m_container->m_transport->getFile(m_name, data);
            p->data = new InternalStream();
            p->data->write(data);
            p->data->setPos(0);
            p->state = (forWriting ? File::DirtyFile : File::CleanFile);
            break;
         }

         case File::CleanFile:
            // Existing file
            if (forWriting) {
                p->state = File::DirtyFile;
            }
            break;

         case File::DirtyFile:
         case File::NewFile:
            // Existing, already dirty file
            break;

         case File::DeletedFile:
         case File::GoneFile:
            // OK
            throw FileProblemException(m_name, Messages::fileNotFound());
        }
        return p->data->createChild();
    }
}


/*
 *  Enum - Implementation of Enumerator
 */
class util::ServerDirectory::Enum : public afl::base::Enumerator<Ptr<DirectoryEntry> > {
 public:
    Enum(Ref<ServerDirectory> container)
        : m_container(container), m_index(0)
        { }

    bool getNextElement(Ptr<DirectoryEntry>& result)
        {
            // Make sure content is loaded
            m_container->loadContent();

            // Skip over deleted entries
            while (m_index < m_container->m_files.size() && m_container->m_files[m_index]->isDeleted()) {
                ++m_index;
            }

            if (m_index < m_container->m_files.size()) {
                result = new Entry(m_container, m_container->m_files[m_index]->name, m_index);
                ++m_index;
                return true;
            } else {
                return false;
            }
        }

 private:
    Ref<ServerDirectory> m_container;
    size_t m_index;
};

/*
 *  ServerDirectory
 */

util::ServerDirectory::ServerDirectory(const afl::base::Ref<Transport>& transport, const String_t& title, const afl::base::Ptr<Directory>& parentDirectory)
    : m_transport(transport),
      m_title(title),
      m_parentDirectory(parentDirectory),
      m_files(),
      m_filesLoaded(false)
{ }

util::ServerDirectory::~ServerDirectory()
{ }

afl::base::Ref<util::ServerDirectory>
util::ServerDirectory::create(afl::base::Ref<Transport> transport, String_t title, afl::base::Ptr<Directory> parentDirectory)
{
    return *new ServerDirectory(transport, title, parentDirectory);
}

void
util::ServerDirectory::flush()
{
    // Flush everything.
    // If we get an exception, try finishing the operation; then rethrow.
    // flushEntry() is responsible for clearing the status before returning false.
    size_t i = 0;
    try {
        while (flushEntry(i))
            ;
    }
    catch (std::exception& e) {
        // Flush remainder
        try {
            while (flushEntry(i))
                ;
        }
        catch (...)
        { }

        // Rethrow original exception
        throw;
    }
}

afl::base::Ref<afl::io::DirectoryEntry>
util::ServerDirectory::getDirectoryEntryByName(String_t name)
{
    return *new Entry(*this, name, findEntry(name, 0).second);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<afl::io::DirectoryEntry> > >
util::ServerDirectory::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
util::ServerDirectory::getParentDirectory()
{
    return m_parentDirectory;
}

String_t
util::ServerDirectory::getDirectoryName()
{
    return String_t();
}

String_t
util::ServerDirectory::getTitle()
{
    return m_title;
}

void
util::ServerDirectory::checkWritable(const String_t& name)
{
    if (!m_transport->isWritable()) {
        throw FileProblemException(name, Messages::cannotWrite());
    }
}

void
util::ServerDirectory::loadContent()
{
    if (!m_filesLoaded) {
        m_filesLoaded = true;
        std::vector<FileInfo> result;
        m_transport->getContent(result);
        for (size_t i = 0, n = result.size(); i < n; ++i) {
            m_files.pushBackNew(new File(result[i].name, result[i].size, result[i].isFile ? File::UnreadFile : File::NotFile));
        }
    }
}

std::pair<util::ServerDirectory::File*, size_t>
util::ServerDirectory::findEntry(const String_t& name, size_t hint)
{
    loadContent();
    if (hint < m_files.size() && name == m_files[hint]->name) {
        return std::make_pair(m_files[hint], hint);
    }

    for (size_t i = 0, n = m_files.size(); i < n; ++i) {
        if (m_files[i]->name == name) {
            return std::make_pair(m_files[i], i);
        }
    }
    return std::make_pair((File*) 0, 0);
}

inline std::pair<util::ServerDirectory::File*, size_t>
util::ServerDirectory::createEntry(const String_t& name)
{
    size_t index = m_files.size();
    File* p = m_files.pushBackNew(new File(name, 0, File::NewFile));
    return std::make_pair(p, index);
}

bool
util::ServerDirectory::flushEntry(size_t& index)
{
    if (index < m_files.size()) {
        // We can do something
        File* p = m_files[index++];
        switch (p->state) {
         case File::NotFile:
         case File::UnreadFile:
         case File::CleanFile:
         case File::GoneFile:
            // Nothing to do
            break;

         case File::DirtyFile:
         case File::NewFile:
            // Must upload
            m_transport->putFile(p->name, p->data->getContent());
            break;

         case File::DeletedFile:
            // Must delete
            m_transport->eraseFile(p->name);
            break;
        }
        return true;
    } else {
        // Completed. Clear cache so next operation will reload.
        m_files.clear();
        m_filesLoaded = false;
        return false;
    }
}
