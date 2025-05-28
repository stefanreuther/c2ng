/**
  *  \file game/pcc/gamefolder.cpp
  *  \brief Class game::pcc::GameFolder
  */

#include "game/pcc/gamefolder.hpp"
#include "afl/charset/codepage.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/string/format.hpp"
#include "game/pcc/browserhandler.hpp"
#include "util/string.hpp"

using afl::sys::LogListener;
using game::Task_t;

namespace {
    const char*const LOG_NAME = "game.pcc";
}

game::pcc::GameFolder::GameFolder(BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& acc, String_t path, size_t hint)
    : m_handler(handler),
      m_account(acc),
      m_path(path),
      m_hint(hint)
{ }

// Folder:
void
game::pcc::GameFolder::loadContent(afl::container::PtrVector<Folder>& /*result*/)
{
    // Nothing to load, there are no subfolders
}

bool
game::pcc::GameFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    return false; /* FIXME: get associated folder from account and load that */
}

void
game::pcc::GameFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{
    /* FIXME: get associated folder from account and save there */
}

bool
game::pcc::GameFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    // FIXME: implement
    return false;
}

std::auto_ptr<game::Task_t>
game::pcc::GameFolder::loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(String_t pathName, size_t hint, BrowserHandler& handler, const afl::base::Ref<game::browser::Account>& account, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then)
            : m_pathName(pathName), m_hint(hint), m_handler(handler), m_account(account), m_config(config), m_then(then)
            { }
        virtual void call()
            {
                m_handler.log().write(LogListener::Trace, LOG_NAME, "Task: GameFolder.loadGameRoot");
                afl::base::Ptr<Root> result;
                try {
                    result = m_handler.loadRoot(m_account, GameFolder(m_handler, m_account, m_pathName, m_hint).getGameListEntry(), m_config);
                }
                catch (std::exception& e) {
                    m_handler.log().write(LogListener::Error, LOG_NAME, String_t(), e);
                }
                m_then->call(result);
            }

     private:
        const String_t m_pathName;
        size_t m_hint;
        BrowserHandler& m_handler;
        afl::base::Ref<game::browser::Account> m_account;
        const game::config::UserConfiguration& m_config;
        std::auto_ptr<game::browser::LoadGameRootTask_t> m_then;
    };

    // Log in, then build the root.
    return m_handler.login(m_account, std::auto_ptr<Task_t>(new Task(m_path, m_hint, m_handler, m_account, config, then)));
}

String_t
game::pcc::GameFolder::getName() const
{
    afl::data::Access a = getGameListEntry();
    String_t name = a("name").toString();
    if (name.empty()) {
        name = m_path;
        if (util::strStartsWith(name, "u/")) {
            name.erase(0, 2);
        }
    }
    int32_t hostGameNumber = a("game").toInteger();
    if (hostGameNumber != 0) {
        name += afl::string::Format(" (#%d)", hostGameNumber);
    }
    return name;
}

util::rich::Text
game::pcc::GameFolder::getDescription() const
{
    return m_handler.translator()("Server-side game directory");
}

bool
game::pcc::GameFolder::isSame(const Folder& other) const
{
    const GameFolder* p = dynamic_cast<const GameFolder*>(&other);
    return p != 0
        && &*p->m_account == &*m_account
        && p->m_path == m_path;
}

bool
game::pcc::GameFolder::canEnter() const
{
    return false;
}

game::pcc::GameFolder::Kind
game::pcc::GameFolder::getKind() const
{
    return kGame;
}

afl::data::Access
game::pcc::GameFolder::getGameListEntry() const
{
    afl::data::Access a = m_handler.getGameListPreAuthenticated(m_account);

    // Try the hint
    {
        afl::data::Access guess = a("reply")[m_hint];
        if (guess("path").toString() == m_path) {
            return guess;
        }
    }

    // No, find it
    for (size_t i = 0, n = a("reply").getArraySize(); i < n; ++i) {
        afl::data::Access elem = a("reply")[i];
        if (elem("path").toString() == m_path) {
            m_hint = i;
            return elem;
        }
    }
    return afl::data::Access();
}
