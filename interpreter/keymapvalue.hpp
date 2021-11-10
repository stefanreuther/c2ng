/**
  *  \file interpreter/keymapvalue.hpp
  *  \brief Class interpreter::KeymapValue
  */
#ifndef C2NG_INTERPRETER_KEYMAPVALUE_HPP
#define C2NG_INTERPRETER_KEYMAPVALUE_HPP

#include "util/keymap.hpp"
#include "interpreter/basevalue.hpp"

namespace interpreter {

    /** Keymap value.
        Script code uses these (temporarily) to refer to keymaps.
        These values are not exposed to the user.
        'Bind' and 'CreateKeymap' commands are compiled into fixed command sequences
        that convert a string into a keymap using ukeylookup or ukeycreate;
        after operating on that value using bkeyaddparent, tkeyadd,
        the KeymapValue will be dropped and the user will not be able to access it.

        A KeymapValue always refers to a keymap.
        If a function producing a KeymapValue has to return a null value,
        it returns null (=empty), not a KeymapValue containing a null pointer.
        See makeKeymapValue(). */
    class KeymapValue : public BaseValue {
     public:
        /** Constructor.
            \param keymap Keymap. Must not be null. */
        KeymapValue(util::KeymapRef_t keymap);

        /** Destructor. */
        ~KeymapValue();

        /** Get keymap.
            \return keymap */
        util::KeymapRef_t getKeymap() const;

        // BaseValue:
        String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, SaveContext& ctx) const;
        virtual KeymapValue* clone() const;

     private:
        util::KeymapRef_t m_keymap;
    };

    /** Construct keymap value.
        \param km Keymap
        \return Newly-allocated KeymapValue if km is non-null; null otherwise */
    KeymapValue* makeKeymapValue(util::KeymapRef_t km);

}

#endif
