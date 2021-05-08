/**
  *  \file u/t_game_interface_userinterfacepropertystack.cpp
  *  \brief Test for game::interface::UserInterfacePropertyStack
  */

#include "game/interface/userinterfacepropertystack.hpp"

#include "t_game_interface.hpp"
#include "game/interface/userinterfaceproperty.hpp"
#include "game/interface/userinterfacepropertyaccessor.hpp"
#include "interpreter/error.hpp"
#include "interpreter/values.hpp"
#include "interpreter/arguments.hpp"

/** Test behaviour with empty stack. */
void
TestGameInterfaceUserInterfacePropertyStack::testEmpty()
{
    game::interface::UserInterfacePropertyStack testee;

    TS_ASSERT(testee.get(game::interface::iuiScreenNumber) == 0);
    TS_ASSERT_THROWS(testee.set(game::interface::iuiScreenNumber, 0), interpreter::Error);
}

/** Test behaviour with multiple elements. */
void
TestGameInterfaceUserInterfacePropertyStack::testMulti()
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
    TS_ASSERT(p.get() == 0);

    // - stacked property
    p.reset(testee.get(game::interface::iuiScreenNumber));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 2);

    // - other property
    p.reset(testee.get(game::interface::iuiSimFlag));
    TS_ASSERT(p.get() != 0);
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 3);

    // Test writing
    p.reset(interpreter::makeIntegerValue(17));
    TS_ASSERT_THROWS_NOTHING(testee.set(game::interface::iuiScreenNumber, p.get()));
    p.reset(interpreter::makeIntegerValue(21));
    TS_ASSERT_THROWS_NOTHING(testee.set(game::interface::iuiSimFlag, p.get()));
    p.reset(interpreter::makeIntegerValue(42));
    TS_ASSERT_THROWS(testee.set(game::interface::iuiIterator, p.get()), interpreter::Error);

    // Re-read
    p.reset(testee.get(game::interface::iuiScreenNumber));
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 17);                                     // changed by set() above
    p.reset(testee.get(game::interface::iuiSimFlag));
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 21);                                     // changed by set() above

    // Modify stack by removing element in the middle and retest
    testee.remove(flag3);
    p.reset(testee.get(game::interface::iuiScreenNumber));
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 17);
    p.reset(testee.get(game::interface::iuiSimFlag));
    TS_ASSERT(p.get() == 0);                                       // was in removed element

    // Modify stack by removing element at end and retest.
    // This uncovers the previous value.
    testee.remove(screen2);
    p.reset(testee.get(game::interface::iuiScreenNumber));
    TS_ASSERT(interpreter::checkIntegerArg(tmp, p.get()));
    TS_ASSERT_EQUALS(tmp, 1);                                      // uncovered
}
