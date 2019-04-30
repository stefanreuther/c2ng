/**
  *  \file util/keymap.hpp
  *  \brief Class util::Keymap
  */
#ifndef C2NG_UTIL_KEYMAP_HPP
#define C2NG_UTIL_KEYMAP_HPP

#include <set>
#include "util/key.hpp"
#include "util/atomtable.hpp"
#include "afl/base/signal.hpp"

namespace util {

    class Keymap;
    class KeymapInformation;

    /** Keymap reference.
        User code dealing with keymaps uses objects of type KeymapRef.
        This might one day be a smart pointer.
        Currently, all live Keymap objects are managed in a PtrVector, so there are no lifetime issues. */
    typedef Keymap* KeymapRef_t;

    /** Set of keys. */
    typedef std::set<Key_t> KeySet_t;

    /** Keymap.
        A keymap maps keys to command/condition pairs.
        Keys are specified as integers.
        Commands and conditions are also specified as integers, which must be atoms that map to a statement or expression, respectively.

        Conditions will be used to detect whether buttons should be enabled or disabled.
        Space for them has therefore been reserved in the data structure.

        A keymap can have one or many parents, thus building an acyclic directed graph.
        Keys not found in a keymap are looked up in all its parents, from first to last. */
    class Keymap {
     public:
        /** Constructor.
            \param name Name of this keymap */
        explicit Keymap(const String_t& name);

        /** Destructor. */
        ~Keymap();

        /** Add key to this keymap.
            \param key         Key to add
            \param command     Command (=atom)
            \param condition   Condition (=atom) */
        void addKey(Key_t key, Atom_t command, Atom_t condition);

        /** Add parent to this keymap.
            \param km Other keymap
            \throw std::runtime_error if km is already a parent of this one
            \change PCC2 throws IntError. We cannot do that because our keymaps are not interpreter-only. */
        void addParent(Keymap& km);

        /** Given a key, look up its command.
            \param key Key to look up
            \return found command (atom), 0 if none */
        Atom_t lookupCommand(Key_t key) const;

        /** Given a key, look up its command and place of definition.
            \param key   [in] Key to look up
            \param where [out] Keymap in which this command is bound
            \return found command (atom), 0 if none */
        Atom_t lookupCommand(Key_t key, KeymapRef_t& where) const;

        /** Given a key, look up its condition.
            \param key Key to look up
            \return found condition (atom), 0 if none */
        Atom_t lookupCondition(Key_t key) const;

        /** Check for parent relation ship, recursively.
            \param km Keymap to find
            \return true iff km is equal to this, or a parent, grandparent, etc., recursively */
        bool hasParent(Keymap& km) const;

        /** Get name of this keymap.
            \return name */
        const String_t& getName() const;

        /** Get number of direct parents.
            \return number of direct parents */
        size_t getNumDirectParents() const;

        /** Get reference to a direct parent.
            \param index [0,getNumDirectParents()) Index
            \return parent; 0 if index is out of range */
        KeymapRef_t getDirectParent(size_t index) const;

        /** Enumerate keys.
            Adds to \c keys the set of all keys bound by this keymap and its parents.
            \param keys [in/out] Keys */
        void enumKeys(KeySet_t& keys) const;

        /** Mark this keymap changed.
            \param state New state. Default value is true to mark this keymap changed for the next notifyListeners iteration. */
        void markChanged(bool state = true);

        /** Check whether this keymap was changed.
            \return true if keymap was changed */
        bool isChanged() const;

        /** Describe keymap structure.
            \param result   [out] Result
            \param maxDepth [in]  Maximum recursion depth */
        void describe(KeymapInformation& result, size_t maxDepth) const;

     private:
        struct Entry {
            Key_t key;
            Atom_t command;
            Atom_t condition;
            Entry(Key_t key, Atom_t command, Atom_t condition)
                : key(key), command(command), condition(condition)
                { }
        };
        std::vector<Keymap*> m_parents;                ///< All this keymap's parents.
        std::vector<Entry> m_keys;                     ///< All this keymap's keys.
        const String_t m_name;                         ///< Name of this keymap.
        bool m_changed;

        const Entry* lookup(Key_t key, KeymapRef_t* where) const;
    };

}

// Get name of this keymap.
inline const String_t&
util::Keymap::getName() const
{
    return m_name;
}

// Get number of direct parents.
inline size_t
util::Keymap::getNumDirectParents() const
{
    return m_parents.size();
}

// Get reference to a direct parent.
inline util::KeymapRef_t
util::Keymap::getDirectParent(size_t index) const
{
    return index < m_parents.size()
         ? m_parents[index]
         : 0;
}

#endif
