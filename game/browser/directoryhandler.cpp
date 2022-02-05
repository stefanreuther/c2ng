/**
  *  \file game/browser/directoryhandler.cpp
  *  \brief Class game::browser::DirectoryHandler
  */

#include "game/browser/directoryhandler.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/filesystemfolder.hpp"
#include "game/browser/filesystemrootfolder.hpp"
#include "game/browser/folder.hpp"
#include "util/charsetfactory.hpp"

using afl::sys::LogListener;
using afl::base::Ref;
using afl::base::Ptr;
using afl::io::Directory;
using afl::container::PtrVector;

namespace {
    const char*const LOG_NAME = "game.browser";
}

// Constructor.
game::browser::DirectoryHandler::DirectoryHandler(Browser& b, afl::base::Ref<afl::io::Directory> defaultSpecificationDirectory, util::ProfileDirectory& profile)
    : m_browser(b),
      m_v3Loader(defaultSpecificationDirectory, &profile, b.translator(), b.log(), b.fileSystem())
{ }

// Handle folder name or URL.
bool
game::browser::DirectoryHandler::handleFolderName(String_t name, afl::container::PtrVector<Folder>& result)
{
    // Is this actually a local folder?
    try {
        Ref<Directory> dir = m_browser.fileSystem().openDirectory(name);
        dir->getDirectoryEntries();
    }
    catch (...) {
        return false;
    }

    PtrVector<Folder> folders;
    try {
        // Get list of roots
        PtrVector<Folder> roots;
        FileSystemRootFolder(m_browser).loadContent(roots);

        // Process the provided folder
        Ptr<Directory> dir = m_browser.fileSystem().openDirectory(m_browser.fileSystem().getAbsolutePathName(name)).asPtr();
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
        m_browser.log().write(LogListener::Warn, LOG_NAME, String_t(), e);
    }

    // Build result
    result.pushBackNew(new FileSystemRootFolder(m_browser));
    while (!folders.empty()) {
        result.pushBackNew(folders.extractLast());
    }
    return true;
}

// Create account folder.
game::browser::Folder*
game::browser::DirectoryHandler::createAccountFolder(Account& /*acc*/)
{
    // No, we cannot handle accounts
    return 0;
}

// Load game root for physical folder.
std::auto_ptr<game::Task_t>
game::browser::DirectoryHandler::loadGameRootMaybe(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then)
{
    const String_t gameType = config.getGameType();
    if (gameType.empty() || gameType == "local") {
        class Task : public Task_t {
         public:
            Task(DirectoryHandler& parent, Ref<Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then)
                : m_parent(parent), m_dir(dir), m_config(config), m_then(then)
                { }
            virtual void call()
                {
                    Ptr<Root> result;
                    try {
                        m_parent.m_browser.log().write(LogListener::Trace, LOG_NAME, "Task: DirectoryHandler.loadGameRootMaybe");
                        std::auto_ptr<afl::charset::Charset> cs(util::CharsetFactory().createCharset(m_config[m_config.Game_Charset]()));
                        if (cs.get() == 0) {
                            cs.reset(new afl::charset::CodepageCharset(afl::charset::g_codepageLatin1));
                        }
                        result = m_parent.m_v3Loader.load(m_dir, *cs, m_config, false);
                    }
                    catch (std::exception& e) {
                        m_parent.m_browser.log().write(LogListener::Warn, LOG_NAME, String_t(), e);
                    }
                    m_then->call(result);
                }
         private:
            DirectoryHandler& m_parent;
            const Ref<Directory> m_dir;
            const game::config::UserConfiguration& m_config;
            const std::auto_ptr<LoadGameRootTask_t> m_then;
        };
        return std::auto_ptr<Task_t>(new Task(*this, dir, config, then));
    } else {
        return std::auto_ptr<Task_t>();
    }
}
