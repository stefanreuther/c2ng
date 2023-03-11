/**
  *  \file util/keymaptable.hpp
  *  \brief Class util::KeymapTable
  */
#ifndef C2NG_UTIL_KEYMAPTABLE_HPP
#define C2NG_UTIL_KEYMAPTABLE_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "util/keymap.hpp"

namespace util {

    /** Keymap table.
        Contains and owns a list of Keymap objects. */
    class KeymapTable {
     public:
        /** Constructor.
            Makes an empty KeymapTable. */
        KeymapTable();

        /** Destructor. */
        ~KeymapTable();

        /** Get keymap by name.
            @param name Name to find (case-sensitive, upper-case)
            @return Found keymap, 0 if it does not exist. */
        KeymapRef_t getKeymapByName(String_t name) const;

        /** Create keymap.
            @param name Name to create (case-sensitive, upper-case)
            @return New keymap.
            @throw std::runtime_error keymap by this name already exists */
        KeymapRef_t createKeymap(String_t name);

        /** Get number of keymaps.
            @return number */
        size_t getNumKeymaps() const;

        /** Get keymap, given an index.
            @param index Index [0,getNumKeymaps())
            @return keymap, null if index is out of range */
        KeymapRef_t getKeymapByIndex(size_t index) const;

        /** Notify listeners.
            Checks all keymaps for changes (Keymap::isChanged()) and resets those flags.
            If any keymap was changed, raises sig_keymapChange. */
        void notifyListeners();

        /** Signal: keymap changed.
            See notifyListeners(). */
        afl::base::Signal<void()> sig_keymapChange;

     private:
        typedef afl::container::PtrVector<Keymap> Vector_t;
        Vector_t m_keymaps;
    };

}

#endif
