/**
  *  \file util/key.cpp
  */

#include <cstring>
#include <cstdlib>
#include "util/key.hpp"
#include "afl/base/countof.hpp"
#include "afl/charset/utf8.hpp"
#include "afl/string/format.hpp"

namespace {
    struct KeyPair {
        const char* name;
        util::Key_t value;
    };

    const KeyPair key_mods[] = {
        { "CTRL-",     util::KeyMod_Ctrl },
        { "C-",        util::KeyMod_Ctrl },
        { "SHIFT-",    util::KeyMod_Shift },
        { "S-",        util::KeyMod_Shift },
        { "ALT-",      util::KeyMod_Alt },
        { "A-",        util::KeyMod_Alt },
        { "META-",     util::KeyMod_Meta },
        { "M-",        util::KeyMod_Meta },
    };

    const KeyPair key_syms[] = {
        // Function keys
        { "F1",        util::Key_F1 },
        { "F2",        util::Key_F2 },
        { "F3",        util::Key_F3 },
        { "F4",        util::Key_F4 },
        { "F5",        util::Key_F5 },
        { "F6",        util::Key_F6 },
        { "F7",        util::Key_F7 },
        { "F8",        util::Key_F8 },
        { "F9",        util::Key_F9 },
        { "F10",       util::Key_F10 },
        { "F11",       util::Key_F11 },
        { "F12",       util::Key_F12 },
        { "F13",       util::Key_F13 },
        { "F14",       util::Key_F14 },
        { "F15",       util::Key_F15 },
        { "F16",       util::Key_F16 },
        { "F17",       util::Key_F17 },
        { "F18",       util::Key_F18 },
        { "F19",       util::Key_F19 },
        { "F20",       util::Key_F20 },

        // Movement keys
        { "UP",        util::Key_Up },
        { "DOWN",      util::Key_Down },
        { "LEFT",      util::Key_Left },
        { "RIGHT",     util::Key_Right },
        { "HOME",      util::Key_Home },
        { "END",       util::Key_End },
        { "PGUP",      util::Key_PgUp },
        { "PGDN",      util::Key_PgDn },

        // Input keys
        { "TAB",       util::Key_Tab },
        { "BS",        util::Key_Backspace },
        { "BACKSPACE", util::Key_Backspace },
        { "DEL",       util::Key_Delete },
        { "INS",       util::Key_Insert },
        { "RET",       util::Key_Return },
        { "ENTER",     util::Key_Return },

        // Modifiers not mapped
        //             util::Key_CapsLock
        //             util::Key_Compose
        //             util::Key_ScrollLock
        //             util::Key_NumLock
        //             util::Key_LAlt
        //             util::Key_RAlt
        //             util::Key_LCtrl
        //             util::Key_RCtrl
        //             util::Key_LMeta
        //             util::Key_RMeta
        //             util::Key_LSuper
        //             util::Key_RSuper
        //             util::Key_LShift
        //             util::Key_RShift

        // Numpad specials
        //             util::Key_Num0
        //             util::Key_Num1
        //             util::Key_Num2
        //             util::Key_Num3
        //             util::Key_Num4
        { "NUM5",      util::Key_Num5 },
        //             util::Key_Num6
        //             util::Key_Num7
        //             util::Key_Num8
        //             util::Key_Num9
        { "NUM+",      util::Key_NumPlus },
        { "NUM-",      util::Key_NumMinus },
        { "NUM*",      util::Key_NumMultiply },
        { "NUM/",      util::Key_NumDivide },
        //             util::Key_NumPeriod
        //             util::Key_NumEnter: PCC2 maps this to Key_Return, PCC1 maps it to "LFD"/"NUMRET". FIXME: determine best way.
        //             util::Key_NumEquals

        // Misc
        { "ESC",       util::Key_Escape },
        { "PRINT",     util::Key_Print },
        { "PAUSE",     util::Key_Pause },
        //             util::Key_Menu: FIXME: PCC2 does not map this

        // Special
        { "WHEELUP",   util::Key_WheelUp },
        { "WHEELDN",   util::Key_WheelDown },
        { "QUIT",      util::Key_Quit },
        { "DBLCLICK",  util::Key_DoubleClick },

        // Printable
        { "SPACE",     ' ' },
        { "SPC",       ' ' },
    };
}


bool
util::parseKey(String_t str, Key_t& result)
{
    // ex int/keymap.h:parseKey
    // ex keymaps.pas:ParseKeycode
    // Upcase it
    str = afl::string::strUCase(str);

    // Parse modifiers
    result = 0;
    while (1) {
        bool did = false;
        for (size_t i = 0; i < countof(key_mods); ++i) {
            size_t len = std::strlen(key_mods[i].name);
            if (str.compare(0, len, key_mods[i].name, len) == 0) {
                did = true;
                result |= key_mods[i].value;
                str.erase(0, len);
            }
        }
        if (!did) {
            break;
        }
    }

    // Parse key
    if (afl::charset::Utf8().length(str) == 1) {
        uint32_t uc = afl::charset::Utf8().charAt(str, 0);
        if (uc >= 'A' && uc <= 'Z') {
            if (result & KeyMod_Shift) {
                // Shifted alphabetical. Because this already is
                // upper-case, just remove the shift.
                result &= ~KeyMod_Shift;
            } else {
                // Non-shifted. Make it lower-case.
                uc += 'a' - 'A';
            }
        }
        if (uc >= Key_FirstSpecial) {
            return false;
        }
        result |= uc;
    } else {
        bool found = false;
        for (size_t i = 0; i < countof(key_syms); ++i) {
            if (str == key_syms[i].name) {
                result |= key_syms[i].value;
                found = true;
                break;
            }
        }
        if (!found && str.size() > 2 && str[0] == '#' && str[1] == '$' && str.size() <= 6) {
            // Up to four hex digits
            const char* p = str.c_str()+2;
            char* pout;
            result |= std::strtoul(p, &pout, 16);
            if (*pout == 0) {
                found = true;
            }
        }
        if (!found) {
            return false;
        }
    }

    return true;
}

String_t
util::formatKey(Key_t key)
{
    // ex int/keymap.h:formatKey
    String_t result;

    // Modifiers
    for (size_t i = 0; i < countof(key_mods); ++i) {
        if (key & key_mods[i].value) {
            result += key_mods[i].name;
            key &= ~key_mods[i].value;
        }
    }

    // Key
    bool did = false;
    for (size_t i = 0; i < countof(key_syms); ++i) {
        if (key == key_syms[i].value) {
            result += key_syms[i].name;
            did = true;
            break;
        }
    }

    if (!did) {
        if (key >= 'A' && key <= 'Z') {
            // Upper-case letter
            result += "SHIFT-";
            result += char(key);
        } else if (key >= 'a' && key <= 'z') {
            // Lower-case letter
            result += char(key - ('a'-'A'));
        } else if (key >= ' ' && key < Key_FirstSpecial && key != 127) {
            // Key from game character set
            afl::charset::Utf8().append(result, key);
        } else {
            result += afl::string::Format("#$%04X", key);
        }
    }

    return result;
}

util::KeyClass
util::classifyKey(Key_t key)
{
    if (key >= KeyRange_FirstModifier && key <= KeyRange_LastModifier) {
        return ModifierKey;
    } else if (key >= KeyRange_FirstVirtual && key <= KeyRange_LastVirtual) {
        return VirtualKey;
    } else {
        return NormalKey;
    }
}
