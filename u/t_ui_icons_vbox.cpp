/**
  *  \file u/t_ui_icons_vbox.cpp
  *  \brief Test for ui::icons::VBox
  */

#include "ui/icons/vbox.hpp"

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
    A: create a VBox with default settings. Add two icons.
    E: verify correct computation of getSize(), correct placement of draw(). */
void
TestUiIconsVBox::testDefault()
{
    Rectangle a1, a2;
    Tester t1(Point(10, 20), a1);
    Tester t2(Point(5, 10), a2);

    ui::icons::VBox testee;
    testee.add(t1);
    testee.add(t2);

    TS_ASSERT_EQUALS(testee.getSize(), Point(10, 30));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(100, 50, 10, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(100, 70,  5, 10));
}

/** Test behaviour with parameters.
    A: create a VBox with default settings. Add two icons. Set alignment and padding.
    E: verify correct computation of getSize(), correct placement of draw(). */
void
TestUiIconsVBox::testParam()
{
    Rectangle a1, a2;
    Tester t1(Point(20, 20), a1);
    Tester t2(Point(50, 10), a2);

    ui::icons::VBox testee;
    testee.add(t1);
    testee.add(t2);
    testee.setPad(3);
    testee.setAlign(gfx::CenterAlign);

    TS_ASSERT_EQUALS(testee.getSize(), Point(50, 33));

    gfx::NullCanvas can;
    gfx::NullColorScheme<SkinColor::Color> colorScheme;
    gfx::Context<SkinColor::Color> ctx(can, colorScheme);
    testee.draw(ctx, Rectangle(100, 50, 200, 60), ui::ButtonFlags_t());

    TS_ASSERT_EQUALS(a1, Rectangle(190, 50, 20, 20));
    TS_ASSERT_EQUALS(a2, Rectangle(175, 73, 50, 10));
}
