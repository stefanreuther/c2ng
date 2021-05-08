/**
  *  \file u/t_ui_icons_icon.cpp
  *  \brief Test for ui::icons::Icon
  */

#include "ui/icons/icon.hpp"

#include "t_ui_icons.hpp"

/** Interface test. */
void
TestUiIconsIcon::testInterface()
{
    class Tester : public ui::icons::Icon {
     public:
        virtual gfx::Point getSize() const
            { return gfx::Point(); }
        virtual void draw(gfx::Context<ui::SkinColor::Color>& /*ctx*/, gfx::Rectangle /*area*/, ui::ButtonFlags_t /*flags*/) const
            { }
    };
    Tester t;
}

