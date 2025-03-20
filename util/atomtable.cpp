/**
  *  \file util/atomtable.cpp
  *  \brief Class util::AtomTable
  *
  *  This class has been rewritten to use NameMap (instead of a custom low-level implementation).
  */

#include "util/atomtable.hpp"

const util::Atom_t util::AtomTable::ATOM_LOWER_BOUND;
const util::Atom_t util::AtomTable::NULL_ATOM;

// Constructor.
util::AtomTable::AtomTable()
    : m_content()
{
    // ex AtomTable::AtomTable
}

// Destructor.
util::AtomTable::~AtomTable()
{
    // ex AtomTable::~AtomTable
}

// Get atom, given a C string.
util::Atom_t
util::AtomTable::getAtomFromString(const char* str)
{
    // ex util/atom.h:atom
    return atomLookup(afl::string::toBytes(str), true);
}

// Get atom, given a string.
util::Atom_t
util::AtomTable::getAtomFromString(const String_t& str)
{
    // ex util/atom.h:atom
    return atomLookup(afl::string::toBytes(str), true);
}

// Get string, given an atom.
String_t
util::AtomTable::getStringFromAtom(Atom_t atom) const
{
    // ex util/atom.h:atomStr
    return atomStrLookup(atom);
}

// Get atom, given a C string, but does not create it.
util::Atom_t
util::AtomTable::getAtomFromStringNC(const char* str) const
{
    // ex util/atom.h:atomNC
    return const_cast<AtomTable*>(this)->atomLookup(afl::string::toBytes(str), false);
}

// Get atom, given a string, but does not create it.
util::Atom_t
util::AtomTable::getAtomFromStringNC(const String_t& str) const
{
    // ex util/atom.h:atomNC
    return const_cast<AtomTable*>(this)->atomLookup(afl::string::toBytes(str), false);
}

// Clear table.
void
util::AtomTable::clear()
{
    // ex atomClear
    // ex AtomTable::reset
    afl::data::NameMap().swap(m_content);
}

// Check validity of atom.
bool
util::AtomTable::isAtom(Atom_t atom) const
{
    // ex util/atom.h:isAtom
    return (atom >= ATOM_LOWER_BOUND && atom < ATOM_LOWER_BOUND + m_content.getNumNames());
}

/** Look up definition of an atom. Returns address/length pair.
    \pre isAtom(atom) */
String_t
util::AtomTable::atomStrLookup(Atom_t atom) const
{
    if (atom >= ATOM_LOWER_BOUND && atom < ATOM_LOWER_BOUND + m_content.getNumNames()) {
        return m_content.getNameByIndex(atom - ATOM_LOWER_BOUND);
    } else {
        return String_t();
    }
}

/** Look up string. This is the common back-end of the getAtomFromString() functions.
    \param a    string
    \param add  when true, this creates the atom when its not found
    \return atom if found or newly-created, NULL_ATOM if not found and add was false. */
util::Atom_t
util::AtomTable::atomLookup(afl::base::ConstBytes_t a, bool add)
{
    if (a.empty()) {
        return NULL_ATOM;
    }

    afl::data::NameMap::Index_t idx = m_content.getIndexByName(afl::string::fromBytes(a));
    if (idx != afl::data::NameMap::nil) {
        return static_cast<Atom_t>(idx + ATOM_LOWER_BOUND);
    }
    if (!add) {
        return NULL_ATOM;
    }
    return static_cast<Atom_t>(m_content.add(afl::string::fromBytes(a)) + ATOM_LOWER_BOUND);
}
