/**
  *  \file util/atomtable.hpp
  */
#ifndef C2NG_UTIL_ATOMTABLE_HPP
#define C2NG_UTIL_ATOMTABLE_HPP

#include <algorithm>
#include "afl/string/string.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"

namespace util {

    /** Integer type that represents an atom. */
    typedef uint32_t Atom_t;

    class AtomTable {
     public:
        /** Lowest possible value returned by atom() for a non-null atom. */
        static const Atom_t ATOM_LOWER_BOUND = 20000;

        /** Null atom. Corresponds to the empty string. */
        static const Atom_t NULL_ATOM = 0;

        AtomTable();
        ~AtomTable();

        Atom_t getAtomFromString(const char* str);
        Atom_t getAtomFromString(const String_t& str);
        String_t getStringFromAtom(Atom_t atom) const;
        Atom_t getAtomFromStringNC(const char* str) const;
        Atom_t getAtomFromStringNC(const String_t& str) const;

        void clear();

        bool isAtom(Atom_t atom) const;

     private:
        /** Manifest constants. */
        enum {
            ATOMS_PER_CHUNK = 128,      /** Atoms per memory chunk. */
            ATOM_HASH       = 128       /** Number of hash-table slots. */
        };

        typedef std::pair<const char*, size_t> AtomDef;

        struct AtomChunk {
            /** All atoms of this chunk in one block */
            String_t data;
            /** Positions of each atom */
            size_t pos[ATOMS_PER_CHUNK];
            size_t length[ATOMS_PER_CHUNK];
            /** Hash links for each atom */
            Atom_t hash_next[ATOMS_PER_CHUNK];
        };

        afl::container::PtrVector<AtomChunk> entry_vec;
        Atom_t hash_first[ATOM_HASH];
        Atom_t next;

        AtomDef atomStrLookup(Atom_t atom) const;
        Atom_t atomLookup(const char* data, size_t length, bool add);
    };

}

#endif
