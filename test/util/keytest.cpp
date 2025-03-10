/**
  *  \file test/util/keytest.cpp
  *  \brief Test for util::Key
  */

#include "util/key.hpp"
#include "afl/test/testrunner.hpp"

/** Test parseKey. */
AFL_TEST("util.Key:parseKey", a)
{
    // ex IntKeymapTestSuite::testParse
    util::Key_t key;
    a.check("01", util::parseKey("a", key));           a.checkEqual("01", key, util::Key_t('a'));
    a.check("02", util::parseKey("s-a", key));         a.checkEqual("02", key, util::Key_t('A'));
    a.check("03", util::parseKey("A", key));           a.checkEqual("03", key, util::Key_t('a'));
    a.check("04", util::parseKey("s-A", key));         a.checkEqual("04", key, util::Key_t('A'));
    a.check("05", util::parseKey("shift-A", key));     a.checkEqual("05", key, util::Key_t('A'));
    a.check("06", util::parseKey("c-A", key));         a.checkEqual("06", key, util::Key_t(util::KeyMod_Ctrl + 'a'));
    a.check("07", util::parseKey("a-A", key));         a.checkEqual("07", key, util::Key_t(util::KeyMod_Alt + 'a'));
    a.check("08", util::parseKey("c-a-A", key));       a.checkEqual("08", key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    a.check("09", util::parseKey("a-c-A", key));       a.checkEqual("09", key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    a.check("10", util::parseKey("alt-ctrl-A", key));  a.checkEqual("10", key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    a.check("11", util::parseKey("alt-c-A", key));     a.checkEqual("11", key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'));
    a.check("12", util::parseKey("A-A-A", key));       a.checkEqual("12", key, util::Key_t(util::KeyMod_Alt + 'a'));

    a.check("21", util::parseKey("1", key));           a.checkEqual("21", key, util::Key_t('1'));
    a.check("22", util::parseKey("s-1", key));         a.checkEqual("22", key, util::Key_t(util::KeyMod_Shift + '1'));

    a.check("31", util::parseKey("f1", key));          a.checkEqual("31", key, util::Key_t(util::Key_F1));
    a.check("32", util::parseKey("c-f1", key));        a.checkEqual("32", key, util::Key_t(util::KeyMod_Ctrl + util::Key_F1));
    a.check("33", util::parseKey("s-f1", key));        a.checkEqual("33", key, util::Key_t(util::KeyMod_Shift + util::Key_F1));
    a.check("34", util::parseKey("m-f1", key));        a.checkEqual("34", key, util::Key_t(util::KeyMod_Meta + util::Key_F1));
    a.check("35", util::parseKey("a-f1", key));        a.checkEqual("35", key, util::Key_t(util::KeyMod_Alt + util::Key_F1));
    a.check("36", util::parseKey("c-a-f1", key));      a.checkEqual("36", key, util::Key_t(util::KeyMod_Ctrl + util::KeyMod_Alt + util::Key_F1));
    a.check("37", util::parseKey("s-a-f1", key));      a.checkEqual("37", key, util::Key_t(util::KeyMod_Shift + util::KeyMod_Alt + util::Key_F1));

    a.check("41", util::parseKey("wheelup", key));     a.checkEqual("41", key, util::Key_t(util::Key_WheelUp));
    a.check("42", util::parseKey("WheelUp", key));     a.checkEqual("42", key, util::Key_t(util::Key_WheelUp));
    a.check("43", util::parseKey("BS", key));          a.checkEqual("43", key, util::Key_t(util::Key_Backspace));
    a.check("44", util::parseKey("ctrl-h", key));      a.checkEqual("44", key, util::Key_t(util::KeyMod_Ctrl + 'h'));
    a.check("45", util::parseKey("ctrl-m", key));      a.checkEqual("45", key, util::Key_t(util::KeyMod_Ctrl + 'm'));
    a.check("46", util::parseKey("ctrl-j", key));      a.checkEqual("46", key, util::Key_t(util::KeyMod_Ctrl + 'j'));
    a.check("47", util::parseKey("enter", key));       a.checkEqual("47", key, util::Key_t(util::Key_Return));
    a.check("48", util::parseKey("ret", key));         a.checkEqual("48", key, util::Key_t(util::Key_Return));
    a.check("49", util::parseKey("ctrl-enter", key));  a.checkEqual("49", key, util::Key_t(util::KeyMod_Ctrl + util::Key_Return));

    // Unicode
    a.check("51", util::parseKey("\xC2\xA0", key));    a.checkEqual("51", key, util::Key_t(0xA0));

    // Cannot test Unicode boundary case. These need hand-crafted out-of-range UTF-8 which Key by default does not parse.
    // a.checkEqual("61", util::Key_FirstSpecial, 0x1000000U);
    // a.check("62", util::parseKey("\xF8\xBF\xBF\xBF\xBF", key)); a.checkEqual("62", key, util::Key_t(0x0FFFFFF));
    // a.check("63", !util::parseKey("\xF9\x80\x80\x80\x80", key));

    // Escapes
    a.check("71", util::parseKey("#$10", key)); a.checkEqual("71", key, util::Key_t(0x10));
    a.check("72", util::parseKey("#$F000", key)); a.checkEqual("72", key, util::Key_t(0xF000));

    // Invalids
    a.check("81", !util::parseKey("#$F0001", key));
    a.check("82", !util::parseKey("xy", key));
    a.check("83", !util::parseKey("re", key));
    a.check("84", !util::parseKey("rett", key));
}

/** Test formatKey. */
AFL_TEST("util.Key:formatKey", a)
{
    // ex IntKeymapTestSuite::testFormat
    a.checkEqual("01", util::formatKey('a'),                                                  "A");
    a.checkEqual("02", util::formatKey('A'),                                                  "SHIFT-A");
    a.checkEqual("03", util::formatKey(util::KeyMod_Ctrl + 'a'),                              "CTRL-A");
    a.checkEqual("04", util::formatKey(util::KeyMod_Alt + 'a'),                               "ALT-A");
    a.checkEqual("05", util::formatKey(util::KeyMod_Ctrl + util::KeyMod_Alt + 'a'),           "CTRL-ALT-A");
    a.checkEqual("06", util::formatKey('1'),                                                  "1");
    a.checkEqual("07", util::formatKey(util::KeyMod_Shift + '1'),                             "SHIFT-1");
    a.checkEqual("08", util::formatKey(util::Key_F1),                                         "F1");
    a.checkEqual("09", util::formatKey(util::KeyMod_Ctrl + util::Key_F1),                     "CTRL-F1");
    a.checkEqual("10", util::formatKey(util::KeyMod_Shift + util::Key_F1),                    "SHIFT-F1");
    a.checkEqual("11", util::formatKey(util::KeyMod_Meta + util::Key_F1),                     "META-F1");
    a.checkEqual("12", util::formatKey(util::KeyMod_Alt + util::Key_F1),                      "ALT-F1");
    a.checkEqual("13", util::formatKey(util::KeyMod_Ctrl + util::KeyMod_Alt + util::Key_F1),  "CTRL-ALT-F1");
    a.checkEqual("14", util::formatKey(util::KeyMod_Shift + util::KeyMod_Alt + util::Key_F1), "SHIFT-ALT-F1");
    a.checkEqual("15", util::formatKey(util::Key_WheelUp),                                    "WHEELUP");
    a.checkEqual("16", util::formatKey(util::Key_Backspace),                                  "BS");
    a.checkEqual("17", util::formatKey(util::KeyMod_Ctrl + 'h'),                              "CTRL-H");
    a.checkEqual("18", util::formatKey(util::KeyMod_Ctrl + 'm'),                              "CTRL-M");
    a.checkEqual("19", util::formatKey(util::KeyMod_Ctrl + 'j'),                              "CTRL-J");
    a.checkEqual("20", util::formatKey(util::Key_Return),                                     "RET");
    a.checkEqual("21", util::formatKey(util::KeyMod_Ctrl + util::Key_Return),                 "CTRL-RET");

    a.checkEqual("31", util::formatKey(util::Key_t(0xF000)),                                  "\xEF\x80\x80");  // Unicode character!
    a.checkEqual("32", util::formatKey(util::Key_t(127)),                                     "#$007F");        // this is the only one that triggers the #$ case
}

AFL_TEST_NOARG("util.Key:unique")
{
    // Test uniqueness of key assignments.
    switch (util::Key_t(0)) {
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
     case util::Key_Help:
     case util::Key_Execute:
     case util::Key_Cut:
     case util::Key_Copy:
     case util::Key_Paste:
     case util::Key_Find:
     case util::Key_Mute:
     case util::Key_VolumeUp:
     case util::Key_VolumeDown:
     case util::Key_Undo:
     case util::Key_Redo:
     case util::Key_Next:
     case util::Key_Previous:
     case util::Key_Stop:
     case util::Key_Play:
     case util::Key_FastRewind:
     case util::Key_FastForward:
     case util::Key_NavHome:
     case util::Key_NavBack:
     case util::Key_NavForward:

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
AFL_TEST("util.Key:classifyKey", a)
{
    // Some regular keys
    a.checkEqual("01", util::classifyKey('a'), util::NormalKey);
    a.checkEqual("02", util::classifyKey(' '), util::NormalKey);
    a.checkEqual("03", util::classifyKey(0x4000), util::NormalKey);
    a.checkEqual("04", util::classifyKey(util::Key_PgDn), util::NormalKey);

    // Shifts
    a.checkEqual("11", util::classifyKey(util::Key_LShift), util::ModifierKey);
    a.checkEqual("12", util::classifyKey(util::Key_LAlt), util::ModifierKey);
    a.checkEqual("13", util::classifyKey(util::Key_LCtrl), util::ModifierKey);
    a.checkEqual("14", util::classifyKey(util::Key_RShift), util::ModifierKey);
    a.checkEqual("15", util::classifyKey(util::Key_RAlt), util::ModifierKey);
    a.checkEqual("16", util::classifyKey(util::Key_RCtrl), util::ModifierKey);

    // Virtual
    a.checkEqual("21", util::classifyKey(util::Key_Quit), util::VirtualKey);
    a.checkEqual("22", util::classifyKey(util::Key_WheelDown), util::VirtualKey);
    a.checkEqual("23", util::classifyKey(util::Key_WheelUp), util::VirtualKey);
}
