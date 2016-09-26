/**
  *  \file game/browser/browser.cpp
  */

#include "game/browser/browser.hpp"
#include "afl/except/fileproblemexception.hpp"
#include "game/browser/folder.hpp"
#include "game/browser/handler.hpp"
#include "game/browser/unsupportedaccountfolder.hpp"

namespace {
    const char LOG_NAME[] = "game.browser";
}

game::browser::Browser::Browser(afl::io::FileSystem& fileSystem,
                                afl::string::Translator& tx,
                                afl::sys::LogListener& log,
                                AccountManager& accounts,
                                UserCallback& callback)
    : m_fileSystem(fileSystem),
      m_translator(tx),
      m_log(log),
      m_accounts(accounts),
      m_callback(callback),
      m_handlers(),
      m_path(),
      m_pathOrigin(),
      m_content(),
      m_rootFolder(*this),
      m_selectedChild(),
      m_childLoaded(false),
      m_childRoot()
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
        try {
            m_childRoot = m_content[n]->loadGameRoot();
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
game::browser::Browser::loadGameRoot(afl::base::Ptr<afl::io::Directory> dir)
{
    return m_handlers.loadGameRoot(dir);
}
