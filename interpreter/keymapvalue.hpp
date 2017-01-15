/**
  *  \file interpreter/keymapvalue.hpp
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
        the KeymapValue will be dropped and the user will not be able to access it. */
    class KeymapValue : public BaseValue {
     public:
        KeymapValue(util::KeymapRef_t keymap);
        ~KeymapValue();
        util::KeymapRef_t getKeymap() const;

        String_t toString(bool readable) const;
        virtual void store(TagNode& out, afl::io::DataSink& aux, afl::charset::Charset& cs, SaveContext& ctx) const;
        virtual KeymapValue* clone() const;

     private:
        util::KeymapRef_t m_keymap;
    };

    KeymapValue* makeKeymapValue(util::KeymapRef_t km);

}

#endif
