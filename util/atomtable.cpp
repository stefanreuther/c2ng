/**
  *  \file util/atomtable.cpp
  *
  *  This module manages atoms. An atom is an integer representing a string.
  *  A fast mapping between strings and atoms is provided.
  *
  *  Differences to the PCC 1.x implementation:
  *  - we have no real upper bound for atoms.
  *  - there are functions atomNC to inquire the atom table without
  *    creating an atom. These should not be used in user code except
  *    as an optimisation.
  *
  *  FIXME: this is more or less the PCC2 implementation.
  *  It happens that NameMap (PCC2: IntVariableNames) solves a very similar problem,
  *  so it makes sense to merge these.
  */

#include <cassert>
#include <cstring>
#include "util/atomtable.hpp"

namespace {
    /** Compute hash value over a character array. For now, we simply add all characters.
        \param str    [in] Array
        \param length [in] Size
        \return hash value */
    size_t strHash(const char* str, std::size_t length)
    {
        // ex util/atom.h:strHash
        size_t sum = 0;
        while (length--)
            sum += static_cast<unsigned char>(*str++);
        return sum;
    }
}

const util::Atom_t util::AtomTable::ATOM_LOWER_BOUND;
const util::Atom_t util::AtomTable::NULL_ATOM;

util::AtomTable::AtomTable()
    : entry_vec(),
      hash_first(),
      next()
{
    // ex AtomTable::AtomTable
    clear();
}

util::AtomTable::~AtomTable()
{
    // ex AtomTable::~AtomTable
}

// /** Get atom corresponding to string str.
//     \param str Atom to look up / create
//     \return atom number
//     \post str!="" => isAtom(return) && atomStr(return)==str */
util::Atom_t
util::AtomTable::getAtomFromString(const char* str)
{
    // ex util/atom.h:atom
    return atomLookup(str, std::strlen(str), true);
}

util::Atom_t
util::AtomTable::getAtomFromString(const String_t& str)
{
    // ex util/atom.h:atom
    return atomLookup(str.data(), str.length(), true);
}

// /** Return string associated with an atom. Returns empty string if atom
//     is invalid. */
String_t
util::AtomTable::getStringFromAtom(Atom_t atom) const
{
    // ex util/atom.h:atomStr
    if (atom < ATOM_LOWER_BOUND || atom >= next) {
        return String_t();
    } else {
        AtomDef d = atomStrLookup(atom);
        return String_t(d.first, d.second);
    }
}

// /** Look up atom corresponding to string str, but does not create it if
//     there isn't one already.
//     \param str [in] C-string
//     \return atom, 0 if not found or str=="" */
util::Atom_t
util::AtomTable::getAtomFromStringNC(const char* str) const
{
    // ex util/atom.h:atomNC
    return const_cast<AtomTable*>(this)->atomLookup(str, std::strlen(str), false);
}

// /** Look up atom corresponding to string str, but does not create it if
//     there isn't one already. \overload */
util::Atom_t
util::AtomTable::getAtomFromStringNC(const String_t& str) const
{
    // ex util/atom.h:atomNC
    return const_cast<AtomTable*>(this)->atomLookup(str.data(), str.size(), false);
}

void
util::AtomTable::clear()
{
    // ex atomClear
    // ex AtomTable::reset
    afl::container::PtrVector<AtomChunk>().swap(entry_vec);
    afl::base::Memory<Atom_t>(hash_first).fill(NULL_ATOM);
    next = ATOM_LOWER_BOUND;
}

// /** True iff atom corresponds to a string. */
bool
util::AtomTable::isAtom(Atom_t atom) const
{
    // ex util/atom.h:isAtom
    return (atom >= ATOM_LOWER_BOUND && atom < next);
}

// /** Look up definition of an atom. Returns address/length pair.
//     \pre isAtom(atom) */
util::AtomTable::AtomDef
util::AtomTable::atomStrLookup(Atom_t atom) const
{
    assert(atom >= ATOM_LOWER_BOUND && atom < next);
    atom -= ATOM_LOWER_BOUND;
    size_t slot = atom % ATOMS_PER_CHUNK;
    size_t blk  = atom / ATOMS_PER_CHUNK;
    const AtomChunk& pac = *entry_vec[blk];
    return std::make_pair(pac.data.data() + pac.pos[slot], pac.length[slot]);
}

// /** Look up value of an atom. This is the common back-end of atom() and
//     atomNC().
//     \param data    text
//     \param length  its length
//     \param add     when true, this creates the atom when its not found
//     \return atom if found or newly-created, NULL_ATOM if not found and
//     add was false. */
util::Atom_t
util::AtomTable::atomLookup(const char* data, size_t length, bool add)
{
    // ex atomLookup
    if (!length) {
        return NULL_ATOM;
    }

    size_t hv = strHash(data, length) % ATOM_HASH;
    Atom_t index = hash_first[hv];
    while (index) {
        AtomDef ad = atomStrLookup(index);
        if (ad.second == length && std::memcmp(ad.first, data, length) == 0) {
            return index;
        }
        index -= ATOM_LOWER_BOUND;
        index = entry_vec[index / ATOMS_PER_CHUNK]->hash_next[index % ATOMS_PER_CHUNK];
    }
    if (!add) {
        return NULL_ATOM;
    }

    index = next - ATOM_LOWER_BOUND;
    size_t slot = index % ATOMS_PER_CHUNK;
    size_t blk  = index / ATOMS_PER_CHUNK;
    if (slot == 0) {
        /* we're starting a new block */
        assert(entry_vec.size() == blk);
        entry_vec.pushBackNew(new AtomChunk());
    }
    AtomChunk& pac = *entry_vec[blk];
    pac.pos[slot] = pac.data.length();
    pac.length[slot] = length;
    pac.data.append(data, length);
    pac.hash_next[slot] = hash_first[hv];
    hash_first[hv] = next;
    return next++;
}
