/**
  *  \file game/nu/accountfolder.cpp
  *  \brief Class game::nu::AccountFolder
  */

#include "game/nu/accountfolder.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/nu/gamefolder.hpp"

namespace {
    const char*const LOG_NAME = "game.nu";
}

game::nu::AccountFolder::AccountFolder(BrowserHandler& handler, game::browser::Account& acc)
    : m_handler(handler),
      m_account(acc)
{ }

std::auto_ptr<game::Task_t>
game::nu::AccountFolder::loadContent(std::auto_ptr<game::browser::LoadContentTask_t> then)
{
    class Task : public Task_t {
     public:
        Task(BrowserHandler& handler, game::browser::Account& acc, std::auto_ptr<game::browser::LoadContentTask_t>& then)
            : m_handler(handler), m_account(acc), m_then(then)
            { }
        virtual void call()
            {
                m_handler.log().write(afl::sys::LogListener::Trace, LOG_NAME, "Task: AccountFolder.loadContent");
                afl::container::PtrVector<Folder> result;
                afl::data::Access parsedResult = m_handler.getGameListPreAuthenticated(m_account);
                for (size_t i = 0, n = parsedResult("games").getArraySize(); i < n; ++i) {
                    result.pushBackNew(new GameFolder(m_handler, m_account, parsedResult("games")[i]("game")("id").toInteger(), 0*i));
                }
                m_then->call(result);
            }
     private:
        BrowserHandler& m_handler;
        game::browser::Account& m_account;
        std::auto_ptr<game::browser::LoadContentTask_t> m_then;
    };
    return m_handler.login(m_account, std::auto_ptr<Task_t>(new Task(m_handler, m_account, then)));
}

bool
game::nu::AccountFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    // No game in this folder
    return false;
}

void
game::nu::AccountFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{ }

bool
game::nu::AccountFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    // No game in this folder
    return false;
}

std::auto_ptr<game::Task_t>
game::nu::AccountFolder::loadGameRoot(const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t> then)
{
    // No game in this folder
    return defaultLoadGameRoot(then);
}

String_t
game::nu::AccountFolder::getName() const
{
    return m_account.getName();
}

util::rich::Text
game::nu::AccountFolder::getDescription() const
{
    return m_handler.translator().translateString("planets.nu account");
}

bool
game::nu::AccountFolder::isSame(const Folder& other) const
{
    const AccountFolder* p = dynamic_cast<const AccountFolder*>(&other);
    return p != 0 && &p->m_account == &m_account;
}

bool
game::nu::AccountFolder::canEnter() const
{
    return true;
}

game::browser::Folder::Kind
game::nu::AccountFolder::getKind() const
{
    return kAccount;
}
