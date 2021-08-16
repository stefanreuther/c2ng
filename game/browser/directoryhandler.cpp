/**
  *  \file game/browser/directoryhandler.cpp
  */

#include "game/browser/directoryhandler.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/filesystemrootfolder.hpp"
#include "game/browser/filesystemfolder.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "util/charsetfactory.hpp"

namespace {
    // FIXME: duplicate to browser.cpp
    const char LOG_NAME[] = "game.browser";
}

game::browser::DirectoryHandler::DirectoryHandler(Browser& b, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory, util::ProfileDirectory& profile, afl::io::FileSystem& fs)
    : m_browser(b),
      m_v3Loader(defaultSpecificationDirectory, &profile, b.translator(), b.log(), fs)
{ }

bool
game::browser::DirectoryHandler::handleFolderName(String_t name, afl::container::PtrVector<Folder>& result)
{
    // Is this actually a local folder?
    try {
        afl::base::Ref<afl::io::Directory> dir = m_browser.fileSystem().openDirectory(name);
        dir->getDirectoryEntries();
    }
    catch (...) {
        return false;
    }

    afl::container::PtrVector<Folder> folders;
    try {
        // Get list of roots
        afl::container::PtrVector<Folder> roots;
        FileSystemRootFolder(m_browser).loadContent(roots);

        // Process the provided folder
        afl::base::Ptr<afl::io::Directory> dir = m_browser.fileSystem().openDirectory(m_browser.fileSystem().getAbsolutePathName(name)).asPtr();
        while (dir.get() != 0) {
            // Create this folder.
            // This produces "Winplan > vpwork3" and "Winplan > bmp" instead of "Winplan > Game 3" and "Winplan > [Directory Contents] > bmp", respectively.
            // Producing the latter would require some backtracking (i.e. pass false, look whether we find it, etc.),
            // which I don't consider worthwhile for now.
            std::auto_ptr<FileSystemFolder> f(new FileSystemFolder(m_browser, *dir, dir->getTitle(), true));

            // Match against roots.
            // We prefer using a root because that has the nicer title than the implicitly created parent.
            // In addition, this makes "/home/user/foo" open as "My Computer > Home Directory > foo"
            // instead of "My Computer > Root Directory > home > user > foo".
            bool found = false;
            for (size_t i = 0, n = roots.size(); i != n; ++i) {
                if (roots[i]->isSame(*f)) {
                    folders.pushBackNew(roots.extractElement(i));
                    found = true;
                    break;
                }
            }
            if (found) {
                break;
            }

            // Not a root, so go up and continue.
            folders.pushBackNew(f.release());
            dir = dir->getParentDirectory();
        }
    }
    catch (std::exception& e) {
        m_browser.log().write(afl::sys::LogListener::Warn, LOG_NAME, String_t(), e);
    }

    // Build result
    result.pushBackNew(new FileSystemRootFolder(m_browser));
    while (!folders.empty()) {
        result.pushBackNew(folders.extractLast());
    }
    return true;
}

game::browser::Folder*
game::browser::DirectoryHandler::createAccountFolder(Account& /*acc*/)
{
    // No, we cannot handle accounts
    return 0;
}

afl::base::Ptr<game::Root>
game::browser::DirectoryHandler::loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config)
{
    String_t gameType = config.getGameType();
    if (gameType.empty() || gameType == "local") {
        std::auto_ptr<afl::charset::Charset> cs(util::CharsetFactory().createCharset(config[config.Game_Charset]()));
        if (cs.get() == 0) {
            cs.reset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
        }
        return m_v3Loader.load(dir, *cs, config, false);
    } else {
        return 0;
    }
}
