/**
  *  \file test/game/interface/userinterfacepropertystacktest.cpp
  *  \brief Test for game::interface::UserInterfacePropertyStack
  */

#include "game/interface/userinterfacepropertystack.hpp"

#include "afl/test/testrunner.hpp"
#include "game/interface/userinterfaceproperty.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "interpreter/arguments.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"

/** Test behaviour with empty stack. */
AFL_TEST("game.interface.UserInterfacePropertyStack:empty", a)
{
    game::interface::UserInterfacePropertyStack testee;

    a.checkNull("get iuiScreenNumber", testee.get(game::interface::iuiScreenNumber));
    AFL_CHECK_THROWS(a("set iuiScreenNumber"), testee.set(game::interface::iuiScreenNumber, 0), interpreter::Error);
}

/** Test behaviour with multiple elements. */
AFL_TEST("game.interface.UserInterfacePropertyStack:multi", a)
{
    // A test class
    class TestUIPA : public game::interface::UserInterfacePropertyAccessor {
     public:
        TestUIPA(game::interface::UserInterfaceProperty p, int initialValue)
            : m_property(p),
              m_value(initialValue)
            { }

        virtual bool get(game::interface::UserInterfaceProperty prop, std::auto_ptr<afl::data::Value>& result)
            {
                if (m_property == prop) {
                    result.reset(interpreter::makeIntegerValue(m_value));
                    return true;
                } else {
                    return false;
                }
            }
        virtual bool set(game::interface::UserInterfaceProperty prop, const afl::data::Value* p)
            {
                if (m_property == prop) {
                    int32_t tmp;
                    if (interpreter::checkIntegerArg(tmp, p)) {
                        m_value = tmp;
                    }
                    return true;
                } else {
                    return false;
                }
            }
     private:
        const game::interface::UserInterfaceProperty m_property;
        int m_value;
    };

    // Build it
    game::interface::UserInterfacePropertyStack testee;
    TestUIPA screen1(game::interface::iuiScreenNumber, 1);
    TestUIPA screen2(game::interface::iuiScreenNumber, 2);
    TestUIPA flag3(game::interface::iuiSimFlag, 3);
    testee.add(screen1);
    testee.add(flag3);
    testee.add(screen2);

    // Test reading
    // - undefined property
    int32_t tmp;
    std::auto_ptr<afl::data::Value> p;
    p.reset(testee.get(game::interface::iuiIterator));
    a.checkNull("01. get iuiIterator", p.get());

    // - stacked property
    p.reset(testee.get(game::interface::iuiScreenNumber));
    a.checkNonNull("11. get iuiScreenNumber", p.get());
    a.check       ("12. get iuiScreenNumber", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual  ("13. get iuiScreenNumber", tmp, 2);

    // - other property
    p.reset(testee.get(game::interface::iuiSimFlag));
    a.checkNonNull("21. get iuiSimFlag", p.get());
    a.check       ("22. get iuiSimFlag", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual  ("23. get iuiSimFlag", tmp, 3);

    // Test writing
    p.reset(interpreter::makeIntegerValue(17));
    AFL_CHECK_SUCCEEDS(a("31. set iuiScreenNumber"), testee.set(game::interface::iuiScreenNumber, p.get()));
    p.reset(interpreter::makeIntegerValue(21));
    AFL_CHECK_SUCCEEDS(a("32. set iuiSimFlag"), testee.set(game::interface::iuiSimFlag, p.get()));
    p.reset(interpreter::makeIntegerValue(42));
    AFL_CHECK_THROWS(a("33. set iuiIterator"), testee.set(game::interface::iuiIterator, p.get()), interpreter::Error);

    // Re-read
    p.reset(testee.get(game::interface::iuiScreenNumber));
    a.check     ("41. get iuiScreenNumber", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual("42. get iuiScreenNumber", tmp, 17);                                     // changed by set() above
    p.reset(testee.get(game::interface::iuiSimFlag));
    a.check     ("43. get iuiSimFlag", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual("44. get iuiSimFlag", tmp, 21);                                     // changed by set() above

    // Modify stack by removing element in the middle and retest
    testee.remove(flag3);
    p.reset(testee.get(game::interface::iuiScreenNumber));
    a.check     ("51. get iuiScreenNumber", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual("52. get iuiScreenNumber", tmp, 17);
    p.reset(testee.get(game::interface::iuiSimFlag));
    a.checkNull ("53. get iuiSimFlag", p.get());                                       // was in removed element

    // Modify stack by removing element at end and retest.
    // This uncovers the previous value.
    testee.remove(screen2);
    p.reset(testee.get(game::interface::iuiScreenNumber));
    a.check     ("61. get iuiScreenNumber", interpreter::checkIntegerArg(tmp, p.get()));
    a.checkEqual("62. get iuiScreenNumber", tmp, 1);                                      // uncovered
}
