/**
  *  \file game/browser/rootfolder.cpp
  *  \brief Class game::browser::RootFolder
  */

#include "game/browser/rootfolder.hpp"
#include "game/browser/filesystemrootfolder.hpp"
#include "game/browser/browser.hpp"
#include "game/browser/accountmanager.hpp"

game::browser::RootFolder::RootFolder(Browser& parent)
    : m_parent(parent)
{ }

game::browser::RootFolder::~RootFolder()
{ }

void
game::browser::RootFolder::loadContent(afl::container::PtrVector<Folder>& result)
{
    // FIXME: favorites
    result.pushBackNew(new FileSystemRootFolder(m_parent));

    for (size_t i = 0, n = m_parent.accounts().getNumAccounts(); i < n; ++i) {
        if (Account* acc = m_parent.accounts().getAccount(i)) {
            if (Folder* f = m_parent.createAccountFolder(*acc)) {
                result.pushBackNew(f);
            }
        }
    }
}

bool
game::browser::RootFolder::loadConfiguration(game::config::UserConfiguration& /*config*/)
{
    return false;
}

void
game::browser::RootFolder::saveConfiguration(const game::config::UserConfiguration& /*config*/)
{ }

bool
game::browser::RootFolder::setLocalDirectoryName(String_t /*directoryName*/)
{
    return false;
}

std::auto_ptr<game::Task_t>
game::browser::RootFolder::loadGameRoot(const game::config::UserConfiguration& /*config*/, std::auto_ptr<LoadGameRootTask_t> then)
{
    // No game in root
    return defaultLoadGameRoot(then);
}

String_t
game::browser::RootFolder::getName() const
{
    // User should never see this
    return "<Root>";
}

util::rich::Text
game::browser::RootFolder::getDescription() const
{
    return util::rich::Text();
}

bool
game::browser::RootFolder::isSame(const Folder& other) const
{
    return dynamic_cast<const RootFolder*>(&other) != 0;
}

bool
game::browser::RootFolder::canEnter() const
{
    return true;
}

game::browser::Folder::Kind
game::browser::RootFolder::getKind() const
{
    return kRoot;
}
