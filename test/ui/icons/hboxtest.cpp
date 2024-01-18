/**
  *  \file test/ui/icons/hboxtest.cpp
  *  \brief Test for ui::icons::HBox
  */

#include "ui/icons/hbox.hpp"

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
    A: create a HBox with default settings (middle alignment, no padding). Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.HBox:default", a)
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);

    a.checkEqual("01. getSize", testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(100, 70, 10, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(110, 75,  5, 10));
}

/** Test top-alignment.
    A: create a HBox with top alignment. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.HBox:TopAlign", a)
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setAlign(gfx::LeftAlign, gfx::TopAlign);

    a.checkEqual("01. getSize", testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(100, 50, 10, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(110, 50,  5, 10));
}

/** Test padding behaviour.
    A: create a HBox with padding. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.HBox:pad", a)
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setPad(7);

    a.checkEqual("01. getSize", testee.getSize(), Point(22, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(100, 70, 10, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(117, 75,  5, 10));
}

/** Test right-alignment.
    A: create a HBox with right alignment. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
AFL_TEST("ui.icons.HBox:RightAlign", a)
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setAlign(gfx::RightAlign, gfx::MiddleAlign);

    a.checkEqual("01. getSize", testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    a.checkEqual("11. pos 1", a1, Rectangle(290, 70, 10, 20));
    a.checkEqual("12. pos 2", a2, Rectangle(285, 75,  5, 10));
}
