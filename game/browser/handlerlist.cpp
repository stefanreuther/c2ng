/**
  *  \file game/browser/handlerlist.cpp
  *  \brief Class game::browser::HandlerList
  */

#include "game/browser/handlerlist.hpp"
#include "game/browser/folder.hpp"

// Constructor.
game::browser::HandlerList::HandlerList()
    : m_handlers()
{ }

// Destructor.
game::browser::HandlerList::~HandlerList()
{ }

// Add new handler.
void
game::browser::HandlerList::addNewHandler(Handler* h)
{
    if (h != 0) {
        m_handlers.pushBackNew(h);
    }
}

// Handle folder name or URL.
bool
game::browser::HandlerList::handleFolderName(String_t name, afl::container::PtrVector<Folder>& result)
{
    for (size_t i = 0, n = m_handlers.size(); i < n; ++i) {
        result.clear();
        if (m_handlers[i]->handleFolderName(name, result)) {
            // OK, found it!
            return true;
        }
    }
    return false;
}

// Create account folder.
game::browser::Folder*
game::browser::HandlerList::createAccountFolder(Account& acc)
{
    for (size_t i = 0, n = m_handlers.size(); i < n; ++i) {
        if (Folder* result = m_handlers[i]->createAccountFolder(acc)) {
            return result;
        }
    }
    return 0;
}

// Load game root for physical folder.
afl::base::Ptr<game::Root>
game::browser::HandlerList::loadGameRoot(afl::base::Ptr<afl::io::Directory> dir)
{
    afl::base::Ptr<game::Root> result;
    for (size_t i = 0, n = m_handlers.size(); i < n; ++i) {
        result = m_handlers[i]->loadGameRoot(dir);
        if (result.get() != 0) {
            break;
        }
    }
    return result;
}
