/**
  *  \file test/ui/icons/vboxtest.cpp
  *  \brief Test for ui::icons::VBox
  */

#include "ui/icons/vbox.hpp"

#include "afl/test/testrunner.hpp"
#include "gfx/nullcanvas.hpp"
#include "gfx/nullcolorscheme.hpp"

using gfx::Point;
using gfx::Rectangle;
using ui::SkinColor;

namespace {
    class Tester : public ui::icons::Icon {
     public:
        Tester(Point size, Rectangle& area)
            : m_size(size), m_area(area)
            { }

        virtual Point getSize() const
            { return m_size; }
        virtual void draw(gfx::Context<SkinColor::Color>& /*ctx*/, gfx::Rectangle area, ui::ButtonFlags_t /*flags*/) const
            { m_area = area; }

     private:
        Point m_size;
        Rectangle& m_area;
    };
}

/** Test default behaviour.
    A: create a VBox with default settings. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.VBox:default", a)
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::VBox testee;
    testee.add(t1);
    testee.add(t2);

    a.checkEqual("01. getSize", testee.getSize(), Point(10, 30));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(100, 50, 10, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(100, 70,  5, 10));
}

/** Test behaviour with parameters.
    A: create a VBox with default settings. Add two icons. Set alignment and padding.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.VBox:param", a)
{
    Rectangle a1, a2;
    Tester t1(Point(20, 20), a1);
    Tester t2(Point(50, 10), a2);

    ui::icons::VBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setPad(3);
    testee.setAlign(gfx::CenterAlign);

    a.checkEqual("01. getSize", testee.getSize(), Point(50, 33));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(190, 50, 20, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(175, 73, 50, 10));
}
