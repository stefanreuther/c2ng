/**
  *  \file util/keymap.cpp
  *  \brief Class util::Keymap
  */

#include <stdexcept>
#include "util/keymap.hpp"

// Constructor.
util::Keymap::Keymap(const String_t& name)
    : m_parents(),
      m_keys(),
      m_name(name),
      m_changed(false)
{
    // ex IntKeymap::IntKeymap
}

// Destructor.
util::Keymap::~Keymap()
{ }

// Add key to this keymap.
void
util::Keymap::addKey(Key_t key, Atom_t command, Atom_t condition)
{
    // ex IntKeymap::addKey(const uint32_t key, const uint32_t command, const uint32_t condition)
    /* Replace existing key? */
    for (std::vector<Entry>::iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        if (it->key == key) {
            if (it->command != command || it->condition != condition) {
                it->command = command;
                it->condition = condition;
                markChanged();
            }
            return;
        }
    }

    /* Add new key */
    m_keys.push_back(Entry(key, command, condition));
    markChanged();
}

// Add parent to this keymap.
void
util::Keymap::addParent(Keymap& km)
{
    // ex IntKeymap::addParent(IntKeymapRef km)
    if (hasParent(km)) {
        throw std::runtime_error("Duplicate parent keymap");
    }
    if (km.hasParent(*this)) {
        throw std::runtime_error("Invalid parent keymap");
    }
    m_parents.push_back(&km);
    markChanged();
}

// Given a key, look up its command.
util::Atom_t
util::Keymap::lookupCommand(Key_t key) const
{
    // ex IntKeymap::lookupCommand(uint32_t key) const
    if (const Entry* e = lookup(key, 0))
        return e->command;
    return 0;
}

// Given a key, look up its command and place of definition.
util::Atom_t
util::Keymap::lookupCommand(Key_t key, KeymapRef_t& where) const
{
    // ex IntKeymap::lookupCommand(uint32_t key, IntKeymapRef& where) const
    where = 0;
    if (const Entry* e = lookup(key, &where))
        return e->command;
    return 0;
}

// Given a key, look up its condition.
util::Atom_t
util::Keymap::lookupCondition(Key_t key) const
{
    // ex IntKeymap::lookupCondition(uint32_t key) const
    if (const Entry* e = lookup(key, 0))
        return e->condition;
    return 0;
}

// Check for parent relation ship, recursively.
bool
util::Keymap::hasParent(Keymap& km) const
{
    // ex IntKeymap::hasParent(IntKeymap* km) const
    if (&km == this) {
        return true;
    }
    for (std::vector<Keymap*>::const_iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
        if ((*it)->hasParent(km)) {
            return true;
        }
    }
    return false;
}

// Enumerate keys.
void
util::Keymap::enumKeys(KeySet_t& keys) const
{
    // Own keys
    for (std::vector<Entry>::const_iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        keys.insert(it->key);
    }

    // Parent keys
    for (std::vector<Keymap*>::const_iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
        (*it)->enumKeys(keys);
    }
}

// Mark this keymap changed.
void
util::Keymap::markChanged(bool state)
{
    m_changed = state;
}

// Check whether this keymap was changed.
bool
util::Keymap::isChanged() const
{
    return m_changed;
}

/** Look up a key, recursively.
    \param key [in] Key to find
    \param where [out,optional] Keymap in which key is bound
    \return found Entry, if any. Otherwise null. */
const util::Keymap::Entry*
util::Keymap::lookup(Key_t key, KeymapRef_t* where) const
{
    // ex IntKeymap::lookup(const uint32_t key, IntKeymapRef*const where) const
    for (std::vector<Entry>::const_iterator it = m_keys.begin(); it != m_keys.end(); ++it) {
        if (it->key == key) {
            if (where) {
                *where = const_cast<Keymap*>(this);
            }
            return &*it;
        }
    }
    for (std::vector<Keymap*>::const_iterator it = m_parents.begin(); it != m_parents.end(); ++it) {
        if (const Entry* e = (*it)->lookup(key, where)) {
            return e;
        }
    }
    return 0;
}
