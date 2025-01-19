/**
  *  \file game/maint/directorywrapper.cpp
  *
  *  The DirectoryWrapper class replaces PCC2's GGameDirManipulator, GSweepProcessor, GSweepRunProcessor, LoggingSweepProcessor hierarchy.
  *  This is probably not more efficient in terms of object code size, but reduces the number of abstractions to deal with.
  */

#include "game/maint/directorywrapper.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/string/messages.hpp"
#include "afl/io/nullstream.hpp"
#include "afl/io/stream.hpp"
#include "afl/string/format.hpp"

using afl::io::FileSystem;
using afl::io::DirectoryEntry;

class game::maint::DirectoryWrapper::Entry : public DirectoryEntry {
 public:
    Entry(DirectoryEntry& entry, DirectoryWrapper& parent);

    virtual String_t getTitle();
    virtual String_t getPathName();
    virtual afl::base::Ref<afl::io::Stream> openFile(FileSystem::OpenMode mode);
    virtual afl::base::Ref<Directory> openDirectory();
    virtual afl::base::Ref<Directory> openContainingDirectory();
    virtual void updateInfo(uint32_t requested);
    virtual void doRename(String_t newName);
    virtual void doErase();
    virtual void doCreateAsDirectory();
    virtual void doSetFlag(FileFlag flag, bool value);
    virtual void doMoveTo(Directory& dir, String_t name);

 private:
    afl::base::Ref<DirectoryEntry> m_parentEntry;
    afl::base::Ref<DirectoryWrapper> m_parentDirectory;
};

class game::maint::DirectoryWrapper::Enum : public afl::base::Enumerator<afl::base::Ptr<DirectoryEntry> > {
 public:
    Enum(DirectoryWrapper& parent);

    virtual bool getNextElement(afl::base::Ptr<DirectoryEntry>& result);

 private:
    afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<DirectoryEntry> > > m_parentEnum;
    afl::base::Ref<DirectoryWrapper> m_parentDirectory;
};

/************************ DirectoryWrapper::Entry ************************/

inline
game::maint::DirectoryWrapper::Entry::Entry(DirectoryEntry& entry, DirectoryWrapper& parent)
    : m_parentEntry(entry),
      m_parentDirectory(parent)
{ }

String_t
game::maint::DirectoryWrapper::Entry::getTitle()
{
    return m_parentEntry->getTitle();
}
String_t
game::maint::DirectoryWrapper::Entry::getPathName()
{
    return m_parentEntry->getPathName();
}

afl::base::Ref<afl::io::Stream>
game::maint::DirectoryWrapper::Entry::openFile(FileSystem::OpenMode mode)
{
    if (mode == FileSystem::OpenRead || m_parentDirectory->m_writeMode == PassThroughWrites) {
        // Open for read always succeeds. Pass-through also succeeds.
        return m_parentEntry->openFile(mode);
    } else {
        // Open for write in non-passthrough mode produces a NullStream.
        // That is good enough an emulation for our purposes.
        return *new afl::io::NullStream();
    }
}

afl::base::Ref<afl::io::Directory>
game::maint::DirectoryWrapper::Entry::openDirectory()
{
    return m_parentEntry->openDirectory();
}

afl::base::Ref<afl::io::Directory>
game::maint::DirectoryWrapper::Entry::openContainingDirectory()
{
    return m_parentDirectory;
}

void
game::maint::DirectoryWrapper::Entry::updateInfo(uint32_t requested)
{
    copyInfo(*m_parentEntry, requested);
}

void
game::maint::DirectoryWrapper::Entry::doRename(String_t /*newName*/)
{
    // We do not support rename. This is not required for our applications, so just refuse it.
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessFiles());
}

void
game::maint::DirectoryWrapper::Entry::doErase()
{
    // ex GGameDirManipulator::removeGameFile, sort-of. Umm.
    switch (m_parentDirectory->m_eraseMode) {
     case PassThroughErase:
        // Pass-through: just pass through.
        m_parentEntry->erase();
        break;

     case LogErase:
        // Log: just pass through. If that didn't throw, log.
        m_parentEntry->erase();
        m_parentDirectory->m_writer.writeLine(afl::string::Format(m_parentDirectory->m_translator.translateString("Erased file %s.").c_str(), getTitle()));
        break;

     case IgnoreAndLogErase:
        // Ignore: probe file existence. If it exists, log; otherwise throw an exception.
        // The exception typically is annihilated by eraseNT(), so it's not too important what it actually is;
        // the original exception from openFile() is probably fine.
        m_parentEntry->openFile(FileSystem::OpenRead);
        m_parentDirectory->m_writer.writeLine(getTitle());
        break;
    }

    // Count it. We're here only if it succeeded.
    ++m_parentDirectory->m_numRemovedFiles;
}

void
game::maint::DirectoryWrapper::Entry::doCreateAsDirectory()
{
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessDirectories());
}

void
game::maint::DirectoryWrapper::Entry::doSetFlag(FileFlag /*flag*/, bool /*value*/)
{
    // We do not support changing. This is not required for our applications, so just refuse it.
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessFiles());
}

void
game::maint::DirectoryWrapper::Entry::doMoveTo(Directory& /*dir*/, String_t /*name*/)
{
    // We do not support move. This is not required for our applications, so just refuse it.
    throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessFiles());
}

/************************* DirectoryWrapper::Enum ************************/

inline
game::maint::DirectoryWrapper::Enum::Enum(DirectoryWrapper& parent)
    : m_parentEnum(parent.getDirectoryEntries()),
      m_parentDirectory(parent)
{ }

bool
game::maint::DirectoryWrapper::Enum::getNextElement(afl::base::Ptr<DirectoryEntry>& result)
{
    afl::base::Ptr<DirectoryEntry> parentEntry;
    if (m_parentEnum->getNextElement(parentEntry) && parentEntry.get() != 0) {
        result = new Entry(*parentEntry, *m_parentDirectory);
        return true;
    } else {
        return false;
    }
}

/**************************** DirectoryWrapper ***************************/

afl::base::Ref<game::maint::DirectoryWrapper>
game::maint::DirectoryWrapper::create(afl::base::Ref<Directory> parent, afl::io::TextWriter& writer, afl::string::Translator& tx)
{
    return *new DirectoryWrapper(parent, writer, tx);
}

inline
game::maint::DirectoryWrapper::DirectoryWrapper(afl::base::Ref<Directory> parent, afl::io::TextWriter& writer, afl::string::Translator& tx)
    : m_parent(parent),
      m_writer(writer),
      m_translator(tx),
      m_eraseMode(PassThroughErase),
      m_writeMode(PassThroughWrites),
      m_numRemovedFiles(0)
{ }

game::maint::DirectoryWrapper::~DirectoryWrapper()
{ }

afl::base::Ref<DirectoryEntry>
game::maint::DirectoryWrapper::getDirectoryEntryByName(String_t name)
{
    return *new Entry(*m_parent->getDirectoryEntryByName(name), *this);
}

afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<DirectoryEntry> > >
game::maint::DirectoryWrapper::getDirectoryEntries()
{
    return *new Enum(*this);
}

afl::base::Ptr<afl::io::Directory>
game::maint::DirectoryWrapper::getParentDirectory()
{
    return 0;
}

String_t
game::maint::DirectoryWrapper::getDirectoryName()
{
    return m_parent->getDirectoryName();
}

String_t
game::maint::DirectoryWrapper::getTitle()
{
    return m_parent->getTitle();
}

void
game::maint::DirectoryWrapper::setWriteMode(WriteMode mode)
{
    m_writeMode = mode;
}

void
game::maint::DirectoryWrapper::setEraseMode(EraseMode mode)
{
    // ex GGameDirManipulator::setVerbose, sort-of
    m_eraseMode = mode;
}

int
game::maint::DirectoryWrapper::getNumRemovedFiles() const
{
    return m_numRemovedFiles;
}
