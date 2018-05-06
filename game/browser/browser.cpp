/**
  *  \file game/browser/browser.cpp
  */

#include <cstring>
#include "game/browser/browser.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/handler.hpp"
#include "game/browser/unsupportedaccountfolder.hpp"
#include "afl/string/char.hpp"
#include "afl/io/directoryentry.hpp"
#include "afl/string/format.hpp"

namespace {
    const char LOG_NAME[] = "game.browser";

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

game::browser::HandlerList&
game::browser::Browser::handlers()
{
    return m_handlers;
}

void
game::browser::Browser::openFolder(String_t name)
{
    afl::container::PtrVector<Folder> result;
    if (m_handlers.handleFolderName(name, result)) {
        m_path.swap(result);
        m_pathOrigin.reset();
        clearContent();
        // return true;
    } else {
        // FIXME: Handle result
        // return false;
    }
}

void
game::browser::Browser::openChild(size_t n)
{
    if (n < m_content.size()) {
        m_path.pushBackNew(m_content.extractElement(n));
        m_pathOrigin.reset();
        clearContent();
    }
}

void
game::browser::Browser::openParent()
{
    if (!m_path.empty()) {
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

void
game::browser::Browser::loadContent()
{
    try {
        // If we have a selected element, but not a previous path element, select that element
        size_t n;
        if (m_pathOrigin.get() == 0 && m_selectedChild.get(n) && n < m_content.size()) {
            m_pathOrigin.reset(m_content.extractElement(n));
        }

        // Replace content with newly-loaded values
        clearContent();
        currentFolder().loadContent(m_content);

        // If we have a previous path element, attempt to locate and select that
        if (m_pathOrigin.get() != 0) {
            for (size_t i = 0, n = m_content.size(); i < n; ++i) {
                if (m_content[i]->isSame(*m_pathOrigin)) {
                    selectChild(i);
                    break;
                }
            }
            m_pathOrigin.reset();
        }
    }
    catch (std::exception& e) {
        m_log.write(m_log.Warn, LOG_NAME, String_t(), e);
    }
}

void
game::browser::Browser::loadChildRoot()
{
    size_t n;
    if (!m_childLoaded && m_selectedChild.get(n) && n < m_content.size()) {
        m_childLoaded = true;
        m_childConfig.reset(new game::config::UserConfiguration());

        // Load configuration
        try {
            // FIXME: can we do anything with the return value?
            m_content[n]->loadConfiguration(*m_childConfig);
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, String_t(), e);
        }

        // Load root
        try {
            m_childRoot = m_content[n]->loadGameRoot(*m_childConfig);
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, String_t(), e);
        }
    }
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

game::browser::Browser::OptionalIndex_t
game::browser::Browser::getSelectedChild() const
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

void
game::browser::Browser::updateConfiguration()
{
    // Update configuration after user modified it:
    // - save to disk
    // - reload the root to let the new configuration take effect
    size_t n;
    if (m_childLoaded && m_selectedChild.get(n) && n < m_content.size() && m_childConfig.get() != 0) {
        // Save configuration
        try {
            m_content[n]->saveConfiguration(*m_childConfig);
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, String_t(), e);
        }

        // Load root
        try {
            m_childRoot = m_content[n]->loadGameRoot(*m_childConfig);
        }
        catch (std::exception& e) {
            m_log.write(m_log.Warn, LOG_NAME, String_t(), e);
        }
    }
}

game::browser::Folder*
game::browser::Browser::createAccountFolder(Account& account)
{
    Folder* result = m_handlers.createAccountFolder(account);
    if (result == 0) {
        result = new UnsupportedAccountFolder(m_translator, account);
    }
    return result;
}

afl::base::Ptr<game::Root>
game::browser::Browser::loadGameRoot(afl::base::Ref<afl::io::Directory> dir, const game::config::UserConfiguration& config)
{
    return m_handlers.loadGameRoot(dir, config);
}

String_t
game::browser::Browser::expandGameDirectoryName(String_t directoryName) const
{
    if (directoryName.compare(0, std::strlen(GAME_PREFIX), GAME_PREFIX, std::strlen(GAME_PREFIX)) == 0) {
        return m_fileSystem.makePathName(m_fileSystem.makePathName(m_profile.open()->getDirectoryName(), GAMES_DIR_NAME), directoryName.substr(5));
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
        updateConfiguration();
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
            if (trySetLocalDirectoryName(*gamesDir, simplifyFileName(afl::string::Format("%s %d", gameName, i)))) {
                return;
            }
        }
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
                m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator.translateString("Using directory \"%s\"").c_str(), directoryName));
                setSelectedLocalDirectoryName(GAME_PREFIX + directoryName);
                return true;
            }
        }
        catch (std::exception&) {
            return false;
        }
    }
}
