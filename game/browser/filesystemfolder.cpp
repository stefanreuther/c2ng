/**
  *  \file game/browser/filesystemfolder.cpp
  */

#include "game/browser/filesystemfolder.hpp"
#include "afl/base/staticassert.hpp"
#include "afl/bits/fixedstring.hpp"
#include "afl/bits/int16le.hpp"
#include "afl/bits/value.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"
#include "afl/string/string.hpp"
#include "game/browser/browser.hpp"

namespace {
    /*
     *  Definition of the gamestat.dat file
     */

    typedef afl::bits::Value<afl::bits::Int16LE> Int16_t;
    typedef afl::bits::Value<afl::bits::FixedString<80> > String80_t;
    typedef afl::bits::Value<afl::bits::FixedString<20> > String20_t;

    const int NUM_SLOTS = 8;

    struct GameStatEntry {
        Int16_t unused;
        Int16_t players[11];
        String80_t downloadPath;
        String80_t uploadPath;
        String20_t name;
        Int16_t unused2;
    };

    struct GameStatFile {
        GameStatEntry slots[NUM_SLOTS];
        char empty;
        Int16_t lastSlot;
    };

    static_assert(sizeof(GameStatEntry) == 206, "sizeof GameStatEntry");
    static_assert(sizeof(GameStatFile) == 1651, "sizeof GameStatFile");


    bool sortFolders(const game::browser::Folder& a, const game::browser::Folder& b)
    {
        return afl::string::strCaseCompare(a.getName(), b.getName()) < 0;
    }
}

game::browser::FileSystemFolder::FileSystemFolder(Browser& parent, afl::base::Ref<afl::io::Directory> dir, String_t title, bool ignoreIndex)
    : m_parent(parent),
      m_directory(dir),
      m_title(title),
      m_ignoreIndex(ignoreIndex)
{ }

void
game::browser::FileSystemFolder::loadContent(afl::container::PtrVector<Folder>& result)
{
    using afl::base::Ptr;
    using afl::base::Ref;
    using afl::base::Enumerator;
    using afl::io::Directory;
    using afl::io::DirectoryEntry;
    using afl::io::Stream;

    // Try to load gamestat file
    if (!m_ignoreIndex) {
        try {
            // Assume Western Windows character set
            afl::charset::CodepageCharset charset(afl::charset::g_codepage1252);

            // Read gamestat file. If file does not exist or cannot be read, it is ignored.
            GameStatFile index;
            m_directory->openFile("gamestat.dat", afl::io::FileSystem::OpenRead)
                ->fullRead(afl::base::fromObject(index));

            // Build content
            for (int i = 0; i < NUM_SLOTS; ++i) {
                result.pushBackNew(new FileSystemFolder(m_parent,
                                                        m_directory->openDirectory(afl::string::Format("vpwork%d", i+1)),
                                                        charset.decode(index.slots[i].name),
                                                        true));
            }
            result.pushBackNew(new FileSystemFolder(m_parent, m_directory, m_parent.translator()("[Directory content]"), true));
            return;
        }
        catch (...) { }
        result.clear();
    }

    // Enumerate directory
    Ref<Enumerator<Ptr<DirectoryEntry> > > content = m_directory->getDirectoryEntries();

    // Build list
    Ptr<DirectoryEntry> elem;
    while (content->getNextElement(elem)) {
        if (elem.get() != 0
            && (elem->getFileType() == DirectoryEntry::tDirectory || elem->getFileType() == DirectoryEntry::tRoot)
            && !elem->getFlags().contains(DirectoryEntry::Hidden))
        {
            result.pushBackNew(new FileSystemFolder(m_parent, elem->openDirectory(), elem->getTitle(), false));
        }
    }

    // Sort
    result.sort(sortFolders);
}

bool
game::browser::FileSystemFolder::loadConfiguration(game::config::UserConfiguration& config)
{
    config.loadGameConfiguration(*m_directory, m_parent.log(), m_parent.translator());
    return true;
}

void
game::browser::FileSystemFolder::saveConfiguration(const game::config::UserConfiguration& config)
{
    config.saveGameConfiguration(*m_directory, m_parent.log(), m_parent.translator());
}

bool
game::browser::FileSystemFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    return false;
}

afl::base::Ptr<game::Root>
game::browser::FileSystemFolder::loadGameRoot(const game::config::UserConfiguration& config)
{
    return m_parent.loadGameRoot(m_directory, config);
}

String_t
game::browser::FileSystemFolder::getName() const
{
    return m_title;
}

util::rich::Text
game::browser::FileSystemFolder::getDescription() const
{
    if (!m_directory->getDirectoryName().empty()) {
        return m_parent.translator()("File system folder");
    } else {
        return m_parent.translator()("Virtual folder");
    }
}

bool
game::browser::FileSystemFolder::isSame(const Folder& other) const
{
    const FileSystemFolder* p = dynamic_cast<const FileSystemFolder*>(&other);
    return p != 0
        && !m_directory->getDirectoryName().empty()
        && p->m_directory->getDirectoryName() == m_directory->getDirectoryName();
}

bool
game::browser::FileSystemFolder::canEnter() const
{
    return true;
}

game::browser::Folder::Kind
game::browser::FileSystemFolder::getKind() const
{
    return kFolder;
}
