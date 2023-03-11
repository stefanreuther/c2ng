/**
  *  \file util/atomtable.hpp
  *  \brief Class util::AtomTable
  */
#ifndef C2NG_UTIL_ATOMTABLE_HPP
#define C2NG_UTIL_ATOMTABLE_HPP

#include <algorithm>
#include "afl/base/growablememory.hpp"
#include "afl/base/memory.hpp"
#include "afl/base/types.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Integer type that represents an atom. */
    typedef uint32_t Atom_t;

    /** Atom (string-to-integer) mapping.

        An AtomTable module manages atoms.
        An atom is an integer representing a string.
        A fast bidirectional mapping between strings and atoms is provided.

        Differences to the PCC 1.x implementation:
        - we have no real upper bound for atoms.
        - there are functions getAtomFromStringNC to inquire the atom table without creating an atom.
          These should not be used in user code except as an optimisation.

        Invariants:
        - the empty string produces 0 (NULL_ATOM), a non-empty string produces a nonzero number (>= ATOM_LOWER_BOUND).
        - requesting the same string again produces the same integer again
        - string comparisons are case-sensitive (no case-folding or other interpretation) */
    class AtomTable {
     public:
        /** Lowest possible value returned by getAtomFromString() for a non-null atom. */
        static const Atom_t ATOM_LOWER_BOUND = 20000;

        /** Null atom. Corresponds to the empty string. */
        static const Atom_t NULL_ATOM = 0;

        /** Constructor.
            Makes an empty table. */
        AtomTable();

        /** Destructor. */
        ~AtomTable();

        /** Get atom, given a C string.
            \param str String to look up/add
            \return atom */
        Atom_t getAtomFromString(const char* str);

        /** Get atom, given a string.
            \param str String to look up/add
            \return atom */
        Atom_t getAtomFromString(const String_t& str);

        /** Get string, given an atom.
            \param atom Atom to look up
            \return associated string, empty if atom does not correspond to an atom we produced */
        String_t getStringFromAtom(Atom_t atom) const;

        /** Get atom, given a C string, but does not create it.
            This function can be used as optimisation.
            \param str String to look up/add
            \return atom, NULL_ATOM if it does not exist */
        Atom_t getAtomFromStringNC(const char* str) const;

        /** Get atom, given a string, but does not create it.
            This function can be used as optimisation.
            \param str String to look up/add
            \return atom, NULL_ATOM if it does not exist */
        Atom_t getAtomFromStringNC(const String_t& str) const;

        /** Clear table.
            Forgets all mappings. */
        void clear();

        /** Check validity of atom.
            \param atom Atom to check
            \return true if atom corresponds to a nonempty string */
        bool isAtom(Atom_t atom) const;

     private:
        /** Manifest constants. */
        enum {
            ATOMS_PER_CHUNK = 128,      /** Atoms per memory chunk. */
            ATOM_HASH       = 128       /** Number of hash-table slots. */
        };

        struct AtomChunk {
            /** All atoms of this chunk in one block */
            afl::base::GrowableBytes_t data;
            /** Positions of each atom */
            size_t pos[ATOMS_PER_CHUNK];
            size_t length[ATOMS_PER_CHUNK];
            /** Hash links for each atom */
            Atom_t hashNext[ATOMS_PER_CHUNK];
        };

        afl::container::PtrVector<AtomChunk> m_entries;
        Atom_t m_hashFirst[ATOM_HASH];
        Atom_t m_nextAtom;

        afl::base::ConstBytes_t atomStrLookup(Atom_t atom) const;
        Atom_t atomLookup(afl::base::ConstBytes_t a, bool add);
    };

}

#endif
