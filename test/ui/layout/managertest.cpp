/**
  *  \file test/ui/layout/managertest.cpp
  *  \brief Test for ui::layout::Manager
  */

#include "ui/layout/manager.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("ui.layout.Manager")
{
    class Tester : public ui::layout::Manager {
     public:
        virtual void doLayout(ui::Widget& /*container*/, gfx::Rectangle /*area*/) const
            { }
        virtual ui::layout::Info getLayoutInfo(const ui::Widget& /*container*/) const
            { return ui::layout::Info(); }
    };
    Tester t;
}
