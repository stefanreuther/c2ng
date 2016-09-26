/**
  *  \file u/t_ui_res_manager.cpp
  *  \brief Test for ui::res::Manager
  */

#include "ui/res/manager.hpp"

#include "t_ui_res.hpp"

/** Simple test.
    For now, test just the idle state. */
void
TestUiResManager::testIt()
{
    ui::res::Manager t;

    // set/get
    t.setScreenSize(gfx::Point(100, 120));
    TS_ASSERT_EQUALS(t.getScreenSize(), gfx::Point(100, 120));

    // load
    afl::base::Ptr<gfx::Canvas> c;
    c = t.loadImage("foo");
    TS_ASSERT(c.get() == 0);
}
