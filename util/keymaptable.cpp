/**
  *  \file util/keymaptable.cpp
  *  \brief Class util::KeymapTable
  */

#include <stdexcept>
#include "util/keymaptable.hpp"

util::KeymapTable::KeymapTable()
    : sig_keymapChange(),
      m_keymaps()
{ }

util::KeymapTable::~KeymapTable()
{ }

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

util::KeymapRef_t
util::KeymapTable::createKeymap(String_t name)
{
    // IntKeymap::createKeymap
    if (getKeymapByName(name) != 0) {
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
