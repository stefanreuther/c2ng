/**
  *  \file game/nu/accountfolder.cpp
  */

#include "game/nu/accountfolder.hpp"
#include "game/nu/browserhandler.hpp"
#include "game/nu/gamefolder.hpp"

game::nu::AccountFolder::AccountFolder(BrowserHandler& handler, game::browser::Account& acc)
    : m_handler(handler),
      m_account(acc)
{ }

void
game::nu::AccountFolder::loadContent(afl::container::PtrVector<Folder>& result)
{
    afl::data::Access parsedResult = m_handler.getGameList(m_account);
    for (size_t i = 0, n = parsedResult("games").getArraySize(); i < n; ++i) {
        result.pushBackNew(new GameFolder(m_handler, m_account, parsedResult("games")[i]("game")("id").toInteger(), 0*i));
    }
}

afl::base::Ptr<game::Root>
game::nu::AccountFolder::loadGameRoot()
{
    return 0;
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
