/**
  *  \file util/keymaptable.hpp
  */
#ifndef C2NG_UTIL_KEYMAPTABLE_HPP
#define C2NG_UTIL_KEYMAPTABLE_HPP

#include "util/keymap.hpp"
#include "afl/container/ptrvector.hpp"
#include "afl/base/signal.hpp"

namespace util {

    class KeymapTable {
     public:
        KeymapTable();
        ~KeymapTable();

        KeymapRef_t getKeymapByName(String_t name) const;
        KeymapRef_t createKeymap(String_t name);

        void notifyListeners();

        afl::base::Signal<void()> sig_keymapChange;

     private:
        typedef afl::container::PtrVector<Keymap> Vector_t;
        Vector_t m_keymaps;
    };

}

#endif
