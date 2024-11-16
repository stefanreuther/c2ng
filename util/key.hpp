/**
  *  \file util/key.hpp
  *  \brief Key Codes
  *
  *  This is our own keymap table, independant of whatever the GUI library is using.
  *  (PCC2 was using SDL keys, causing the SDL headers to leak into everything.)
  *
  *  Keys do not have any particularily stable format other than that printable
  *  keys have their Unicode codepoint as code.
  *
  *  Some other "one-shot" input events are also mapped to keys (mouse wheel, close request).
  */
#ifndef C2NG_UTIL_KEY_HPP
#define C2NG_UTIL_KEY_HPP

#include "afl/base/types.hpp"
#include "afl/string/string.hpp"

namespace util {

    /** Typedef for a key code (Key_XXX) with modifiers (KeyMod_XXX). */
    typedef uint32_t Key_t;

    // Value below Key_FirstSpecial: Unicode
    const Key_t Key_FirstSpecial = 0x1000000;
    const Key_t Key_Mask         = 0x1FFFFFF;

    // Function keys
    const Key_t Key_F1             = Key_FirstSpecial;
    const Key_t Key_F2             = Key_F1+1;
    const Key_t Key_F3             = Key_F1+2;
    const Key_t Key_F4             = Key_F1+3;
    const Key_t Key_F5             = Key_F1+4;
    const Key_t Key_F6             = Key_F1+5;
    const Key_t Key_F7             = Key_F1+6;
    const Key_t Key_F8             = Key_F1+7;
    const Key_t Key_F9             = Key_F1+8;
    const Key_t Key_F10            = Key_F1+9;
    const Key_t Key_F11            = Key_F1+10;
    const Key_t Key_F12            = Key_F1+11;
    const Key_t Key_F13            = Key_F1+12;
    const Key_t Key_F14            = Key_F1+13;
    const Key_t Key_F15            = Key_F1+14;
    const Key_t Key_F16            = Key_F1+15;
    const Key_t Key_F17            = Key_F1+16;
    const Key_t Key_F18            = Key_F1+17;
    const Key_t Key_F19            = Key_F1+18;
    const Key_t Key_F20            = Key_F1+19;

    // Movement
    const Key_t Key_Up             = Key_FirstSpecial+0x100;
    const Key_t Key_Down           = Key_Up+1;
    const Key_t Key_Left           = Key_Up+2;
    const Key_t Key_Right          = Key_Up+3;
    const Key_t Key_Home           = Key_Up+4;
    const Key_t Key_End            = Key_Up+5;
    const Key_t Key_PgUp           = Key_Up+6;
    const Key_t Key_PgDn           = Key_Up+7;

    // Input
    const Key_t Key_Tab            = Key_Up+8;
    const Key_t Key_Backspace      = Key_Tab+1;
    const Key_t Key_Delete         = Key_Tab+2;
    const Key_t Key_Insert         = Key_Tab+3;
    const Key_t Key_Return         = Key_Tab+4;

    // Modifiers
    const Key_t Key_CapsLock       = Key_Tab+5;
    const Key_t Key_Compose        = Key_CapsLock+1;
    const Key_t Key_ScrollLock     = Key_CapsLock+2;
    const Key_t Key_NumLock        = Key_CapsLock+3;
    const Key_t Key_LAlt           = Key_CapsLock+4;
    const Key_t Key_RAlt           = Key_CapsLock+5;
    const Key_t Key_LCtrl          = Key_CapsLock+6;
    const Key_t Key_RCtrl          = Key_CapsLock+7;
    const Key_t Key_LMeta          = Key_CapsLock+8;
    const Key_t Key_RMeta          = Key_CapsLock+9;
    const Key_t Key_LSuper         = Key_CapsLock+10;
    const Key_t Key_RSuper         = Key_CapsLock+11;
    const Key_t Key_LShift         = Key_CapsLock+12;
    const Key_t Key_RShift         = Key_CapsLock+13;
    const Key_t KeyRange_FirstModifier = Key_CapsLock;
    const Key_t KeyRange_LastModifier  = Key_RShift;

    // Numpad specials
    const Key_t Key_Num0           = Key_CapsLock+14;
    const Key_t Key_Num1           = Key_Num0+1;
    const Key_t Key_Num2           = Key_Num0+2;
    const Key_t Key_Num3           = Key_Num0+3;
    const Key_t Key_Num4           = Key_Num0+4;
    const Key_t Key_Num5           = Key_Num0+5;
    const Key_t Key_Num6           = Key_Num0+6;
    const Key_t Key_Num7           = Key_Num0+7;
    const Key_t Key_Num8           = Key_Num0+8;
    const Key_t Key_Num9           = Key_Num0+9;
    const Key_t Key_NumPlus        = Key_Num0+10;
    const Key_t Key_NumMinus       = Key_Num0+11;
    const Key_t Key_NumMultiply    = Key_Num0+12;
    const Key_t Key_NumDivide      = Key_Num0+13;
    const Key_t Key_NumPeriod      = Key_Num0+14;
    const Key_t Key_NumEnter       = Key_Num0+15;
    const Key_t Key_NumEquals      = Key_Num0+16;

    // Misc
    const Key_t Key_Escape         = Key_Num0+17;
    const Key_t Key_Print          = Key_Escape+1;
    const Key_t Key_Pause          = Key_Escape+2;
    const Key_t Key_Menu           = Key_Escape+3;

    // Special
    const Key_t Key_WheelUp        = Key_Escape+4;
    const Key_t Key_WheelDown      = Key_WheelUp+1;
    const Key_t Key_Quit           = Key_WheelUp+2;
    const Key_t Key_DoubleClick    = Key_WheelUp+3;
    const Key_t KeyRange_FirstVirtual = Key_WheelUp;
    const Key_t KeyRange_LastVirtual  = Key_DoubleClick;

    // Unmapped:
    //   4 SDLK_SPACE
    //   1 SDLK_MODE


    const Key_t KeyMod_Mask        = static_cast<Key_t>(0xFE000000);
    const Key_t KeyMod_Shift       = 0x02000000;
    const Key_t KeyMod_Ctrl        = 0x04000000;
    const Key_t KeyMod_Alt         = 0x08000000;
    const Key_t KeyMod_Meta        = 0x10000000;

    /** Parse string into key.
        \param str [in] String given by user
        \param result [out] Result
        \return true if parsed successfully, false on error */
    bool parseKey(String_t str, Key_t& result);

    /** Format key code into string.
        \param key Keycode
        \return Key string so that parseKey(return) == key */
    String_t formatKey(Key_t key);


    /** Class of a key.
        Distringuishes "real" keypresses from modifiers and virtual keys. */
    enum KeyClass {
        NormalKey,    //< A normal key which is perceived as a "key press" by normal users. Includes printing keys and function keys.
        ModifierKey,  //< A key which is perceived as a "modifier key". Those are usually used as parts of a key combination (Alt, Ctrl, ...).
        VirtualKey    //< A virtual key (event mapped to key).
    };

    /** Classify a key.
        \param key Keycode
        \return class */
    KeyClass classifyKey(Key_t key);

}

#endif
