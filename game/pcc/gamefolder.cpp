/**
  *  \file game/pcc/gamefolder.cpp
  */

#include "game/pcc/gamefolder.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/pcc/serverdirectory.hpp"
#include "afl/charset/codepagecharset.hpp"
#include "afl/charset/codepage.hpp"

using afl::sys::LogListener;

namespace {
    const char*const LOG_NAME = "game.pcc";
}

game::pcc::GameFolder::GameFolder(BrowserHandler& handler, game::browser::Account& acc, String_t path, size_t hint)
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

std::auto_ptr<game::browser::Task_t>
game::pcc::GameFolder::loadGameRoot(const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t> then)
{
    class Task : public game::browser::Task_t {
     public:
        Task(String_t pathName, BrowserHandler& handler, game::browser::Account& account, const game::config::UserConfiguration& config, std::auto_ptr<game::browser::LoadGameRootTask_t>& then)
            : m_pathName(pathName), m_handler(handler), m_account(account), m_config(config), m_then(then)
            { }
        virtual void call()
            {
                m_handler.log().write(LogListener::Trace, LOG_NAME, "GameFolder.loadGameRoot.Task");
                afl::base::Ptr<Root> result;
                try {
                    // Quick and dirty solution: pretend this to be a local folder and work with that.
                    // FIXME: this needs a lot of optimisation (and quite a number of protocol improvements on server side).
                    afl::charset::CodepageCharset cs(afl::charset::g_codepageLatin1);
                    result = m_handler.loader().load(*new ServerDirectory(m_handler, m_account, m_pathName), cs, m_config, false);
                }
                catch (std::exception& e) {
                    m_handler.log().write(LogListener::Error, LOG_NAME, String_t(), e);
                }
                m_then->call(result);
            }

     private:
        const String_t m_pathName;
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        const game::config::UserConfiguration& m_config;
        std::auto_ptr<game::browser::LoadGameRootTask_t> m_then;
    };
    return std::auto_ptr<game::browser::Task_t>(new Task(m_path, m_handler, m_account, config, then));
}

String_t
game::pcc::GameFolder::getName() const
{
    afl::data::Access a = getGameListEntry();
    String_t name = a("name").toString();
    if (name.empty()) {
        return m_path;
    } else {
        return name;
    }
}

util::rich::Text
game::pcc::GameFolder::getDescription() const
{
    return m_handler.translator().translateString("Server-side game directory");
}

bool
game::pcc::GameFolder::isSame(const Folder& other) const
{
    const GameFolder* p = dynamic_cast<const GameFolder*>(&other);
    return p != 0
        && &p->m_account == &m_account
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
    afl::data::Access a = m_handler.getGameList(m_account);

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
