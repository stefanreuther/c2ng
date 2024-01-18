/**
  *  \file test/ui/icons/icontest.cpp
  *  \brief Test for ui::icons::Icon
  */

#include "ui/icons/icon.hpp"
#include "afl/test/testrunner.hpp"

/** Interface test. */
AFL_TEST_NOARG("ui.icons.Icon")
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
