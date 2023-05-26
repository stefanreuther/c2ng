/**
  *  \file u/t_ui_layout_manager.cpp
  *  \brief Test for ui::layout::Manager
  */

#include "ui/layout/manager.hpp"

#include "t_ui_layout.hpp"

/** Interface test. */
void
TestUiLayoutManager::testInterface()
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

