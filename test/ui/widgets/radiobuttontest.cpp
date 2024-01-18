/**
  *  \file test/ui/widgets/radiobuttontest.cpp
  *  \brief Test for ui::widgets::RadioButton
  */

#include "ui/widgets/radiobutton.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

/** Basic functionality test. */
AFL_TEST("ui.widgets.RadioButton:basics", a)
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Testee
    afl::base::Observable<int> value;
    ui::widgets::RadioButton first(root, 'x', "Text 1", value, 0);
    ui::widgets::RadioButton second(root, 'y', "Text 2", value, 1);
    a.checkEqual("01. get", value.get(), 0);

    // Click it
    a.check("11. handleKey", first.handleKey('x', 0));
    a.check("12. handleKey", !second.handleKey('x', 0));
    a.checkEqual("13. get", value.get(), 0);

    a.check("21. handleKey", !first.handleKey('y', 0));
    a.check("22. handleKey", second.handleKey('y', 0));
    a.checkEqual("23. get", value.get(), 1);

    a.check("31. handleKey", first.handleKey('x', 0));
    a.check("32. handleKey", !second.handleKey('x', 0));
    a.checkEqual("33. get", value.get(), 0);
}

/** Test lifetime.
    The radio button must retract its event subscriptions when it dies. */
AFL_TEST_NOARG("ui.widgets.RadioButton:lifetime")
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Testee
    afl::base::Observable<int> value;
    {
        ui::widgets::RadioButton testee(root, 'x', "Text", value, 0);
        value.set(1);
    }

    // If the radio button still has an active listener, this will call it.
    // (on my machine, this error is only visible in Valgrind, but could be a segfault.)
    value.set(0);
}
