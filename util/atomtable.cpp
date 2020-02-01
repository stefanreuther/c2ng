/**
  *  \file util/atomtable.cpp
  *  \brief Class util::AtomTable
  *
  *  FIXME: this almost the PCC2 implementation, slightle beefed up to use afl APIs.
  *  It happens that NameMap (PCC2: IntVariableNames) solves a very similar problem,
  *  so it makes sense to merge these.
  */

#include <cassert>
#include "util/atomtable.hpp"
#include "afl/checksums/bytesum.hpp"

namespace {
    // Compute hash value over a character array. For now, we simply add all characters.
    size_t strHash(afl::base::ConstBytes_t a)
    {
        // ex util/atom.h:strHash
        return afl::checksums::ByteSum().add(a, 0);
    }
}

const util::Atom_t util::AtomTable::ATOM_LOWER_BOUND;
const util::Atom_t util::AtomTable::NULL_ATOM;

// Constructor.
util::AtomTable::AtomTable()
    : m_entries(),
      m_hashFirst(),
      m_nextAtom()
{
    // ex AtomTable::AtomTable
    clear();
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
    return afl::string::fromBytes(atomStrLookup(atom));
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
    afl::container::PtrVector<AtomChunk>().swap(m_entries);
    afl::base::Memory<Atom_t>(m_hashFirst).fill(NULL_ATOM);
    m_nextAtom = ATOM_LOWER_BOUND;
}

// Check validity of atom.
bool
util::AtomTable::isAtom(Atom_t atom) const
{
    // ex util/atom.h:isAtom
    return (atom >= ATOM_LOWER_BOUND && atom < m_nextAtom);
}

/** Look up definition of an atom. Returns address/length pair.
    \pre isAtom(atom) */
afl::base::ConstBytes_t
util::AtomTable::atomStrLookup(Atom_t atom) const
{
    if (atom >= ATOM_LOWER_BOUND && atom < m_nextAtom) {
        atom -= ATOM_LOWER_BOUND;
        const size_t slot = atom % ATOMS_PER_CHUNK;
        const size_t blk  = atom / ATOMS_PER_CHUNK;
        const AtomChunk& pac = *m_entries[blk];
        return pac.data.subrange(pac.pos[slot], pac.length[slot]);
    } else {
        return afl::base::ConstBytes_t();
    }
}

/** Look up string. This is the common back-end of the getAtomFromString() functions.
    \param a    string
    \param add  when true, this creates the atom when its not found
    \return atom if found or newly-created, NULL_ATOM if not found and add was false. */
util::Atom_t
util::AtomTable::atomLookup(afl::base::ConstBytes_t a, bool add)
{
    // ex atomLookup
    if (a.empty()) {
        return NULL_ATOM;
    }

    size_t hv = strHash(a) % ATOM_HASH;
    Atom_t index = m_hashFirst[hv];
    while (index) {
        afl::base::ConstBytes_t ad = atomStrLookup(index);
        if (ad.equalContent(a)) {
            return index;
        }
        index -= ATOM_LOWER_BOUND;
        index = m_entries[index / ATOMS_PER_CHUNK]->hashNext[index % ATOMS_PER_CHUNK];
    }
    if (!add) {
        return NULL_ATOM;
    }

    index = m_nextAtom - ATOM_LOWER_BOUND;
    size_t slot = index % ATOMS_PER_CHUNK;
    size_t blk  = index / ATOMS_PER_CHUNK;
    if (slot == 0) {
        /* we're starting a new block */
        assert(m_entries.size() == blk);
        m_entries.pushBackNew(new AtomChunk());
    }
    AtomChunk& pac = *m_entries[blk];
    pac.pos[slot] = pac.data.size();
    pac.length[slot] = a.size();
    pac.data.append(a);
    pac.hashNext[slot] = m_hashFirst[hv];
    m_hashFirst[hv] = m_nextAtom;
    return m_nextAtom++;
}
