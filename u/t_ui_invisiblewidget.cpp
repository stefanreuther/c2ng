/**
  *  \file u/t_ui_invisiblewidget.cpp
  *  \brief Test for ui::InvisibleWidget
  */

#include "ui/invisiblewidget.hpp"

#include "t_ui.hpp"

/** Interface test. */
void
TestUiInvisibleWidget::testIt()
{
    class MyWidget : public ui::InvisibleWidget {
     public:
        virtual bool handleKey(util::Key_t /*key*/, int /*prefix*/)
            { return false; }
        virtual bool handleMouse(gfx::Point /*pt*/, MouseButtons_t /*pressedButtons*/)
            { return false; }
    };
    MyWidget t;
}
