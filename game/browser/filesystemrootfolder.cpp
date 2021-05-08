/**
  *  \file game/browser/filesystemrootfolder.cpp
  */

#include "game/browser/filesystemrootfolder.hpp"
#include "afl/io/filesystem.hpp"
#include "afl/io/directory.hpp"
#include "afl/io/directoryentry.hpp"
#include "game/browser/filesystemfolder.hpp"
#include "afl/string/translator.hpp"
#include "game/browser/browser.hpp"

game::browser::FileSystemRootFolder::FileSystemRootFolder(Browser& parent)
    : m_parent(parent)
{ }

void
game::browser::FileSystemRootFolder::loadContent(afl::container::PtrVector<Folder>& result)
{
    using afl::base::Ptr;
    using afl::base::Ref;
    using afl::base::Enumerator;
    using afl::io::FileSystem;
    using afl::io::Directory;
    using afl::io::DirectoryEntry;

    // Open root list
    Ref<Directory> dir = m_parent.fileSystem().openRootDirectory();

    // Enumerate root
    Ref<Enumerator<Ptr<DirectoryEntry> > > content = dir->getDirectoryEntries();

    // Build list
    Ptr<DirectoryEntry> elem;
    while (content->getNextElement(elem)) {
        if (elem.get() != 0 && (elem->getFileType() == DirectoryEntry::tDirectory || elem->getFileType() == DirectoryEntry::tRoot)) {
            result.pushBackNew(new FileSystemFolder(m_parent, elem->openDirectory(), elem->getTitle(), false));
        }
    }
}

bool
game::browser::FileSystemRootFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    // Root has no physical location, so we cannot load a configuration here
    return false;
}

void
game::browser::FileSystemRootFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{ }

bool
game::browser::FileSystemRootFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    return false;
}

afl::base::Ptr<game::Root>
game::browser::FileSystemRootFolder::loadGameRoot(const game::config::UserConfiguration& /*config*/)
{
    // No games in file system root
    return 0;
}

String_t
game::browser::FileSystemRootFolder::getName() const
{
    return m_parent.translator().translateString("My Computer");
}

util::rich::Text
game::browser::FileSystemRootFolder::getDescription() const
{
    return m_parent.translator().translateString("Browse folders on this computer");
}

bool
game::browser::FileSystemRootFolder::isSame(const Folder& other) const
{
    return dynamic_cast<const FileSystemRootFolder*>(&other) != 0;
}

bool
game::browser::FileSystemRootFolder::canEnter() const
{
    return true;
}

game::browser::Folder::Kind
game::browser::FileSystemRootFolder::getKind() const
{
    return kLocal;
}
