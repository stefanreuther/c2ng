/**
  *  \file u/t_util_key.cpp
  *  \brief Test for util::Key
  */

#include "util/key.hpp"

#include "t_util.hpp"

/** Test parseKey. */
void
TestUtilKey::testParse()
{
    // ex IntKeymapTestSuite::testParse
    util::Key_t key;
    TS_ASSERT(util::parseKey("a", key));           TS_ASSERT_EQUALS(key, util::Key_t('a'));
    TS_ASSERT(util::parseKey("s-a", key));         TS_ASSERT_EQUALS(key, util::Key_t('A'));
    TS_ASSERT(util::parseKey("A", key));           TS_ASSERT_EQUALS(key, util::Key_t('a'));
    TS_ASSERT(util::parseKey("s-A", key));         TS_ASSERT_EQUALS(key, util::Key_t('A'));
    TS_ASSERT(util::parseKey("shift-A", key));     TS_ASSERT_EQUALS(key, util::Key_t('A'));
    TS_ASSERT(util::parseKey("c-A", key));         TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + 'a'));
    TS_ASSERT(util::parseKey("a-A", key));         TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Alt + 'a'));
    TS_ASSERT(util::parseKey("c-a-A", key));       TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    TS_ASSERT(util::parseKey("a-c-A", key));       TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    TS_ASSERT(util::parseKey("alt-ctrl-A", key));  TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    TS_ASSERT(util::parseKey("alt-c-A", key));     TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    TS_ASSERT(util::parseKey("A-A-A", key));       TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Alt + 'a'));

    TS_ASSERT(util::parseKey("1", key));           TS_ASSERT_EQUALS(key, util::Key_t('1'));
    TS_ASSERT(util::parseKey("s-1", key));         TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Shift + '1'));

    TS_ASSERT(util::parseKey("f1", key));          TS_ASSERT_EQUALS(key, util::Key_t(util::Key_F1));
    TS_ASSERT(util::parseKey("c-f1", key));        TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::Key_F1));
    TS_ASSERT(util::parseKey("s-f1", key));        TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Shift + util::Key_F1));
    TS_ASSERT(util::parseKey("m-f1", key));        TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Meta + util::Key_F1));
    TS_ASSERT(util::parseKey("a-f1", key));        TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Alt + util::Key_F1));
    TS_ASSERT(util::parseKey("c-a-f1", key));      TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + util::Key_F1));
    TS_ASSERT(util::parseKey("s-a-f1", key));      TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Shift + util::KeyMod_Alt + util::Key_F1));

    TS_ASSERT(util::parseKey("wheelup", key));     TS_ASSERT_EQUALS(key, util::Key_t(util::Key_WheelUp));
    TS_ASSERT(util::parseKey("WheelUp", key));     TS_ASSERT_EQUALS(key, util::Key_t(util::Key_WheelUp));
    TS_ASSERT(util::parseKey("BS", key));          TS_ASSERT_EQUALS(key, util::Key_t(util::Key_Backspace));
    TS_ASSERT(util::parseKey("ctrl-h", key));      TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + 'h'));
    TS_ASSERT(util::parseKey("ctrl-m", key));      TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + 'm'));
    TS_ASSERT(util::parseKey("ctrl-j", key));      TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + 'j'));
    TS_ASSERT(util::parseKey("enter", key));       TS_ASSERT_EQUALS(key, util::Key_t(util::Key_Return));
    TS_ASSERT(util::parseKey("ret", key));         TS_ASSERT_EQUALS(key, util::Key_t(util::Key_Return));
    TS_ASSERT(util::parseKey("ctrl-enter", key));  TS_ASSERT_EQUALS(key, util::Key_t(util::KeyMod_Ctrl + util::Key_Return));

    // Unicode
    TS_ASSERT(util::parseKey("\xC2\xA0", key));    TS_ASSERT_EQUALS(key, util::Key_t(0xA0));

    // Cannot test Unicode boundary case. These need hand-crafted out-of-range UTF-8 which Key by default does not parse.
    // TS_ASSERT_EQUALS(util::Key_FirstSpecial, 0x1000000U);
    // TS_ASSERT(util::parseKey("\xF8\xBF\xBF\xBF\xBF", key)); TS_ASSERT_EQUALS(key, util::Key_t(0x0FFFFFF));
    // TS_ASSERT(!util::parseKey("\xF9\x80\x80\x80\x80", key));

    // Escapes
    TS_ASSERT(util::parseKey("#$10", key)); TS_ASSERT_EQUALS(key, util::Key_t(0x10));
    TS_ASSERT(util::parseKey("#$F000", key)); TS_ASSERT_EQUALS(key, util::Key_t(0xF000));

    // Invalids
    TS_ASSERT(!util::parseKey("#$F0001", key));
    TS_ASSERT(!util::parseKey("xy", key));
    TS_ASSERT(!util::parseKey("re", key));
    TS_ASSERT(!util::parseKey("rett", key));
}

/** Test formatKey. */
void
TestUtilKey::testFormat()
{
    // ex IntKeymapTestSuite::testFormat
    TS_ASSERT_EQUALS(util::formatKey('a'),                                                  "A");
    TS_ASSERT_EQUALS(util::formatKey('A'),                                                  "SHIFT-A");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + 'a'),                              "CTRL-A");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Alt + 'a'),                               "ALT-A");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'),           "CTRL-ALT-A");
    TS_ASSERT_EQUALS(util::formatKey('1'),                                                  "1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Shift + '1'),                             "SHIFT-1");
    TS_ASSERT_EQUALS(util::formatKey(util::Key_F1),                                         "F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + util::Key_F1),                     "CTRL-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Shift + util::Key_F1),                    "SHIFT-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Meta + util::Key_F1),                     "META-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Alt + util::Key_F1),                      "ALT-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + util::KeyMod_Alt + util::Key_F1),  "CTRL-ALT-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Shift + util::KeyMod_Alt + util::Key_F1), "SHIFT-ALT-F1");
    TS_ASSERT_EQUALS(util::formatKey(util::Key_WheelUp),                                    "WHEELUP");
    TS_ASSERT_EQUALS(util::formatKey(util::Key_Backspace),                                  "BS");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + 'h'),                              "CTRL-H");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + 'm'),                              "CTRL-M");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + 'j'),                              "CTRL-J");
    TS_ASSERT_EQUALS(util::formatKey(util::Key_Return),                                     "RET");
    TS_ASSERT_EQUALS(util::formatKey(util::KeyMod_Ctrl + util::Key_Return),                 "CTRL-RET");

    TS_ASSERT_EQUALS(util::formatKey(util::Key_t(0xF000)),                                  "\xEF\x80\x80");  // Unicode character!
    TS_ASSERT_EQUALS(util::formatKey(util::Key_t(127)),                                     "#$007F");        // this is the only one that triggers the #$ case
}

void
TestUtilKey::testUnique()
{
    // Test uniqueness of key assignments.
    switch (0) {
     case util::Key_F1:
     case util::Key_F2:
     case util::Key_F3:
     case util::Key_F4:
     case util::Key_F5:
     case util::Key_F6:
     case util::Key_F7:
     case util::Key_F8:
     case util::Key_F9:
     case util::Key_F10:
     case util::Key_F11:
     case util::Key_F12:
     case util::Key_F13:
     case util::Key_F14:
     case util::Key_F15:
     case util::Key_F16:
     case util::Key_F17:
     case util::Key_F18:
     case util::Key_F19:
     case util::Key_F20:

        // Movement
     case util::Key_Up:
     case util::Key_Down:
     case util::Key_Left:
     case util::Key_Right:
     case util::Key_Home:
     case util::Key_End:
     case util::Key_PgUp:
     case util::Key_PgDn:

        // Input
     case util::Key_Tab:
     case util::Key_Backspace:
     case util::Key_Delete:
     case util::Key_Insert:
     case util::Key_Return:

        // Modifiers
     case util::Key_CapsLock:
     case util::Key_Compose:
     case util::Key_ScrollLock:
     case util::Key_NumLock:
     case util::Key_LAlt:
     case util::Key_RAlt:
     case util::Key_LCtrl:
     case util::Key_RCtrl:
     case util::Key_LMeta:
     case util::Key_RMeta:
     case util::Key_LSuper:
     case util::Key_RSuper:
     case util::Key_LShift:
     case util::Key_RShift:

        // Numpad specials
     case util::Key_Num0:
     case util::Key_Num1:
     case util::Key_Num2:
     case util::Key_Num3:
     case util::Key_Num4:
     case util::Key_Num5:
     case util::Key_Num6:
     case util::Key_Num7:
     case util::Key_Num8:
     case util::Key_Num9:
     case util::Key_NumPlus:
     case util::Key_NumMinus:
     case util::Key_NumMultiply:
     case util::Key_NumDivide:
     case util::Key_NumPeriod:
     case util::Key_NumEnter:
     case util::Key_NumEquals:

        // Misc
     case util::Key_Escape:
     case util::Key_Print:
     case util::Key_Pause:
     case util::Key_Menu:

        // Special
     case util::Key_WheelUp:
     case util::Key_WheelDown:
     case util::Key_Quit:
     case util::Key_DoubleClick:

     case util::KeyMod_Mask:
     case util::KeyMod_Shift:
     case util::KeyMod_Ctrl:
     case util::KeyMod_Alt:
     case util::KeyMod_Meta:
        break;
    }
}

/** Test classifyKey. */
void
TestUtilKey::testClassify()
{
    // Some regular keys
    TS_ASSERT_EQUALS(util::classifyKey('a'), util::NormalKey);
    TS_ASSERT_EQUALS(util::classifyKey(' '), util::NormalKey);
    TS_ASSERT_EQUALS(util::classifyKey(0x4000), util::NormalKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_PgDn), util::NormalKey);

    // Shifts
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_LShift), util::ModifierKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_LAlt), util::ModifierKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_LCtrl), util::ModifierKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_RShift), util::ModifierKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_RAlt), util::ModifierKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_RCtrl), util::ModifierKey);

    // Virtual
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_Quit), util::VirtualKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_WheelDown), util::VirtualKey);
    TS_ASSERT_EQUALS(util::classifyKey(util::Key_WheelUp), util::VirtualKey);
}

