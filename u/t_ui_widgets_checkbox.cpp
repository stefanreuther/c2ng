/**
  *  \file u/t_ui_widgets_checkbox.cpp
  *  \brief Test for ui::widgets::Checkbox
  */

#include "ui/widgets/checkbox.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

/** Test cycling a checkbox. */
void
TestUiWidgetsCheckbox::testCycle()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, 100, 100, 8, gfx::Engine::WindowFlags_t());

    // Testee
    afl::base::Observable<int> value;
    ui::widgets::Checkbox testee(root, 'x', "Text", value);
    TS_ASSERT_EQUALS(value.get(), 0);
    testee.setExtent(gfx::Rectangle(0, 0, 10, 10));

    // Add some states
    testee.addImage(1, "one");
    testee.addImage(3, "three");
    testee.addImage(2, "two");
    testee.addImage(0, "zero");
    TS_ASSERT_EQUALS(value.get(), 0);

    // Cycle using key
    TS_ASSERT(testee.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 1);

    TS_ASSERT(testee.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 3);

    TS_ASSERT(testee.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 2);

    TS_ASSERT(testee.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 0);

    // Cycle using mouse
    const gfx::EventConsumer::MouseButtons_t LEFT = gfx::EventConsumer::MouseButtons_t(gfx::EventConsumer::LeftButton);
    const gfx::EventConsumer::MouseButtons_t NONE = gfx::EventConsumer::MouseButtons_t();
    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), LEFT));
    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), NONE));
    TS_ASSERT_EQUALS(value.get(), 1);

    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), LEFT));
    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), NONE));
    TS_ASSERT_EQUALS(value.get(), 3);

    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), LEFT));
    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), NONE));
    TS_ASSERT_EQUALS(value.get(), 2);

    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), LEFT));
    TS_ASSERT(testee.handleMouse(gfx::Point(3,3), NONE));
    TS_ASSERT_EQUALS(value.get(), 0);
}

/** Test lifetime.
    The checkbox must retract its event subscriptions when it dies. */
void
TestUiWidgetsCheckbox::testLifetime()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, 100, 100, 8, gfx::Engine::WindowFlags_t());

    // Testee
    afl::base::Observable<int> value;
    {
        ui::widgets::Checkbox testee(root, 'x', "Text", value);
        testee.addDefaultImages();
        value.set(1);
    }

    // If the checkbox still has an active listener, this will call it.
    // (on my machine, this error is only visible in Valgrind, but could be a segfault.)
    value.set(0);
}
