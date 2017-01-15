/**
  *  \file util/profiledirectory.cpp
  */

#include "util/profiledirectory.hpp"
#include "afl/io/directory.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"

namespace {
    /** Try to create a path. Creates a complete path that can contain multiple non-existant
        directory levels. This does not fail when the path cannot be created; in that case,
        subsequent operations using the path will fail.
        \param dirName Name of path to create */
    void tryCreatePath(afl::io::FileSystem& fs, const String_t dirName)
    {
        // ex game/backup.cc:tryCreatePath
        // FIXME: move this function to a more sensible place. Even afl maybe?
        const String_t parentName = fs.getDirectoryName(dirName);
        const String_t childName  = fs.getFileName(dirName);

        // If parentName is the same as dirName, this means that dirName does not have a parent.
        // In this case, we don't do anything.
        if (parentName != dirName) {
            // Try enumerating the parent's content. If that fails, try to create it.
            // (openDir alone does not check whether the directory actually exists.)
            try {
                afl::base::Ref<afl::io::Directory> parent = fs.openDirectory(parentName);
                parent->getDirectoryEntries();
            }
            catch (afl::except::FileProblemException&) {
                tryCreatePath(fs, parentName);
            }

            // Parent should now exist. Try creating child in it unless it already exists.
            try {
                afl::base::Ref<afl::io::Directory> parent = fs.openDirectory(parentName);
                afl::base::Ref<afl::io::DirectoryEntry> entry = parent->getDirectoryEntryByName(childName);
                if (entry->getFileType() != afl::io::DirectoryEntry::tDirectory) {
                    entry->createAsDirectory();
                }
            }
            catch (afl::except::FileProblemException&) { }
        }
    }
}

util::ProfileDirectory::ProfileDirectory(afl::sys::Environment& env,
                                         afl::io::FileSystem& fileSystem,
                                         afl::string::Translator& tx, afl::sys::LogListener& log)
    : m_name(env.getSettingsDirectoryName("PCC2")),
      m_fileSystem(fileSystem),
      m_translator(tx),
      m_log(log)
{ }

afl::base::Ptr<afl::io::Stream>
util::ProfileDirectory::openFileNT(String_t name)
{
    try {
        afl::base::Ref<afl::io::Directory> parent = m_fileSystem.openDirectory(m_name);
        return parent->openFile(name, afl::io::FileSystem::OpenRead).asPtr();
    }
    catch (afl::except::FileProblemException&) {
        return 0;
    }
}

afl::base::Ref<afl::io::Stream>
util::ProfileDirectory::createFile(String_t name)
{
    return open()->openFile(name, afl::io::FileSystem::Create);
}

afl::base::Ref<afl::io::Directory>
util::ProfileDirectory::open()
{
    tryCreatePath(m_fileSystem, m_name);
    return m_fileSystem.openDirectory(m_name);
}
