/**
  *  \file game/browser/browser.cpp
  *  \brief Class game::browser::Browser
  */

#include "game/browser/browser.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/char.hpp"
#include "afl/string/format.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/handler.hpp"
#include "game/browser/unsupportedaccountfolder.hpp"
#include "util/string.hpp"

using afl::string::Format;
using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "game.browser";

    const char* GAMES_DIR_NAME = "games";
    const char* GAME_PREFIX = "game:";

    String_t stripDecorations(String_t gameName)
    {
        String_t::size_type n = gameName.find('/');
        if (n != String_t::npos) {
            gameName.erase(0, n);
        }
        n = gameName.find_first_of("[(#");
        if (n != String_t::npos) {
            gameName.erase(n);
        }
        return gameName;
    }

    String_t simplifyFileName(const String_t name)
    {
        String_t result;
        bool hadSpace = true;
        for (size_t i = 0; i < name.size(); ++i) {
            if (afl::string::charIsAlphanumeric(name[i])) {
                result += afl::string::charToLower(name[i]);
                hadSpace = false;
            } else if (name[i] == '\'' || name[i] == '"') {
                // ignore
            } else {
                if (!hadSpace) {
                    result += '_';
                    hadSpace = true;
                }
            }
        }
        if (hadSpace && !result.empty()) {
            result.erase(result.size()-1);
        }
        return result;
    }
}

game::browser::Browser::Browser(afl::io::FileSystem& fileSystem,
                                afl::string::Translator& tx,
                                afl::sys::LogListener& log,
                                AccountManager& accounts,
                                util::ProfileDirectory& profile,
                                UserCallback& callback)
    : m_fileSystem(fileSystem),
      m_translator(tx),
      m_log(log),
      m_accounts(accounts),
      m_profile(profile),
      m_callback(callback),
      m_handlers(),
      m_path(),
      m_pathOrigin(),
      m_content(),
      m_rootFolder(*this),
      m_selectedChild(),
      m_childLoaded(false),
      m_childRoot(),
      m_childConfig()
{ }

game::browser::Browser::~Browser()
{ }

afl::io::FileSystem&
game::browser::Browser::fileSystem()
{
    return m_fileSystem;
}

afl::string::Translator&
game::browser::Browser::translator()
{
    return m_translator;
}

afl::sys::LogListener&
game::browser::Browser::log()
{
    return m_log;
}

game::browser::AccountManager&
game::browser::Browser::accounts()
{
    return m_accounts;
}

game::browser::UserCallback&
game::browser::Browser::callback()
{
    return m_callback;
}

util::ProfileDirectory&
game::browser::Browser::profile()
{
    return m_profile;
}

void
game::browser::Browser::addNewHandler(Handler* h)
{
    m_handlers.addNewHandler(h);
}

bool
game::browser::Browser::openFolder(String_t name)
{
    afl::container::PtrVector<Folder> result;
    if (m_handlers.handleFolderName(name, result)) {
        m_log.write(LogListener::Trace, LOG_NAME, Format("Browser.openFolder('%s') ok", name));
        m_path.swap(result);
        m_pathOrigin.reset();
        clearContent();
        return true;
    } else {
        m_log.write(LogListener::Trace, LOG_NAME, Format("Browser.openFolder('%s') failed", name));
        return false;
    }
}

void
game::browser::Browser::openChild(size_t n)
{
    if (n < m_content.size()) {
        m_log.write(LogListener::Trace, LOG_NAME, Format("Browser.openChild(%d)", n));
        m_path.pushBackNew(m_content.extractElement(n));
        m_pathOrigin.reset();
        clearContent();
    }
}

void
game::browser::Browser::openParent()
{
    if (!m_path.empty()) {
        m_log.write(LogListener::Trace, LOG_NAME, "Browser.openParent");
        m_pathOrigin.reset(m_path.extractLast());
        clearContent();
    }
}

void
game::browser::Browser::selectChild(size_t n)
{
    if (!m_selectedChild.isSame(n)) {
        m_selectedChild = n;
        m_childLoaded = false;
        m_childRoot.reset();
        m_childConfig.reset();
    }
}

game::browser::Folder&
game::browser::Browser::currentFolder()
{
    if (!m_path.empty()) {
        return *m_path.back();
    } else {
        return m_rootFolder;
    }
}

const afl::container::PtrVector<game::browser::Folder>&
game::browser::Browser::path()
{
    return m_path;
}

const afl::container::PtrVector<game::browser::Folder>&
game::browser::Browser::content()
{
    return m_content;
}

void
game::browser::Browser::clearContent()
{
    m_content.clear();
    m_selectedChild = afl::base::Nothing;
    m_childLoaded = false;
    m_childRoot.reset();
    m_childConfig.reset();
}

game::browser::Folder*
game::browser::Browser::getSelectedChild() const
{
    size_t pos;
    if (m_selectedChild.get(pos) && pos < m_content.size()) {
        return m_content[pos];
    } else {
        return 0;
    }
}

game::browser::Browser::OptionalIndex_t
game::browser::Browser::getSelectedChildIndex() const
{
    return m_selectedChild;
}

afl::base::Ptr<game::Root>
game::browser::Browser::getSelectedRoot() const
{
    return m_childRoot;
}

game::config::UserConfiguration*
game::browser::Browser::getSelectedConfiguration() const
{
    return m_childConfig.get();
}

bool
game::browser::Browser::isSelectedFolderSetupSuggested() const
{
    /* This function is used to suggest configuring a local folder.
       We suggest that if
       - the game reports aLocalSetup (=a local folder can be configured)
       - the game is not aLoadEditable (=local folder may make it editable)
       - read-onlyness cannot be configured (=game cannot become editable by configuring away read-onlyness) */
    using game::Root;
    bool result = false;
    if (m_childRoot.get() != 0 && m_childConfig.get() != 0) {
        const Root::Actions_t as = m_childRoot->getPossibleActions();
        if (as.contains(Root::aLocalSetup) && !as.contains(Root::aLoadEditable)) {
            if (!as.contains(Root::aConfigureReadOnly) || !(*m_childConfig)[game::config::UserConfiguration::Game_ReadOnly]()) {
                result = true;
            }
        }
    }
    return result;
}

std::auto_ptr<game::Task_t>
game::browser::Browser::loadContent(std::auto_ptr<Task_t> then)
{
    // Task to receive result and replace content with newly-loaded values
    class Then : public LoadContentTask_t {
     public:
        Then(Browser& browser, std::auto_ptr<Task_t> then)
            : m_browser(browser), m_then(then)
            { }
        virtual void call(afl::container::PtrVector<Folder>& result)
            {
                m_browser.m_log.write(LogListener::Trace, LOG_NAME, "Task: Browser.loadContent.Then");

                // If we have a previous path element, attempt to locate and select that
                m_browser.m_content.swap(result);
                if (m_browser.m_pathOrigin.get() != 0) {
                    for (size_t i = 0, n = m_browser.m_content.size(); i < n; ++i) {
                        if (m_browser.m_content[i]->isSame(*m_browser.m_pathOrigin)) {
                            m_browser.selectChild(i);
                            break;
                        }
                    }
                    m_browser.m_pathOrigin.reset();
                }

                m_then->call();
            }
     private:
        Browser& m_browser;
        std::auto_ptr<Task_t> m_then;
    };

    // Main task (to allow creation of task ahead of time)
    class Task : public Task_t {
     public:
        Task(Browser& parent, std::auto_ptr<Task_t>& then)
            : m_parent(parent), m_then(then)
            { }
        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: Browser.loadContent");

                // If we have a selected element, but not a previous path element, select that element.
                // This preserves the cursor when we reload a directory.
                size_t n;
                if (m_parent.m_pathOrigin.get() == 0 && m_parent.m_selectedChild.get(n) && n < m_parent.m_content.size()) {
                    m_parent.m_pathOrigin.reset(m_parent.m_content.extractElement(n));
                }

                // Start task
                m_parent.clearContent();
                m_then = m_parent.currentFolder().loadContent(std::auto_ptr<LoadContentTask_t>(new Then(m_parent, m_then)));
                m_then->call();
            }
     private:
        Browser& m_parent;
        std::auto_ptr<Task_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, then));
}

std::auto_ptr<game::Task_t>
game::browser::Browser::loadChildRoot(std::auto_ptr<Task_t> then)
{
    // Separate task to allow sequential execution
    class Task : public Task_t {
     public:
        Task(Browser& parent, std::auto_ptr<Task_t> then)
            : m_parent(parent), m_then(then)
            { }
        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: Browser.loadChildRoot");

                size_t n;
                if (!m_parent.m_childLoaded && m_parent.m_selectedChild.get(n) && n < m_parent.m_content.size()) {
                    m_parent.m_childLoaded = true;
                    m_parent.m_childConfig = game::config::UserConfiguration::create().asPtr();

                    // Load configuration
                    try {
                        m_parent.m_content[n]->loadConfiguration(*m_parent.m_childConfig);
                    }
                    catch (std::exception& e) {
                        m_parent.m_log.write(LogListener::Warn, LOG_NAME, String_t(), e);
                    }

                    // Load root
                    m_then = m_parent.loadGameRoot(n, m_then);
                }
                m_then->call();
            }
     private:
        Browser& m_parent;
        std::auto_ptr<Task_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, then));
}

std::auto_ptr<game::Task_t>
game::browser::Browser::updateConfiguration(std::auto_ptr<Task_t> then)
{
    // Separate task to allow sequential execution
    class Task : public Task_t {
     public:
        Task(Browser& parent, std::auto_ptr<Task_t> then)
            : m_parent(parent), m_then(then)
            { }
        virtual void call()
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Browser.updateConfiguration");

                // Update configuration after user modified it:
                // - save to disk
                // - reload the root to let the new configuration take effect
                size_t n;
                if (m_parent.m_childLoaded && m_parent.m_selectedChild.get(n) && n < m_parent.m_content.size() && m_parent.m_childConfig.get() != 0) {
                    // Save configuration
                    try {
                        m_parent.m_content[n]->saveConfiguration(*m_parent.m_childConfig);
                    }
                    catch (std::exception& e) {
                        m_parent.m_log.write(LogListener::Warn, LOG_NAME, String_t(), e);
                    }

                    // Load root
                    m_then = m_parent.loadGameRoot(n, m_then);
                }
                m_then->call();
            }
     private:
        Browser& m_parent;
        std::auto_ptr<Task_t> m_then;
    };
    return std::auto_ptr<Task_t>(new Task(*this, then));
}

std::auto_ptr<game::Task_t>
game::browser::Browser::loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config, std::auto_ptr<LoadGameRootTask_t>& then)
{
    std::auto_ptr<Task_t> t = m_handlers.loadGameRootMaybe(dir, config, then);
    if (t.get() == 0) {
        t = Folder::defaultLoadGameRoot(then);
    }
    return t;
}

game::browser::Folder*
game::browser::Browser::createAccountFolder(const afl::base::Ref<Account>& account)
{
    Folder* result = m_handlers.createAccountFolder(account);
    if (result == 0) {
        result = new UnsupportedAccountFolder(m_translator, account);
    }
    return result;
}

String_t
game::browser::Browser::expandGameDirectoryName(String_t directoryName) const
{
    if (const char* suffix = util::strStartsWith(directoryName, GAME_PREFIX)) {
        return m_fileSystem.makePathName(m_fileSystem.makePathName(m_profile.open()->getDirectoryName(), GAMES_DIR_NAME), suffix);
    } else {
        return directoryName;
    }
}

void
game::browser::Browser::setSelectedLocalDirectoryName(String_t directoryName)
{
    size_t n;
    if (m_selectedChild.get(n) && n < m_content.size()) {
        m_content[n]->setLocalDirectoryName(directoryName);
    }
}

void
game::browser::Browser::setSelectedLocalDirectoryAutomatically()
{
    size_t n;
    if (m_selectedChild.get(n) && n < m_content.size()) {
        // Profile
        afl::base::Ref<afl::io::Directory> profileDirectory = m_profile.open();

        // "games" folder
        afl::base::Ref<afl::io::DirectoryEntry> gamesEntry = profileDirectory->getDirectoryEntryByName(GAMES_DIR_NAME);
        if (gamesEntry->getFileType() != afl::io::DirectoryEntry::tDirectory) {
            gamesEntry->createAsDirectory();
        }
        afl::base::Ref<afl::io::Directory> gamesDir = gamesEntry->openDirectory();

        // Assuming game name has form "a b (c)" or "zz/a b"...
        String_t gameName = m_content[n]->getName();

        // ...try "a_b"
        if (trySetLocalDirectoryName(*gamesDir, simplifyFileName(stripDecorations(gameName)))) {
            return;
        }

        // ...try "zz_a_b_c"
        if (trySetLocalDirectoryName(*gamesDir, simplifyFileName(gameName))) {
            return;
        }

        // ...try "zz_a_b_c" with numeric index
        for (int i = 1; i <= 10000; ++i) {
            if (trySetLocalDirectoryName(*gamesDir, simplifyFileName(Format("%s %d", gameName, i)))) {
                return;
            }
        }
    }
}

game::browser::Browser::DirectoryStatus
game::browser::Browser::verifyLocalDirectory(const String_t directoryName)
{
    try {
        afl::base::Ref<afl::io::Directory> dir = m_fileSystem.openDirectory(directoryName);

        // Try creating files
        bool ok = false;
        for (int i = 0; i < 1000; ++i) {
            String_t fileName = Format("_%d.tmp", i);
            if (dir->openFileNT(fileName, afl::io::FileSystem::CreateNew).get() != 0) {
                dir->eraseNT(fileName);
                ok = true;
                break;
            }
        }
        if (!ok) {
            return NotWritable;
        }

        // Check content
        afl::base::Ptr<afl::io::DirectoryEntry> e;
        if (dir->getDirectoryEntries()->getNextElement(e)) {
            return NotEmpty;
        }

        // Success
        return Success;
    }
    catch (...) {
        return Missing;
    }
}

bool
game::browser::Browser::trySetLocalDirectoryName(afl::io::Directory& gamesDir, String_t directoryName)
{
    if (directoryName.empty()) {
        return false;
    } else {
        try {
            afl::base::Ref<afl::io::DirectoryEntry> child = gamesDir.getDirectoryEntryByName(directoryName);
            if (child->getFileType() != afl::io::DirectoryEntry::tUnknown) {
                return false;
            } else {
                child->createAsDirectory();
                m_log.write(LogListener::Info, LOG_NAME, Format(m_translator("Using directory \"%s\""), directoryName));
                setSelectedLocalDirectoryName(GAME_PREFIX + directoryName);
                return true;
            }
        }
        catch (std::exception&) {
            return false;
        }
    }
}

std::auto_ptr<game::Task_t>
game::browser::Browser::loadGameRoot(size_t n, std::auto_ptr<Task_t>& then)
{
    class Then : public LoadGameRootTask_t {
     public:
        Then(Browser& parent, std::auto_ptr<Task_t>& then)
            : m_parent(parent), m_then(then)
            { }
        virtual void call(afl::base::Ptr<Root> root)
            {
                m_parent.m_log.write(LogListener::Trace, LOG_NAME, "Task: Browser.loadGameRoot.Then");
                m_parent.m_childRoot = root;
                m_then->call();
            }
     private:
        Browser& m_parent;
        std::auto_ptr<Task_t> m_then;
    };
    return m_content[n]->loadGameRoot(*m_childConfig, std::auto_ptr<LoadGameRootTask_t>(new Then(*this, then)));
}
