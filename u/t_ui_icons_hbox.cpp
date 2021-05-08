/**
  *  \file u/t_ui_icons_hbox.cpp
  *  \brief Test for ui::icons::HBox
  */

#include "ui/icons/hbox.hpp"

#include "t_ui_icons.hpp"
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
void
TestUiIconsHBox::testDefault()
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);

    TS_ASSERT_EQUALS(testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(100, 70, 10, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(110, 75,  5, 10));
}

/** Test top-alignment.
    A: create a HBox with top alignment. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
void
TestUiIconsHBox::testTop()
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setAlign(gfx::LeftAlign, gfx::TopAlign);

    TS_ASSERT_EQUALS(testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(100, 50, 10, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(110, 50,  5, 10));
}

/** Test padding behaviour.
    A: create a HBox with padding. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
void
TestUiIconsHBox::testPad()
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setPad(7);

    TS_ASSERT_EQUALS(testee.getSize(), Point(22, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(100, 70, 10, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(117, 75,  5, 10));
}

/** Test right-alignment.
    A: create a HBox with right alignment. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
void
TestUiIconsHBox::testRight()
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::HBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setAlign(gfx::RightAlign, gfx::MiddleAlign);

    TS_ASSERT_EQUALS(testee.getSize(), Point(15, 20));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(290, 70, 10, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(285, 75,  5, 10));
}

