/**
  *  \file u/t_ui_widgets_radiobutton.cpp
  *  \brief Test for ui::widgets::RadioButton
  */

#include "ui/widgets/radiobutton.hpp"

#include "t_ui_widgets.hpp"
#include "gfx/nullengine.hpp"
#include "gfx/nullresourceprovider.hpp"

/** Basic functionality test. */
void
TestUiWidgetsRadioButton::testIt()
{
    // Environment
    gfx::NullEngine engine;
    gfx::NullResourceProvider provider;
    ui::Root root(engine, provider, gfx::WindowParameters());

    // Testee
    afl::base::Observable<int> value;
    ui::widgets::RadioButton first(root, 'x', "Text 1", value, 0);
    ui::widgets::RadioButton second(root, 'y', "Text 2", value, 1);
    TS_ASSERT_EQUALS(value.get(), 0);

    // Click it
    TS_ASSERT(first.handleKey('x', 0));
    TS_ASSERT(!second.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 0);

    TS_ASSERT(!first.handleKey('y', 0));
    TS_ASSERT(second.handleKey('y', 0));
    TS_ASSERT_EQUALS(value.get(), 1);

    TS_ASSERT(first.handleKey('x', 0));
    TS_ASSERT(!second.handleKey('x', 0));
    TS_ASSERT_EQUALS(value.get(), 0);
}

/** Test lifetime.
    The radio button must retract its event subscriptions when it dies. */
void
TestUiWidgetsRadioButton::testLifetime()
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
