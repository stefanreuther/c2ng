/**
  *  \file test/ui/widgets/checkboxtest.cpp
  *  \brief Test for ui::widgets::Checkbox
  */

#include "ui/widgets/checkbox.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

/** Test cycling a checkbox. */
AFL_TEST("ui.widgets.Checkbox:cycle", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Testee
    afl::base::Observable<int> value;
    ui::widgets::Checkbox testee(root, 'x', "Text", value);
    a.checkEqual("01. get", value.get(), 0);
    testee.setExtent(gfx::Rectangle(0, 0, 10, 10));

    // Add some states
    testee.addImage(1, "one");
    testee.addImage(3, "three");
    testee.addImage(2, "two");
    testee.addImage(0, "zero");
    a.checkEqual("11. get", value.get(), 0);

    // Cycle using key
    a.check("21", testee.handleKey('x', 0));
    a.checkEqual("22. get", value.get(), 1);

    a.check("31", testee.handleKey('x', 0));
    a.checkEqual("32. get", value.get(), 3);

    a.check("41", testee.handleKey('x', 0));
    a.checkEqual("42. get", value.get(), 2);

    a.check("51", testee.handleKey('x', 0));
    a.checkEqual("52. get", value.get(), 0);

    // Cycle using mouse
    const gfx::EventConsumer::MouseButtons_t LEFT = gfx::EventConsumer::MouseButtons_t(gfx::EventConsumer::LeftButton);
    const gfx::EventConsumer::MouseButtons_t NONE = gfx::EventConsumer::MouseButtons_t();
    a.check("61. handleMouse", testee.handleMouse(gfx::Point(3,3), LEFT));
    a.check("62. handleMouse", testee.handleMouse(gfx::Point(3,3), NONE));
    a.checkEqual("63. get", value.get(), 1);

    a.check("71. handleMouse", testee.handleMouse(gfx::Point(3,3), LEFT));
    a.check("72. handleMouse", testee.handleMouse(gfx::Point(3,3), NONE));
    a.checkEqual("73. get", value.get(), 3);

    a.check("81. handleMouse", testee.handleMouse(gfx::Point(3,3), LEFT));
    a.check("82. handleMouse", testee.handleMouse(gfx::Point(3,3), NONE));
    a.checkEqual("83. get", value.get(), 2);

    a.check("91. handleMouse", testee.handleMouse(gfx::Point(3,3), LEFT));
    a.check("92. handleMouse", testee.handleMouse(gfx::Point(3,3), NONE));
    a.checkEqual("93. get", value.get(), 0);
}

/** Test lifetime.
    The checkbox must retract its event subscriptions when it dies. */
AFL_TEST_NOARG("ui.widgets.Checkbox:lifetime")
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

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
