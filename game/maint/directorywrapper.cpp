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
    Entry(DirectoryEntry& entry, DirectoryWrapper& parent)
        : m_parentEntry(entry),
          m_parentDirectory(parent)
        { }

    virtual String_t getTitle()
        { return m_parentEntry->getTitle(); }
    virtual String_t getPathName()
        { return m_parentEntry->getPathName(); }
    virtual afl::base::Ref<afl::io::Stream> openFile(FileSystem::OpenMode mode)
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
    virtual afl::base::Ref<Directory> openDirectory()
        { return m_parentEntry->openDirectory(); }
    virtual afl::base::Ref<Directory> openContainingDirectory()
        { return m_parentDirectory; }
    virtual void updateInfo(uint32_t requested)
        {
            // FIXME: move the copy-DirectoryEntry functionality into afl?
            if ((requested & InfoType) != 0) {
                setFileType(m_parentEntry->getFileType());
            }
            if ((requested & InfoSize) != 0) {
                setFileSize(m_parentEntry->getFileSize());
            }
            if ((requested & InfoLinkText) != 0) {
                setLinkText(m_parentEntry->getLinkText());
            }
            if ((requested & InfoModificationTime) != 0) {
                setModificationTime(m_parentEntry->getModificationTime());
            }
            if ((requested & InfoFlags) != 0) {
                setFlags(m_parentEntry->getFlags());
            }
        }
    virtual void doRename(String_t /*newName*/)
        {
            // We do not support rename. This is not required for our applications, so just refuse it.
            throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessFiles());
        }
    virtual void doErase()
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
    virtual void doCreateAsDirectory()
        { throw afl::except::FileProblemException(getPathName(), afl::string::Messages::cannotAccessDirectories()); }

 private:
    afl::base::Ref<DirectoryEntry> m_parentEntry;
    afl::base::Ref<DirectoryWrapper> m_parentDirectory;
};

class game::maint::DirectoryWrapper::Enum : public afl::base::Enumerator<afl::base::Ptr<DirectoryEntry> > {
 public:
    Enum(DirectoryWrapper& parent)
        : m_parentEnum(parent.getDirectoryEntries()),
          m_parentDirectory(parent)
        { }

    bool getNextElement(afl::base::Ptr<DirectoryEntry>& result)
        {
            afl::base::Ptr<DirectoryEntry> parentEntry;
            if (m_parentEnum->getNextElement(parentEntry) && parentEntry.get() != 0) {
                result = new Entry(*parentEntry, *m_parentDirectory);
                return true;
            } else {
                return false;
            }
        }

 private:
    afl::base::Ref<afl::base::Enumerator<afl::base::Ptr<DirectoryEntry> > > m_parentEnum;
    afl::base::Ref<DirectoryWrapper> m_parentDirectory;
};


/**************************** DirectoryWrapper ***************************/

afl::base::Ref<game::maint::DirectoryWrapper>
game::maint::DirectoryWrapper::create(afl::base::Ref<Directory> parent, afl::io::TextWriter& writer, afl::string::Translator& tx)
{
    return *new DirectoryWrapper(parent, writer, tx);
}

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
