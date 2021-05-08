/**
  *  \file util/keymaptable.cpp
  */

#include <stdexcept>
#include "util/keymaptable.hpp"

util::KeymapTable::KeymapTable()
    : sig_keymapChange(),
      m_keymaps()
{ }

util::KeymapTable::~KeymapTable()
{ }

/** Get keymap by name. Note that the name must be in upper-case, because that's how it
    comes in from the script interface.
    \param name Name to find
    \return Found keymap, 0 if it does not exist. */
util::KeymapRef_t
util::KeymapTable::getKeymapByName(String_t name) const
{
    // ex IntKeymap::getKeymapByName
    for (Vector_t::iterator it = m_keymaps.begin(); it != m_keymaps.end(); ++it) {
        if ((*it)->getName() == name) {
            return *it;
        }
    }
    return 0;
}

/** Create keymap. Note that the name must be in upper-case, because that's how it
    comes in from the script interface.
    \param name Name to create
    \return New keymap.
    \throw IntError if keymap by this name already exists */
util::KeymapRef_t
util::KeymapTable::createKeymap(String_t name)
{
    // IntKeymap::createKeymap
    // FIXME: PCC2 throws IntError
    if (getKeymapByName(name)) {
        throw std::runtime_error("Keymap already exists");
    }
    return m_keymaps.pushBackNew(new Keymap(name));
}

size_t
util::KeymapTable::getNumKeymaps() const
{
    return m_keymaps.size();
}

util::KeymapRef_t
util::KeymapTable::getKeymapByIndex(size_t index) const
{
    if (index < m_keymaps.size()) {
        return m_keymaps[index];
    } else {
        return 0;
    }
}

void
util::KeymapTable::notifyListeners()
{
    bool doit = false;
    for (Vector_t::iterator it = m_keymaps.begin(); it != m_keymaps.end(); ++it) {
        if ((*it)->isChanged()) {
            doit = true;
            (*it)->markChanged(false);
        }
    }
    if (doit) {
        sig_keymapChange.raise();
    }
}
