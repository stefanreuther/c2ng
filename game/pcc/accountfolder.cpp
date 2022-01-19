/**
  *  \file game/pcc/accountfolder.cpp
  */

#include "game/pcc/accountfolder.hpp"
#include "game/pcc/browserhandler.hpp"
#include "game/pcc/gamefolder.hpp"

namespace {
    struct SortByName {
        bool operator()(const game::browser::Folder& a, const game::browser::Folder& b)
            { return a.getName() < b.getName(); }
    };
}


game::pcc::AccountFolder::AccountFolder(BrowserHandler& handler, game::browser::Account& acc)
    : m_handler(handler),
      m_account(acc)
{ }

void
game::pcc::AccountFolder::loadContent(afl::container::PtrVector<Folder>& result)
{
    afl::data::Access p = m_handler.getGameList(m_account)("reply");
    for (size_t i = 0, n = p.getArraySize(); i < n; ++i) {
        result.pushBackNew(new GameFolder(m_handler, m_account, p[i]("path").toString(), i));
    }
    result.sort(SortByName());
}

bool
game::pcc::AccountFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    return false;
}

void
game::pcc::AccountFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{ }

bool
game::pcc::AccountFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    return false;
}

std::auto_ptr<game::browser::Task_t>
game::pcc::AccountFolder::loadGameRoot(const game::config::UserConfiguration& /*config*/, std::auto_ptr<game::browser::LoadGameRootTask_t> then)
{
    return defaultLoadGameRoot(then);
}

String_t
game::pcc::AccountFolder::getName() const
{
    return m_account.getName();
}

util::rich::Text
game::pcc::AccountFolder::getDescription() const
{
    return m_handler.translator().translateString("planetscentral.com account");
}

bool
game::pcc::AccountFolder::isSame(const Folder& other) const
{
    const AccountFolder* p = dynamic_cast<const AccountFolder*>(&other);
    return p != 0 && &p->m_account == &m_account;
}

bool
game::pcc::AccountFolder::canEnter() const
{
    return true;
}

game::pcc::AccountFolder::Kind
game::pcc::AccountFolder::getKind() const
{
    return kAccount;
}
