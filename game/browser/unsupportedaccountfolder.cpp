/**
  *  \file game/browser/unsupportedaccountfolder.cpp
  *  \brief Class game::browser::UnsupportedAccountFolder
  */

#include "game/browser/unsupportedaccountfolder.hpp"
#include "afl/string/format.hpp"
#include "game/browser/account.hpp"
#include "util/rich/text.hpp"

game::browser::UnsupportedAccountFolder::UnsupportedAccountFolder(afl::string::Translator& tx, const Account& account)
    : m_translator(tx),
      m_account(account)
{ }

game::browser::UnsupportedAccountFolder::~UnsupportedAccountFolder()
{ }

void
game::browser::UnsupportedAccountFolder::loadContent(afl::container::PtrVector<Folder>& /*result*/)
{
    // No content.
}

bool
game::browser::UnsupportedAccountFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    return false;
}

void
game::browser::UnsupportedAccountFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{ }

bool
game::browser::UnsupportedAccountFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    return false;
}

std::auto_ptr<game::Task_t>
game::browser::UnsupportedAccountFolder::loadGameRoot(const game::config::UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t> then)
{
    // No content.
    return defaultLoadGameRoot(then);
}

String_t
game::browser::UnsupportedAccountFolder::getName() const
{
    return m_account.getName();
}

util::rich::Text
game::browser::UnsupportedAccountFolder::getDescription() const
{
    return util::rich::Text(afl::string::Format(m_translator("This version of PCC2 does not support this account of type \"%s\"."), m_account.getType())).withColor(util::SkinColor::Red);
}

bool
game::browser::UnsupportedAccountFolder::isSame(const Folder& other) const
{
    const UnsupportedAccountFolder* p = dynamic_cast<const UnsupportedAccountFolder*>(&other);
    return p != 0
        && &p->m_account == &m_account;
}

bool
game::browser::UnsupportedAccountFolder::canEnter() const
{
    return false;
}

game::browser::Folder::Kind
game::browser::UnsupportedAccountFolder::getKind() const
{
    return kAccount;
}
